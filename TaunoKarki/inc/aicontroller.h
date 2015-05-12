#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include <vector>
#include <SDL\SDL_timer.h>
#include "component.h"
#include "transform.h"
#include "weapon.h"
#include "rigidbody.h"

class AIController : public Component
{
public:
	AIController(GameObject* owner, Tilemap* tilemap, b2World* world);
	~AIController();

	void update();
	void giveWeapon(Weapon* weapon) { this->weapon = weapon; this->weapon->setOwner(this->owner); this->weapon->reload(true); }
	bool droppedItem;
	Weapon* getWeapon() { return weapon; }
	void getNewTarget();

	void initWander();
	void initAttack();
	void initPursue();
	void initEscape();

	void gotShot(glm::vec3 from);

	int AIAudioChannel;
private:

	struct Node
	{
		Node(unsigned int x, unsigned int y, Node* parent) : x(x), y(y), parent(parent), H(0), G(0), F(0)
		{
		}

		int x;
		int y;
		int H;
		int G;
		int F;

		Node* parent;

		bool operator==(const Node& other) const
		{
			return x == other.x && y == other.y;
		}

		bool operator<(const Node& other) const
		{
			return F < other.F;
		}

		Node(const Node& other)
		{
			x = other.x;
			y = other.y;
			H = other.H;
			G = other.G;
			F = other.F;
			parent = other.parent;
		}

		void operator=(const Node& other)
		{
			x = other.x;
			y = other.y;
			H = other.H;
			G = other.G;
			F = other.F;
			parent = other.parent;
		}
	};

	class RayCastCallback : public b2RayCastCallback
	{
	public:
		RayCastCallback() : playerIsVisible(true) {}
		bool playerIsVisible;
		float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction);
	};

	void wander();
	void attack();
	void pursue();
	void escape();

	bool calculatePath();
	void getNeighbours(Node* node, std::vector<Node*>& neighbours);
	void checkPosition(Node* parent, unsigned int x, unsigned int y, std::vector<Node*>& neighbours);
	void constructPath(Node* node);

	void shoot();
	void moveTo(glm::vec3 position);

	bool isPlayerInSight(GameObject* player);

	enum states
	{
		WANDER,
		ATTACK,
		PURSUE,
		ESCAPE
	};

	states state;
	Transform* transform;
	Weapon* weapon;
	b2Body* body;
	Tilemap* tilemap;
	b2World* world;

	std::vector<glm::vec3> path;

	glm::vec3 target;
	glm::vec3 playerLastPosition;

	Uint32 lastShot;
	float moveSpeed;
};

#endif