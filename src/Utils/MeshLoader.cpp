#include "MeshLoader.h"

#include <Engine/Math.h>
#include <Engine/Render/Material.h>
#include <Engine/Render/Mesh.h>
#include <Engine/Render/Texture.h>

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

	for (const tinyobj::shape_t& shape : reader.GetShapes())
	{
		mesh.m_Indices.reserve(shape.mesh.indices.size());
		mesh.m_Vertices.reserve(shape.mesh.indices.size());

		NRender::SMesh::SSubMesh sub_mesh;
		sub_mesh.m_Name = shape.name;
		sub_mesh.m_IndexCount = shape.mesh.indices.size();
		sub_mesh.m_IndexOffset = mesh.m_Indices.size();
		sub_mesh.m_VertexOffset = mesh.m_Vertices.size();
		sub_mesh.m_Material = shape.mesh.material_ids.front();

		for (auto& indices : shape.mesh.indices)
		{
			NRender::SMesh::SVertexData vertex_data;

			vertex_data.m_Position = {
				attributes.vertices[indices.vertex_index * 3 + 0],
				attributes.vertices[indices.vertex_index * 3 + 1],
				attributes.vertices[indices.vertex_index * 3 + 2]
			};

			if (attributes.colors.size() != 0)
			{
				vertex_data.m_Color = {
					attributes.colors[indices.vertex_index * 3 + 0],
					attributes.colors[indices.vertex_index * 3 + 1],
					attributes.colors[indices.vertex_index * 3 + 2]
				};
			}

			if (attributes.normals.size() != 0)
			{
				vertex_data.m_Normal = {
					attributes.normals[indices.normal_index * 3 + 0],
					attributes.normals[indices.normal_index * 3 + 1],
					attributes.normals[indices.normal_index * 3 + 2]
				};
			}

			if (attributes.texcoords.size() != 0)
			{
				vertex_data.m_UV = {
					attributes.texcoords[indices.texcoord_index * 2 + 0],
					1.0f - attributes.texcoords[indices.texcoord_index * 2 + 1]
				};
			}

			mesh.m_Vertices.push_back(vertex_data);
			mesh.m_Indices.push_back(mesh.m_Indices.size());
		}

		// Calculate normals and tangents
		for (size_t i = 0; i < mesh.m_Vertices.size() / 3; ++i)
		{
			const CVector3f position_0(&mesh.m_Vertices[i * 3 + 0].m_Position.m_X);
			const CVector3f position_1(&mesh.m_Vertices[i * 3 + 1].m_Position.m_X);
			const CVector3f position_2(&mesh.m_Vertices[i * 3 + 2].m_Position.m_X);
			const CVector3f edge_0 = position_1 - position_0;
			const CVector3f edge_1 = position_2 - position_0;

			const CVector2f uv_0(&mesh.m_Vertices[i * 3 + 0].m_UV.m_X);
			const CVector2f uv_1(&mesh.m_Vertices[i * 3 + 1].m_UV.m_X);
			const CVector2f uv_2(&mesh.m_Vertices[i * 3 + 2].m_UV.m_X);
			const CVector2f delta_uv_0 = uv_1 - uv_0;
			const CVector2f delta_uv_1 = uv_2 - uv_0;

			float f = 1.0f / (delta_uv_0.x() * delta_uv_1.y() - delta_uv_1.x() * delta_uv_0.y());

			const CVector3f tangent_1 =
				CVector3f(
					f * (delta_uv_1.y() * edge_0.x() - delta_uv_0.y() * edge_1.x()),
					f * (delta_uv_1.y() * edge_0.y() - delta_uv_0.y() * edge_1.y()),
					f * (delta_uv_1.y() * edge_0.z() - delta_uv_0.y() * edge_1.z()))
					.normalized();

			const CVector3f tangent_2 =
				CVector3f(
					f * (-delta_uv_1.x() * edge_0.x() + delta_uv_0.x() * edge_1.x()),
					f * (-delta_uv_1.x() * edge_0.y() + delta_uv_0.x() * edge_1.y()),
					f * (-delta_uv_1.x() * edge_0.z() + delta_uv_0.x() * edge_1.z()))
					.normalized();

			const CVector3f normal = edge_0.cross(edge_1).normalized();

			CVector4f tangent((tangent_1 - normal * normal.dot(tangent_1)).normalized().data());
			tangent.w() = (normal.cross(tangent_1).dot(tangent_2) < 0.0f ? -1.0f : 1.0f);

			memcpy(&mesh.m_Vertices[i * 3 + 0].m_Tangent, tangent.data(), sizeof(CVector4f));
			memcpy(&mesh.m_Vertices[i * 3 + 1].m_Tangent, tangent.data(), sizeof(CVector4f));
			memcpy(&mesh.m_Vertices[i * 3 + 2].m_Tangent, tangent.data(), sizeof(CVector4f));

			memcpy(&mesh.m_Vertices[i * 3 + 0].m_Normal, normal.data(), sizeof(CVector3f));
			memcpy(&mesh.m_Vertices[i * 3 + 1].m_Normal, normal.data(), sizeof(CVector3f));
			memcpy(&mesh.m_Vertices[i * 3 + 2].m_Normal, normal.data(), sizeof(CVector3f));
		}

		mesh.m_SubMeshes.push_back(sub_mesh);
	}

	return true;
};
