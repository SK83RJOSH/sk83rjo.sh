#pragma once

#include "Utils/Singleton.h"

using SDL_GLContext = void*;

class CWindow : public TSingleton<CWindow>
{
public:
	bool Initialize(const int initial_width, const int initial_height);
	void Present();
	bool IsInitialized() const { return m_Initialized; };

	bool HasMouse();
	void GrabMouse();
	void ReleaseMouse();

private:
	bool m_Initialized = false;

	class SDL_Window* m_Window = nullptr;
	SDL_GLContext m_GLContext = nullptr;
	class CShaderProgram* m_Shader = nullptr;
};
