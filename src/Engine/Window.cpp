#include "Window.h"
#include "ShaderProgram.h"

#include <SDL_video.h>
#include <SDL_mouse.h>
#include <SDL_opengl.h>

#include <stdio.h>

bool CWindow::Initialize(const int initial_width, const int initial_height)
{
	if (m_Initialized)
	{
		return true;
	}

	m_Window = SDL_CreateWindow("", 0, 0, initial_width, initial_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!m_Window)
	{
		printf("Failed to create SDL window: %s\n", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	m_GLContext = SDL_GL_CreateContext(m_Window);

	if (!m_GLContext)
	{
		printf("Failed to create GL context: %s\n", SDL_GetError());
		SDL_DestroyWindow(m_Window);
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_Shader = new CShaderProgram("assets/shaders/basic");
	m_Initialized = true;
	return true;
}

void CWindow::Present()
{
	if (m_GLContext)
	{
		SDL_GL_SwapWindow(m_Window);
	}
}

bool CWindow::HasMouse()
{
	return SDL_GetRelativeMouseMode();
}

void CWindow::GrabMouse()
{
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_ShowCursor(0);
}

void CWindow::ReleaseMouse()
{
	SDL_SetRelativeMouseMode(SDL_FALSE);
	SDL_ShowCursor(1);
}
