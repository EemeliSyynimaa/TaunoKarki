#ifndef GAMESCENE_H
#define GAMESCENE_H

#include "scene.h"
#include "gameobject.h"
#include "component.h"
#include "texture.h"
#include "componentmanager.h"

// Components
#include "sprite.h"
#include "player.h"
#include "transform.h"

class GameScene : public Scene
{
public:
	GameScene(Game& game);
	~GameScene();

	void update();
	void draw();
	GameObject* addGameObject();
private:
	ShaderProgram* shaderProgram;
	Texture* texture;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	std::vector<GameObject*> gameObjects;
	ComponentManager<Sprite> spriteComponents;
	ComponentManager<Player> playerComponents;
	ComponentManager<Transform> transformComponents;
};

#endif