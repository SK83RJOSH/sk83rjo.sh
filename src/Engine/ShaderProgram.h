#pragma once

#include <SDL_opengl.h>

class CShaderProgram
{
public:
	CShaderProgram(const char* source);
	~CShaderProgram();

private:
	GLuint m_VertexShader = 0;
	GLuint m_FragmentShader = 0;
	GLuint m_Program = 0;
};
