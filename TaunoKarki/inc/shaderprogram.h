#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <string>
#include "GL\glew.h"

class ShaderProgram
{
public:
	ShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);
	~ShaderProgram();

	GLuint& getID() { return programID; }
	std::string readFile(const std::string& path);
	void bind() const;
	void unbind() const;
	GLuint getUniformLocation(char* uniformId) const { return glGetUniformLocation(programID, uniformId); }
private:
	GLuint programID;
};

#endif