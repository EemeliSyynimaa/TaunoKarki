#include "boxcollider.h"

BoxCollider::BoxCollider(GameObject* owner, float width, float height) : Collider(owner)
{
	shape.SetAsBox(width, height);

	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.0f;
	fixtureDef.shape = &shape;
}

BoxCollider::~BoxCollider()
{
}