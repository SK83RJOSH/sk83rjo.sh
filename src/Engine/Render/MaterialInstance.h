#pragma once

#include <SDL_opengl.h>

#include <memory>
#include <array>

namespace NRender
{
struct SMaterial;
using HMaterial = std::shared_ptr<SMaterial>;

struct STexture;
using HTexture = std::shared_ptr<STexture>;

class CMaterialInstance
{
public:
	CMaterialInstance(HMaterial material);
	~CMaterialInstance();

	void Reload();
	void Bind();
	void Unbind();

private:
	void CreateTexture(size_t index, HTexture texture);
	void CreateTextures();
	void DestroyTextures();

	HMaterial m_Material;
	std::array<GLuint, 2> m_Textures;
};
}  // namespace NRender
