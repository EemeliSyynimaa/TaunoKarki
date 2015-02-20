#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "scene.h"
#include "gameobject.h"
#include "component.h"
#include "texture.h"
#include "mesh.h"
#include "shaderprogram.h"

class GameScene : public Scene
{
public:
	GameScene(Game& game);
	~GameScene();

	void update();
	void draw();
private:
	ShaderProgram* shaderProgram;
	Texture* texture;
	Mesh* mesh;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	std::vector<GameObject*> gameObjects;
};

#endif