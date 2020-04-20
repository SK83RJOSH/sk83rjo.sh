#include "MaterialInstance.h"

#include "Texture.h"
#include "Material.h"

namespace NRender
{
CMaterialInstance::CMaterialInstance(HMaterial material)
	: m_Material(material)
{
	CreateTextures();
};

CMaterialInstance::~CMaterialInstance()
{
	DestroyTextures();
}

void CMaterialInstance::Reload()
{
	CreateTextures();
}

void CMaterialInstance::Bind()
{
	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_Textures[i]);
	}
}

void CMaterialInstance::Unbind()
{
	for (size_t i = 0; i < m_Textures.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void CMaterialInstance::CreateTexture(size_t index, HTexture texture)
{
	GLint mode = 0;

	// We assume 8-bits per pixel *always*
	switch (texture->m_BytesPerPixel)
	{
	case 1:
		mode = GL_R;
		break;
	case 2:
		mode = GL_RG;
		break;
	case 3:
		mode = GL_RGB;
		break;
	case 4:
		mode = GL_RGBA;
		break;
	default: printf("Invalid texture: %hhu\n", texture->m_BytesPerPixel); break;
	}

	glBindTexture(GL_TEXTURE_2D, m_Textures[index]);
	glTexImage2D(GL_TEXTURE_2D, 0, mode, texture->m_Width, texture->m_Height, 0, mode, GL_UNSIGNED_BYTE, texture->m_Buffer.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void CMaterialInstance::CreateTextures()
{
	DestroyTextures();
	glGenTextures(m_Textures.size(), m_Textures.data());

	if (m_Material->m_AlbedoTexture != nullptr)
	{
		CreateTexture(0, m_Material->m_AlbedoTexture);
	}

	if (m_Material->m_DetailTexture != nullptr)
	{
		CreateTexture(1, m_Material->m_DetailTexture);
	}
}

void CMaterialInstance::DestroyTextures()
{
	glDeleteTextures(m_Textures.size(), m_Textures.data());
	m_Textures.fill(0);
}
}  // namespace NRender
