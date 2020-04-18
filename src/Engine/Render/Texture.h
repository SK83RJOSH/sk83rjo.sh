#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace NRender
{
struct STexture
{
	std::string m_Name;
	uint16_t m_Width = 0;
	uint16_t m_Height = 0;
	uint8_t m_BytesPerPixel = 0;
	std::vector<uint8_t> m_Buffer;
};
}  // namespace NRender
