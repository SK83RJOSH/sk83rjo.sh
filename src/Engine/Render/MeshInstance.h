#pragma once

#include <Engine/Math.h>

#include <SDL_opengl.h>

#include <array>
#include <memory>
#include <vector>

struct SDL_Surface;
struct SDL_Texture;

namespace NRender
{
struct SMesh;
using HMesh = std::shared_ptr<SMesh>;

class CMaterialInstance;
using HMaterialInstance = std::shared_ptr<CMaterialInstance>;

class CMeshInstance
{
public:
	CMeshInstance(std::shared_ptr<SMesh> mesh);
	~CMeshInstance();

	void Draw();
	void Scale(const float scale);
	void Rotate(const CMatrix3f& rotation);
	void SetPosition(const CVector3f& position);
	void SetPosition(float x, float y, float z) { SetPosition(CVector3f(x, y, z)); };
	void Reload();

private:
	void CreateBuffers();
	void DestroyBuffers();

	HMesh m_Mesh;
	CMatrix4f m_Transform;
	std::array<GLuint, 1> m_VAO = {};
	std::array<GLuint, 2> m_VBO = {};
	std::vector<HMaterialInstance> m_Materials;
};
};	// namespace NRender
