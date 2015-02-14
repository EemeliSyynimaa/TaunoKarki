#include "shaderprogram.h"
#include <cassert>
#include <fstream>

ShaderProgram::ShaderProgram() : programID(glCreateProgram())
{
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(programID);
}

void ShaderProgram::loadShaders(const std::string& vertexPath, const std::string& fragmentPath)
{
	GLint result = GL_FALSE;
	std::string temp;
	const char* source;

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	temp = readFile(vertexPath);
	source = temp.c_str();
	glShaderSource(vertexShader, 1, &source, nullptr);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	assert(result == GL_TRUE);

	temp = readFile(fragmentPath);
	source = temp.c_str();
	glShaderSource(fragmentShader, 1, &source, nullptr);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	assert(result == GL_TRUE);
	
	glAttachShader(programID, vertexShader);
	glAttachShader(programID, fragmentShader);

	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	assert(result == GL_TRUE);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

std::string ShaderProgram::readFile(const std::string& path)
{
	std::string source;
	std::ifstream fileStream(path, std::ios::binary);

	if (fileStream.good())
	{
		fileStream.seekg(0u, std::ios::end);
		source.reserve(static_cast<unsigned int>(fileStream.tellg()));
		fileStream.seekg(0u, std::ios::beg);

		source.assign(std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>());

		assert(fileStream.good());
	}

	return source;
}

void ShaderProgram::bind() const {
	glUseProgram(programID);
}

void ShaderProgram::unbind() const {
	glUseProgram(0);
}