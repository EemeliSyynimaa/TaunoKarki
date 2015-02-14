#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <string>
#include "GL\glew.h"

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();

	GLuint& getProgramID() { return programID; }
	void loadShaders(const std::string& vertexPath, const std::string& fragmentPath);
	std::string readFile(const std::string& path);
	void bind() const;
	void unbind() const;
private:
	GLuint programID;
};

#endif