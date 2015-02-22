#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "Box2D\Box2D.h"
#include "component.h"
#include "glm\glm.hpp"
#include "transform.h"

class Rigidbody : public Component
{
public:
	Rigidbody(GameObject* owner, b2World& world);
	~Rigidbody();

	void update();
	b2Body* getBody() const { return body; }
private:
	b2World& world;
	b2Body* body;
	Transform* transform;
};

#endif