#include "playercontroller.h"
#include "gameobject.h"
#include "rigidbody.h"
#include "SDL\SDL.h"

#include "health.h"

#include "pistol.h"
#include "machinegun.h"
#include "shotgun.h"

PlayerController::PlayerController(GameObject* owner) : 
    Component(owner),
    moveSpeed(GLOBALS::PLAYER_SPEED),
    weapon(nullptr)
{
    transform = owner->getComponent<Transform>();
    assert(transform);
}

PlayerController::~PlayerController()
{
    delete weapon;
}

void PlayerController::update(game_input* input)
{
    f32 move_speed = GLOBALS::PLAYER_SPEED;
    glm::vec3 velocity = {};

    if (input->move_left.key_down)
    {
        velocity.x = -move_speed;
    }
    else if (input->move_right.key_down)
    {
        velocity.x = move_speed;
    }

    if (input->move_up.key_down)
    {
        velocity.y = move_speed;
    }
    else if (input->move_down.key_down)
    {
        velocity.y = -move_speed;
    }

    transform->setPosition(transform->getPosition() + velocity);

    // Lets check if we start or stop firing
    if (!weapon->isTriggerPulled() && input->shoot.key_down)
    {
        weapon->pullTheTrigger();
    }
    else if (!input->shoot.key_down)
    {
        weapon->releaseTheTrigger();
    }

    if (input->reload.key_down && !weapon->isReloading())
    {
        weapon->reload();
    }

    weapon->update(1 / 60.0f);

    Camera& camera = owner->gameObjectManager.getCamera(); 

    f32 halfX = (camera.getWidth() / 2.0f - input->mouse_x) * -1;
    f32 halfY = (camera.getHeight() / 2.0f - input->mouse_y) * -1;
    glm::vec3 mouseCoords = glm::vec3(halfX, -halfY, 0.0f);

    Transform* transform = owner->getComponent<Transform>();
    transform->lookAt(transform->getPosition() + mouseCoords);
    camera.follow(glm::vec2(transform->getPosition().x, transform->getPosition().y));
}

void PlayerController::giveWeapon(Weapon* weapon, bool instantReload)
{
    if (this->weapon) delete this->weapon;

    this->weapon = weapon; 
    this->weapon->setOwner(this->owner);
    this->weapon->reload(instantReload);
}

void PlayerController::handleItem(COLLECTIBLES item)
{
    if (weapon->getType() == item)
    {
        weapon->levelUp();
    }
    else
    {
        switch (item)
        {
            case COLLECTIBLES::PISTOL:
            {
                giveWeapon(new Pistol(owner->gameObjectManager));
            } break;
            case COLLECTIBLES::MACHINEGUN: 
            {
                giveWeapon(new MachineGun(owner->gameObjectManager));
            } break;
            case COLLECTIBLES::SHOTGUN:
            {
                giveWeapon(new Shotgun(owner->gameObjectManager));
            } break;
            case COLLECTIBLES::HEALTHPACK: 
            {
                owner->getComponent<Health>()->change(GLOBALS::PLAYER_HEALTH_PER_PACK); 
            } break;
        }
    }
}