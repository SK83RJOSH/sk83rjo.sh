#include "MeshLoader.h"

#include <Engine/Math.h>
#include <Engine/Render/Material.h>
#include <Engine/Render/Mesh.h>
#include <Engine/Render/Texture.h>

#include <algorithm>
#include <vector>

#include <SDL_image.h>
#include <SDL_surface.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#undef TINYOBJLOADER_IMPLEMENTATION

bool NUtils::LoadMesh(const char* filename, const char* asset_path, NRender::SMesh& mesh)
{
	tinyobj::ObjReader reader;
	reader.ParseFromFile(filename);

	if (!reader.Warning().empty())
	{
		printf("Warning: %s\n", reader.Warning().c_str());
	}

	if (!reader.Error().empty())
	{
		printf("Error: %s\n", reader.Error().c_str());
	}

	if (!reader.Valid())
	{
		printf("Failed to load: %s\n", filename);
		return false;
	}

	using HTexture = std::shared_ptr<NRender::STexture>;

	std::map<std::string, HTexture> texture_cache;

	auto GetOrCreateTexture = [&texture_cache](std::string path) -> HTexture {
		auto texture_pair = texture_cache.find(path);

		if (texture_pair != texture_cache.end())
		{
			return texture_pair->second;
		}

		SDL_Surface* surface = IMG_Load(path.c_str());

		if (surface == nullptr)
		{
			return HTexture(nullptr);
		}

		// We're going to assume 8-bits per pixels from here on out
		HTexture texture = std::make_shared<NRender::STexture>();
		texture->m_Width = surface->w;
		texture->m_Height = surface->h;
		texture->m_BytesPerPixel = surface->format->BytesPerPixel;
		texture->m_Buffer.resize(surface->pitch * surface->h);

		// Copy the bare minimum data we can
		size_t size = surface->w * surface->format->BytesPerPixel;

		for (size_t y = 0; y < surface->h; ++y)
		{
			size_t offset = surface->pitch * y;
			memcpy(texture->m_Buffer.data() + offset, static_cast<uint8_t*>(surface->pixels) + offset, size);
		}

		// Free the surface
		SDL_FreeSurface(surface);

		return texture;
	};

	mesh.m_Materials.reserve(reader.GetMaterials().size());

	const std::string base_path("assets/models/textures/");

	for (const tinyobj::material_t& mat : reader.GetMaterials())
	{
		printf("%s, %s, %s\n", mat.name.c_str(), mat.diffuse_texname.c_str(), mat.bump_texname.c_str());
		std::shared_ptr<NRender::SMaterial> material = std::make_shared<NRender::SMaterial>();
		material->m_Name = mat.name;
		material->m_AlbedoTexture = GetOrCreateTexture(base_path + mat.diffuse_texname);
		material->m_DetailTexture = GetOrCreateTexture(base_path + mat.bump_texname);

		mesh.m_Materials.push_back(material);
	}

	const tinyobj::attrib_t& attributes = reader.GetAttrib();
	mesh.m_SubMeshes.reserve(reader.GetShapes().size());

	// Create vertices
	mesh.m_Vertices.resize(attributes.vertices.size(), NRender::SMesh::SVertexData());
	for (size_t i = 0; i < attributes.vertices.size() / 3; ++i)
	{
		NRender::SMesh::SVertexData& vertex_data = mesh.m_Vertices[i];

		vertex_data.m_Position = {
			attributes.vertices[i * 3 + 0],
			attributes.vertices[i * 3 + 1],
			attributes.vertices[i * 3 + 2]
		};

		if (!attributes.colors.empty())
		{
			vertex_data.m_Color = {
				attributes.colors[i * 3 + 0],
				attributes.colors[i * 3 + 1],
				attributes.colors[i * 3 + 2]
			};
		}
	}

	// Create map of vertex groups
	struct SVertexIndices
	{
		size_t m_VertexIndex;
		size_t m_CoordIndex;
	};
	struct SVertexGroup
	{
		std::vector<SVertexIndices> m_Indices;
		std::vector<size_t> m_SubMeshes;
	};
	std::map<size_t, SVertexGroup> vertex_groups;

	// Create indices
	for (const tinyobj::shape_t& shape : reader.GetShapes())
	{
		// Would be nice to preload the entire indices array, but this will do
		mesh.m_Indices.reserve(mesh.m_Indices.size() + shape.mesh.indices.size());

		NRender::SMesh::SSubMesh sub_mesh;
		sub_mesh.m_Name = shape.name;
		sub_mesh.m_IndexCount = shape.mesh.indices.size();
		sub_mesh.m_IndexOffset = mesh.m_Indices.size();
		sub_mesh.m_Material = shape.mesh.material_ids.front();

		for (auto& indices : shape.mesh.indices)
		{
			SVertexGroup& vertex_group = vertex_groups[indices.vertex_index];
			vertex_group.m_SubMeshes.push_back(mesh.m_SubMeshes.size());
			vertex_group.m_Indices.push_back({ mesh.m_Indices.size(),
											   static_cast<size_t>(indices.texcoord_index) });

			mesh.m_Indices.push_back(indices.vertex_index);
		}

		mesh.m_SubMeshes.push_back(sub_mesh);
	}

	// Handle UVs and duplication of vertices
	for (const auto& entry : vertex_groups)
	{
		const size_t current_vertex_index = entry.first;
		SVertexGroup vertex_group = entry.second;
		std::vector<NRender::SMesh::SVertexData>& vertices = mesh.m_Vertices;
		std::vector<size_t> unique_coords;

		// Build a list of unique coordinates (emscripten has a bug with std::set)
		for (const auto indice : vertex_group.m_Indices)
		{
			const bool unique = std::none_of(
				unique_coords.begin(),
				unique_coords.end(),
				[&indice](size_t coord_index) -> bool { return coord_index == indice.m_CoordIndex; });

			if (unique)
			{
				unique_coords.emplace_back(indice.m_CoordIndex);
			}
		}

		// Store the current size of the vertex array
		const size_t last = vertices.size();
		// Calculate how many duplicate vertices we need, if any
		const size_t new_vertices = unique_coords.size() - 1;

		// Copy this vertex n times at the end of the array
		vertices.reserve(last + new_vertices);

		// Update indices
		for (size_t i = 0; i < unique_coords.size(); ++i)
		{
			const size_t coord_index = unique_coords[i];
			const size_t new_index = (i == 0 ? current_vertex_index : vertices.size());

			if (i > 0)
			{
				vertices.emplace_back(vertices[current_vertex_index]);
			}

			for (SVertexIndices& indices : vertex_group.m_Indices)
			{
				if (indices.m_CoordIndex == coord_index)
				{
					mesh.m_Indices[indices.m_VertexIndex] = new_index;
				}
			}

			vertices[new_index].m_UV = {
				attributes.texcoords[coord_index * 2 + 0],
				1.0f - attributes.texcoords[coord_index * 2 + 1]
			};
		}
	}

	// Create vertex normal data
	struct SVertexNormal
	{
		CVector3f m_Normal = CVector3f::Zero();
		CVector3f m_Tangent = CVector3f::Zero();
		CVector3f m_Bitangent = CVector3f::Zero();
	};
	std::vector<SVertexNormal> vertex_normals;
	vertex_normals.resize(mesh.m_Vertices.size(), SVertexNormal());

	// Calculate vertex normal and tangents
	for (size_t i = 0; i < mesh.m_Indices.size() / 3; ++i)
	{
		const size_t index_0 = mesh.m_Indices[i * 3 + 0];
		const size_t index_1 = mesh.m_Indices[i * 3 + 1];
		const size_t index_2 = mesh.m_Indices[i * 3 + 2];

		SVertexNormal& vertex_normal_0 = vertex_normals[index_0];
		SVertexNormal& vertex_normal_1 = vertex_normals[index_1];
		SVertexNormal& vertex_normal_2 = vertex_normals[index_2];

		const CVector3f position_0(&mesh.m_Vertices[index_0].m_Position.m_X);
		const CVector3f position_1(&mesh.m_Vertices[index_1].m_Position.m_X);
		const CVector3f position_2(&mesh.m_Vertices[index_2].m_Position.m_X);
		const CVector3f edge_0 = position_1 - position_0;
		const CVector3f edge_1 = position_2 - position_0;

		const CVector2f uv_0(&mesh.m_Vertices[index_0].m_UV.m_X);
		const CVector2f uv_1(&mesh.m_Vertices[index_1].m_UV.m_X);
		const CVector2f uv_2(&mesh.m_Vertices[index_2].m_UV.m_X);
		const CVector2f delta_uv_0 = uv_1 - uv_0;
		const CVector2f delta_uv_1 = uv_2 - uv_0;

		float f = 1.0f / (delta_uv_0.x() * delta_uv_1.y() - delta_uv_1.x() * delta_uv_0.y());

		CVector3f normal = edge_0.cross(edge_1);
		const CVector3f tangent = CVector3f(
			f * (delta_uv_1.y() * edge_0.x() - delta_uv_0.y() * edge_1.x()),
			f * (delta_uv_1.y() * edge_0.y() - delta_uv_0.y() * edge_1.y()),
			f * (delta_uv_1.y() * edge_0.z() - delta_uv_0.y() * edge_1.z()));
		const CVector3f bitangent = CVector3f(
			f * (-delta_uv_1.x() * edge_0.x() + delta_uv_0.x() * edge_1.x()),
			f * (-delta_uv_1.x() * edge_0.y() + delta_uv_0.x() * edge_1.y()),
			f * (-delta_uv_1.x() * edge_0.z() + delta_uv_0.x() * edge_1.z()));

		vertex_normal_0.m_Normal += normal;
		vertex_normal_1.m_Normal += normal;
		vertex_normal_2.m_Normal += normal;

		vertex_normal_0.m_Tangent += tangent;
		vertex_normal_1.m_Tangent += tangent;
		vertex_normal_2.m_Tangent += tangent;

		vertex_normal_0.m_Bitangent += bitangent;
		vertex_normal_1.m_Bitangent += bitangent;
		vertex_normal_2.m_Bitangent += bitangent;
	}

	// Smooth vertex_normals
	for (size_t i = 0; i < vertex_normals.size(); ++i)
	{
		const SVertexNormal& vertex_normal = vertex_normals[i];
		const CVector3f normal = vertex_normal.m_Normal.normalized();
		const CVector3f tangent = vertex_normal.m_Tangent;
		const CVector3f bitangent = vertex_normal.m_Bitangent;
		const CVector3f ortho_tangent((tangent - normal * normal.dot(tangent)).normalized().data());
		const float w = normal.cross(tangent).dot(bitangent) < 0.0f ? -1.0f : 1.0f;

		for (const SVertexIndices& indices : vertex_groups[i].m_Indices)
		{
			NRender::SMesh::SVertexData& vertex_data = mesh.m_Vertices[mesh.m_Indices[indices.m_VertexIndex]];
			vertex_data.m_Normal = {
				normal.x(),
				normal.y(),
				normal.z()
			};
			vertex_data.m_Tangent = {
				ortho_tangent.x(),
				ortho_tangent.y(),
				ortho_tangent.z(),
				w
			};
		}
	}

	return true;
};
