#include "MeshLoader.h"

#include <Engine/Math.h>
#include <Engine/Render/Material.h>
#include <Engine/Render/Mesh.h>
#include <Engine/Render/Texture.h>

#include <algorithm>
#include <vector>
#include <map>

#include <SDL_image.h>
#include <SDL_surface.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static Assimp::Importer importer;

bool NUtils::LoadMesh(const char* filename, const char* asset_path, NRender::SMesh& mesh)
{
	// Load triangles from the scene
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
	const aiScene* ai_scene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs | aiProcess_FixInfacingNormals | aiProcess_ValidateDataStructure);
	if (!ai_scene)
	{
		printf("%s\n", importer.GetErrorString());
		return false;
	}

	// Material helpers
	using HTexture = std::shared_ptr<NRender::STexture>;
	std::map<std::string, HTexture> texture_cache;

	auto GetOrCreateTexture = [&texture_cache](std::string path) -> HTexture {
		path = std::string("assets/models/textures/") + path;

		auto texture_pair = texture_cache.find(path);

		if (texture_pair != texture_cache.end())
		{
			return texture_pair->second;
		}

		SDL_Surface* surface = IMG_Load(path.c_str());

		if (surface == nullptr)
		{
			printf("Could not load texture: %s\n", path.c_str());
			return HTexture(nullptr);
		}

		// We're going to assume 8-bits per channel from here on out
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

		// Store the texture in our cache
		texture_cache.emplace(path, texture);

		return texture;
	};

	// Preload materials vector
	mesh.m_Materials.resize(ai_scene->mNumMaterials);

	for (size_t i = 0; i < ai_scene->mNumMaterials; ++i)
	{
		const aiMaterial* ai_material = ai_scene->mMaterials[i];
		NRender::HMaterial material = std::make_shared<NRender::SMaterial>();

		aiString name;
		aiGetMaterialString(ai_material, AI_MATKEY_NAME, &name);
		material->m_Name = name.C_Str();

		float transparency = 0.0f;
		aiGetMaterialFloat(ai_material, AI_MATKEY_OPACITY, &transparency);
		printf("%f\n", transparency);

		if (ai_material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;
			ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			material->m_AlbedoTexture = GetOrCreateTexture(path.C_Str());
		}

		if (ai_material->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			aiString path;
			ai_material->GetTexture(aiTextureType_NORMALS, 0, &path);
			material->m_DetailTexture = GetOrCreateTexture(path.C_Str());
		};

		mesh.m_Materials[i] = material;
	}

	// Count total vertices and indices
	size_t vertex_count = 0;
	size_t index_count = 0;
	for (size_t i = 0; i < ai_scene->mNumMeshes; ++i)
	{
		vertex_count += ai_scene->mMeshes[i]->mNumVertices;
		index_count += ai_scene->mMeshes[i]->mNumFaces * 3;
	}

	// Create mesh and preload vectors
	mesh.m_Vertices.resize(vertex_count, {});
	mesh.m_Indices.resize(index_count, 0);
	mesh.m_SubMeshes.resize(ai_scene->mNumMeshes, {});

	// Populate vertices, indices, and submeshes
	size_t current_vertex = 0;
	size_t current_index = 0;
	for (size_t i = 0; i < ai_scene->mNumMeshes; ++i)
	{
		const aiMesh* ai_mesh = ai_scene->mMeshes[i];

		// Populate vertices
		for (size_t v = 0; v < ai_mesh->mNumVertices; ++v)
		{
			NRender::SMesh::SVertexData& vertex_data = mesh.m_Vertices[current_vertex + v];

			if (ai_mesh->HasPositions())
			{
				const aiVector3D& position = ai_mesh->mVertices[v];
				vertex_data.m_Position = { position.x, position.y, position.z };
			}

			if (ai_mesh->HasVertexColors(0))
			{
				const aiColor4D& color = ai_mesh->mColors[0][v];
				vertex_data.m_Color = { color.r, color.g, color.b };
			}

			if (ai_mesh->HasNormals() && ai_mesh->HasTangentsAndBitangents())
			{
				const CVector3f normal(&ai_mesh->mNormals[v].x);
				const CVector3f tangent(&ai_mesh->mTangents[v].x);
				const CVector3f bitangent(&ai_mesh->mBitangents[v].x);
				const float w = normal.cross(tangent).dot(bitangent) < 0.0f ? -1.0f : 1.0f;	 // This might be broken
				vertex_data.m_Normal = { normal.x(), normal.y(), normal.z() };
				vertex_data.m_Tangent = { tangent.x(), tangent.y(), tangent.z(), w };
			}

			if (ai_mesh->HasTextureCoords(0))
			{
				const aiVector3D& coord = ai_mesh->mTextureCoords[0][v];
				vertex_data.m_UV = { coord.x, coord.y };
			}
		}

		// Populate indices
		for (size_t f = 0; f < ai_mesh->mNumFaces; ++f)
		{
			const aiFace& face = ai_mesh->mFaces[f];
			mesh.m_Indices[current_index + (f * 3) + 0] = current_vertex + face.mIndices[0];
			mesh.m_Indices[current_index + (f * 3) + 1] = current_vertex + face.mIndices[1];
			mesh.m_Indices[current_index + (f * 3) + 2] = current_vertex + face.mIndices[2];
		}

		// Setup submesh
		NRender::SMesh::SSubMesh& sub_mesh = mesh.m_SubMeshes[i];
		sub_mesh.m_Name = ai_mesh->mName.C_Str();
		sub_mesh.m_VertexCount = ai_mesh->mNumVertices;
		sub_mesh.m_VertexOffset = current_vertex;
		sub_mesh.m_IndexCount = ai_mesh->mNumFaces * 3;
		sub_mesh.m_IndexOffset = current_index;
		sub_mesh.m_Material = ai_mesh->mMaterialIndex;

		// Update current indices
		current_index += ai_mesh->mNumFaces * 3;
		current_vertex += ai_mesh->mNumVertices;
	}

	return true;
};
