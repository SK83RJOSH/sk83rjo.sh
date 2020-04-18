#pragma once

#include <stdint.h>
#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

namespace NRender
{
struct SMaterial;
using HMaterial = std::shared_ptr<SMaterial>;

struct SMesh
{
	struct SVector2
	{
		float m_X = 0.0f;
		float m_Y = 0.0f;
	};

	struct SVector3
	{
		float m_X = 0.0f;
		float m_Y = 0.0f;
		float m_Z = 0.0f;
	};

	struct SVector4
	{
		float m_X = 0.0f;
		float m_Y = 0.0f;
		float m_Z = 0.0f;
		float m_W = 0.0f;
	};

	struct SVertexData
	{
		SVector3 m_Position;
		SVector3 m_Normal;
		SVector4 m_Tangent;
		SVector3 m_Color;
		SVector2 m_UV;
	};

	struct SSubMesh
	{
		std::string m_Name;
		size_t m_IndexOffset = 0;
		size_t m_VertexOffset = 0;
		size_t m_IndexCount = 0;
		size_t m_Material = 0;
	};

	std::vector<SVertexData> m_Vertices;
	std::vector<uint32_t> m_Indices;
	std::vector<HMaterial> m_Materials;
	std::vector<SSubMesh> m_SubMeshes;
};
}  // namespace NRender
