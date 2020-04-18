#include "ShaderProgram.h"

#include <string>
#include <vector>
#include <cstdio>

static const char* vertex_suffix = ".vert";
static const char* fragment_suffix = ".frag";

GLuint LoadShader(GLenum type, const char* filename)
{
	// Create shader
	GLuint shader = glCreateShader(type);

	if (!shader)
	{
		printf("Failed to create shader: %s\n", filename);
		return 0;
	}

	// Open file
	FILE* file = fopen(filename, "rb");

	if (!file)
	{
		printf("Failed to read shader source: %s\n", filename);
		glDeleteShader(shader);
		return 0;
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);

	// Write file contents
	std::vector<GLchar> source(file_size + 1);
	rewind(file);
	fread(source.data(), 1, file_size, file);
	fclose(file);

	// Compile shader
	const GLchar* source_ptr = source.data();
	glShaderSource(shader, 1, &source_ptr, 0);
	glCompileShader(shader);

	// Check compilation status
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		GLint error_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &error_length);

		std::vector<GLchar> error(error_length);
		glGetShaderInfoLog(shader, error_length, &error_length, &error[0]);
		glDeleteShader(shader);
		printf("Failed to compiled shader (%s):\n%s\n%s\n", filename, &error[0], &source[0]);

		return 0;
	}

	return shader;
};

CShaderProgram::CShaderProgram(const char* filename)
{
	// Create vertex shader
	std::string vertex_filename = std::string(filename) + vertex_suffix;
	m_VertexShader = LoadShader(GL_VERTEX_SHADER, vertex_filename.c_str());

	if (!m_VertexShader)
	{
		return;
	}

	// Create fragment shader
	std::string fragment_filename = std::string(filename) + fragment_suffix;
	m_FragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragment_filename.c_str());

	if (!m_FragmentShader)
	{
		return;
	}

	// Create program
	m_Program = glCreateProgram();

	if (!m_Program)
	{
		printf("Failed to create program: %s\n", filename);
		return;
	}

	glAttachShader(m_Program, m_VertexShader);
	glAttachShader(m_Program, m_FragmentShader);
	glBindAttribLocation(m_Program, 0, "Position");
	glBindAttribLocation(m_Program, 1, "Normal");
	glLinkProgram(m_Program);

	GLint linked;
	glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		GLint error_length = 0;
		glGetProgramiv(m_Program, GL_INFO_LOG_LENGTH, &error_length);

		std::vector<GLchar> error(error_length);
		glGetProgramInfoLog(m_Program, error_length, &error_length, &error[0]);
		glDeleteProgram(m_Program);
		printf("Failed to link shader (%s):\n%s\n", filename, &error[0]);
		return;
	}

	glUseProgram(m_Program);
	glUniform1i(glGetUniformLocation(m_Program, "Albedo"), 0);
	glUniform1i(glGetUniformLocation(m_Program, "Detail"), 1);
}

CShaderProgram::~CShaderProgram()
{
	if (m_Program)
	{
		glDeleteProgram(m_Program);
	}

	if (m_VertexShader)
	{
		glDeleteShader(m_VertexShader);
	}

	if (m_FragmentShader)
	{
		glDeleteShader(m_FragmentShader);
	}
}
