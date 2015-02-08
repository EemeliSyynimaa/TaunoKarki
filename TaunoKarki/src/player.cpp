#include "player.h"
#include "ShaderSource.h"
#include <iostream>

Player::Player()
{
	std::cout << "PLAYER IS HERE" << std::endl;

	GLfloat vertexData[] = {
		0.0f, 0.0f, 0.0f,
		0.50f, 0.0f, 0.0f,
		0.0f, 0.50f, 0.0f,
		0.50f, 0.50f, 0.0f
	};

	GLushort indices[] = {
		0, 1, 2, 2, 1, 3
	};

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	std::string path;
	const char* temp;
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	path = ShaderSource::load("assets/shaders/trivial_vertex.glsl");
	temp = path.c_str();

	glShaderSource(vertexShader, 1, &temp, nullptr);
	glCompileShader(vertexShader);

	path = ShaderSource::load("assets/shaders/trivial_fragment.glsl");
	temp = path.c_str();

	glShaderSource(fragmentShader, 1, &temp, nullptr);
	glCompileShader(fragmentShader);

	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
}

Player::~Player()
{
	std::cout << "PLAYER IS DIE" << std::endl;

	glDeleteProgram(program);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
}

void Player::update()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnableVertexAttribArray(0);

	glUseProgram(program);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);

	glDisableVertexAttribArray(0);
}