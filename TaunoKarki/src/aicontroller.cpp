#include "aicontroller.h"

AIController::AIController(GameObject* owner) : Component(owner)
{
	state = WANDER;
}

AIController::~AIController()
{
}

void AIController::update(float deltaTime)
{
	switch (state)
	{
	case WANDER: wander(); break;
	case ATTACK: attack(); break;
	case PURSUE: pursue(); break;
	case ESCAPE: escape(); break;
	default: break;
	}
}

void AIController::wander()
{
}

void AIController::attack()
{
}

void AIController::pursue()
{
}

void AIController::escape()
{
}