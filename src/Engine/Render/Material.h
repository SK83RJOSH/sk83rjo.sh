#pragma once

#include <string>
#include <memory>

namespace NRender
{
struct STexture;
using HTexture = std::shared_ptr<STexture>;

struct SMaterial
{
	std::string m_Name;
	HTexture m_AlbedoTexture;
	HTexture m_DetailTexture;
};
}  // namespace NRender
