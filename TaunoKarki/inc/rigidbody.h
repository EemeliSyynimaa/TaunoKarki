#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "Box2D\Box2D.h"
#include "component.h"
#include "glm\glm.hpp"
#include "transform.h"
#include "collider.h"

class RigidBody : public Component
{
public:
	RigidBody(GameObject* owner, b2World& world);
	~RigidBody();

	void update( );
	b2Body* getBody() const { return body; }
private:
	b2World& world;
	b2Body* body;
	Transform* transform;
	Collider* collider;
};

#endif