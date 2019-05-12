#include "boxcollider.h"

BoxCollider::BoxCollider(GameObject* owner, float width, float height, uint16 categoryBits, uint16 maskBits) : Collider(owner, categoryBits, maskBits)
{
	shape.SetAsBox(width, height);

	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.0f;
	fixtureDef.shape = &shape;
}

BoxCollider::~BoxCollider()
{
}