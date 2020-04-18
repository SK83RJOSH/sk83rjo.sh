#include "MeshInstance.h"

#include "Mesh.h"
#include "MaterialInstance.h"

#include <SDL_opengl.h>
#include <SDL_image.h>

namespace NRender
{
CMeshInstance::CMeshInstance(std::shared_ptr<SMesh> mesh)
	: m_Mesh(mesh)
{
	m_Transform.setIdentity();
	CreateBuffers();
}

CMeshInstance::~CMeshInstance()
{
	DestroyBuffers();
}

void CMeshInstance::CreateBuffers()
{
	// Clean up old buffers
	DestroyBuffers();

	// Load materials
	m_Materials.reserve(m_Mesh->m_Materials.size());
	for (const NRender::HMaterial& material : m_Mesh->m_Materials)
	{
		HMaterialInstance material_instance = std::make_shared<CMaterialInstance>(material);
		m_Materials.push_back(material_instance);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// Generate and bind VAO
	glGenVertexArrays(m_VAO.size(), m_VAO.data());
	glBindVertexArray(m_VAO[0]);

	// Generate buffer objects
	glGenBuffers(m_VBO.size(), m_VBO.data());

	// Setup vertices
#define OFFSET(TYPE, MEMBER) ((void*)&((TYPE*)0)->MEMBER)
	using SVertexData = NRender::SMesh::SVertexData;

	printf("Vertices, vertex size, buffer size: %lu, %lu, %lu\n",
		   m_Mesh->m_Vertices.size(),
		   sizeof(SVertexData),
		   m_Mesh->m_Vertices.size() * sizeof(SVertexData));

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, m_Mesh->m_Vertices.size() * sizeof(SVertexData), m_Mesh->m_Vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);  // Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SVertexData), OFFSET(SVertexData, m_Position));

	glEnableVertexAttribArray(1);  // Normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(SVertexData), OFFSET(SVertexData, m_Normal));

	glEnableVertexAttribArray(2);  // Tangent
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SVertexData), OFFSET(SVertexData, m_Tangent));

	glEnableVertexAttribArray(3);  // Color
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SVertexData), OFFSET(SVertexData, m_Color));

	glEnableVertexAttribArray(4);  // UV
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(SVertexData), OFFSET(SVertexData, m_UV));
#undef OFFSET

	// Setup indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Mesh->m_Indices.size() * sizeof(m_Mesh->m_Indices[0]), m_Mesh->m_Indices.data(), GL_STATIC_DRAW);

	// Unbind VAO
	glBindVertexArray(0);
}

void CMeshInstance::DestroyBuffers()
{
	glDeleteBuffers(m_VBO.size(), m_VBO.data());
	m_VBO.fill(0);

	glDeleteVertexArrays(m_VAO.size(), m_VAO.data());
	m_VAO.fill(0);
}

void CMeshInstance::Draw()
{
	GLint program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);
	glUniformMatrix4fv(glGetUniformLocation(program, "ModelMatrix"), 1, GL_FALSE, m_Transform.data());

	glBindVertexArray(m_VAO[0]);
	for (const auto& sub_mesh : m_Mesh->m_SubMeshes)
	{
		m_Materials[sub_mesh.m_Material]->Bind();
		glDrawElements(GL_TRIANGLES, sub_mesh.m_IndexCount, GL_UNSIGNED_INT, (void*)(sub_mesh.m_IndexOffset * sizeof(size_t)));
		m_Materials[sub_mesh.m_Material]->Unbind();
	}
	glBindVertexArray(0);
}

void CMeshInstance::Scale(const float scale)
{
	m_Transform.scale(scale);
}

void CMeshInstance::Rotate(const CMatrix3f& rotation)
{
	m_Transform.rotate(rotation);
}

void CMeshInstance::SetPosition(const CVector3f& position)
{
	m_Transform.translation() = position;
}

void CMeshInstance::Reload()
{
	CreateBuffers();

	for (HMaterialInstance& material : m_Materials)
	{
		material->Reload();
	}
}
};	// namespace NRender
