#include "playercontroller.h"
#include "gameobject.h"
#include "rigidbody.h"
#include "SDL\SDL.h"

#include "health.h"

#include "pistol.h"
#include "machinegun.h"
#include "shotgun.h"

PlayerController::PlayerController(GameObject* owner) : Component(owner),
moveSpeed(GLOBALS::PLAYER_SPEED), playerAudioChannel(-1), body(nullptr), weapon(nullptr)
{
    RigidBody* rigidbody = owner->getComponent<RigidBody>();
    assert(rigidbody);
    body = rigidbody->getBody();

    transform = owner->getComponent<Transform>();
    assert(transform);
}

PlayerController::~PlayerController()
{
    delete weapon;
}

void PlayerController::update(tk_state_player_input_t* input)
{
    b2Vec2 desiredVelocity(0.0f, 0.0f);
    b2Vec2 velocityChange(0.0f, 0.0f);
    b2Vec2 impulse(0.0f, 0.0f);
    b2Vec2 velocity = body->GetLinearVelocity();
    glm::vec3 mouseCoords(0.0f, 0.0f, 0.0f);
    int x = 0, y = 0;
    float halfX = 0.0f, halfY = 0.0f;

    if (input->player_move_left)
        desiredVelocity.x = -moveSpeed;
    else if (input->player_move_right)
        desiredVelocity.x = moveSpeed;

    if (input->player_move_up)
        desiredVelocity.y = moveSpeed;
    else if (input->player_move_down)
        desiredVelocity.y = -moveSpeed;

    if (input->player_reload && !weapon->isReloading())
        weapon->reload();

    velocityChange.x = desiredVelocity.x - velocity.x;
    velocityChange.y = desiredVelocity.y - velocity.y;

    impulse.x = body->GetMass() * velocityChange.x;
    impulse.y = body->GetMass() * velocityChange.y;

    body->ApplyLinearImpulse(impulse, body->GetWorldCenter(), true);

    // Lets check if we start or stop firing
    if (!weapon->isTriggerPulled() && input->player_shoot)
        weapon->pullTheTrigger();
    else if (!input->player_shoot)
        weapon->releaseTheTrigger();

    weapon->update();

    Camera& camera = owner->gameObjectManager.getCamera(); 

    halfX = (camera.getWidth() / 2.0f - input->mouse_x) * -1;
    halfY = (camera.getHeight() / 2.0f - input->mouse_y) * -1;
    mouseCoords = glm::vec3(halfX, -halfY, 0.0f);

    Transform* transform = owner->getComponent<Transform>();
    transform->lookAt(transform->getPosition() + mouseCoords);
    camera.follow(glm::vec2(transform->getPosition().x, transform->getPosition().y));

    if (!tk_sound_is_playing(playerAudioChannel))
    {
        playerAudioChannel = -1;
    }
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
    switch (item)
    {
    case COLLECTIBLES::PISTOL:
    {
        if (weapon->getType() == item) weapon->levelUp();
        else giveWeapon(new Pistol(owner->gameObjectManager));
        break;
    }
    case COLLECTIBLES::MACHINEGUN: {
        if (weapon->getType() == item) weapon->levelUp();
        else giveWeapon(new MachineGun(owner->gameObjectManager));
        break;
    }
    case COLLECTIBLES::SHOTGUN: {
        if (weapon->getType() == item) weapon->levelUp();
        else giveWeapon(new Shotgun(owner->gameObjectManager));
        break;
    }
    case COLLECTIBLES::HEALTHPACK: owner->getComponent<Health>()->change(GLOBALS::PLAYER_HEALTH_PER_PACK); break;
    default: break;
    }
}