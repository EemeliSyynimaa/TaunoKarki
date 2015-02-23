#include "circlecollider.h"

CircleCollider::CircleCollider(GameObject* owner, float radius) : Collider(owner)
{
	shape.m_radius = radius;

	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.0f;
	fixtureDef.shape = &shape;
}

CircleCollider::~CircleCollider()
{
}