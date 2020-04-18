#pragma once

namespace NRender
{
struct SMesh;
}

namespace NUtils
{
bool LoadMesh(const char* filename, const char* asset_path, NRender::SMesh& mesh);
}  // namespace NUtils
