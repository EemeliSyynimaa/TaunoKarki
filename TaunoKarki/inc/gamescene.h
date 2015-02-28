#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "SDL/SDL_events.h"
#include "scene.h"
#include "gameobject.h"
#include "component.h"
#include "texture.h"
#include "mesh.h"
#include "shaderprogram.h"
#include "tilemap.h"
#include "Box2D\Box2D.h"

class GameScene : public Scene
{
public:
	GameScene(Game& game);
	~GameScene();

	void update(float deltaTime);
	void draw();
	void handleEvent(SDL_Event& event);
private:
	ShaderProgram* shaderProgram;
	Texture* texture;
	Texture* mapTexture;
	Texture* sphereTexture;
	Mesh* mesh;
	Mesh* sphereMesh;
	Tilemap* tilemap;
	b2World world;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	std::vector<GameObject*> gameObjects;

	bool moveForward;
	bool moveBackward;
	bool turnLeft;
	bool turnRight;

	GameObject* plr;
};

#endif