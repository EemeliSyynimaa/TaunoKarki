#include "circlecollider.h"

CircleCollider::CircleCollider(GameObject* owner, float radius, uint16 categoryBits, uint16 maskBits) : Collider(owner, categoryBits, maskBits)
{
	shape.m_radius = radius;

	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.0f;
	fixtureDef.shape = &shape;
}

CircleCollider::~CircleCollider()
{
}