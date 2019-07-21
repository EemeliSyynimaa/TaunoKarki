#include "aicontroller.h"
#include "gameobject.h"
#include "rigidbody.h"
#include "gameobjectmanager.h"
#include "tilemap.h"
#include "glm/gtc/constants.hpp"
#include <iostream>
#include <queue>
#include <algorithm>

#define randomInt std::uniform_int_distribution<int>

AIController::AIController(GameObject* owner, Tilemap* tilemap, b2World* world) : 
    Component(owner),
    tilemap(tilemap),
    world(world),
    lastShot(0),
    droppedItem(false),
    moveSpeed(GLOBALS::ENEMY_SPEED),
    target(0.0f),
    playerLastPosition(0.0f),
    weapon(nullptr)
{
    transform = owner->getComponent<Transform>();
    RigidBody* rigidbody = owner->getComponent<RigidBody>();
    assert(transform && rigidbody);
    body = rigidbody->getBody();

    initWander();
}

AIController::~AIController()
{
    delete weapon;
}

void AIController::update(game_input* input)
{
    switch (state)
    {
    case WANDER: wander(); break;
    case ATTACK: attack(1 / 60.0f); break;
    case PURSUE: pursue(); break;
    case ESCAPE: escape(); break;
    default: break;
    }

    if (weapon)
        weapon->update(1 / 60.0f);
}

void AIController::wander()
{
    GameObject* player = getOwner()->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

    if (player && isPlayerInSight(player))
        initAttack();
    else
    {
        if (!path.empty() && transform->distanceTo(path.front()) < 0.1f)
        {
            path.erase(path.begin());

            if (path.empty())
                getNewTarget();
        }

        moveTo(path.front());
    }
}

void AIController::attack(f32 delta_time)
{
    GameObject* player = getOwner()->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

    if (player && isPlayerInSight(player))
    { 
        playerLastPosition = player->getComponent<Transform>()->getPosition();
        moveTo(playerLastPosition);
    
        if (weapon) shoot(delta_time);
    }
    else initPursue();
}

void AIController::shoot(f32 delta_time)
{
    switch (weapon->getType())
    {
    case COLLECTIBLES::PISTOL:
    {
        if (lastShot > 0)
        {
            lastShot -= delta_time;
        }
        else if (!weapon->isTriggerPulled())
        {
            weapon->pullTheTrigger();
            lastShot = 0.45f;
        }
        else 
        {
            weapon->releaseTheTrigger();
        } break;
    }
    case COLLECTIBLES::SHOTGUN:
    {
        if (!weapon->isTriggerPulled()) weapon->pullTheTrigger();
        else weapon->releaseTheTrigger();
        break;
    }
    case COLLECTIBLES::MACHINEGUN: weapon->pullTheTrigger(); break;
    default: break;
    }
}

void AIController::moveTo(glm::vec3 position)
{
    b2Vec2 impulse(0.0f, 0.0f);

    transform->lookAt(position);

    impulse.x = body->GetMass() * transform->getDirVec().x * moveSpeed - body->GetLinearVelocity().x;
    impulse.y = body->GetMass() * transform->getDirVec().y * moveSpeed - body->GetLinearVelocity().y;

    body->ApplyLinearImpulse(impulse, body->GetWorldCenter(), true);
}

void AIController::pursue()
{
    GameObject* player = getOwner()->gameObjectManager.getFirstObjectOfType(GAMEOBJECT_TYPES::PLAYER);

    if (player && isPlayerInSight(player))
        initAttack();
    else
    { 
        moveTo(playerLastPosition);

        if (transform->distanceTo(playerLastPosition) < 0.1f)
        {
            playerLastPosition = glm::vec3(0.0f);
            initWander();
        }
    }
}

void AIController::escape()
{
}

void AIController::initAttack()
{
    state = ATTACK;
}

void AIController::initWander()
{
    state = WANDER;

    body->SetLinearVelocity(b2Vec2_zero);

    if (weapon) weapon->releaseTheTrigger();
    
    getNewTarget();
}

void AIController::initEscape()
{
    state = ESCAPE;
}

void AIController::initPursue()
{
    state = PURSUE;

    body->SetLinearVelocity(b2Vec2_zero);

    if (weapon) weapon->releaseTheTrigger();
}

void AIController::getNewTarget()
{
    path.clear();

    bool found = false;

    do
    {
        target = tilemap->getRandomFreePosition();
        found = calculatePath();
    } while (!found);
}

bool AIController::calculatePath()
{
    Node start(int(round(transform->getPosition().x / 2)), int(round(transform->getPosition().y / 2)), nullptr);
    Node end(int(round(target.x / 2)), int((round(target.y / 2))), nullptr);

    start.G = 0;
    start.H = abs(end.x - start.x) + abs(end.y - start.y);
    start.F = start.H;

    std::vector<Node*> openSet;
    std::vector<Node*> closedSet;

    openSet.push_back(&start);
    
    while (!openSet.empty())
    {
        Node* current = openSet.front();
        openSet.erase(openSet.begin());

        if (*current == end)
        {
            constructPath(current);

            // We erase the first node.
            path.erase(path.begin());

            return !path.empty();
        }

        closedSet.push_back(current);

        std::vector<Node*> neighbours;

        getNeighbours(current, neighbours);

        for (Node* neighbour : neighbours)
        {
            if (std::find_if(closedSet.begin(), closedSet.end(), [neighbour](Node* node)
            {
                return neighbour->x == node->x && neighbour->y == node->y;
            }) != closedSet.end()) continue;

            int tentativeGScore = current->G + abs(current->x - neighbour->x) + abs(current->y - neighbour->y);

            std::vector<Node*>::iterator it = std::find_if(openSet.begin(), openSet.end(), [neighbour](Node* node)
            {
                return neighbour->x == node->x && neighbour->y == node->y;
            });

            if (it != openSet.end())
            {
                if (tentativeGScore < (*it)->G)
                {
                    (*it)->G = tentativeGScore;
                    (*it)->parent = current;
                    (*it)->F = (*it)->G + (*it)->H;
                }
            }
            else
            {
                neighbour->G = tentativeGScore;
                neighbour->H = abs(neighbour->x - end.x) + abs(neighbour->y - end.y);
                neighbour->F = neighbour->G + neighbour->H;

                openSet.push_back(neighbour);
            }

            std::sort(openSet.begin(), openSet.end());
        }
    }

    return false;
}

void AIController::getNeighbours(Node* node, std::vector<Node*>& neighbours)
{
    bool left = checkPosition(node, node->x + 1, node->y, neighbours);
    bool right = checkPosition(node, node->x - 1, node->y, neighbours);
    bool up = checkPosition(node, node->x, node->y + 1, neighbours);
    bool down = checkPosition(node, node->x, node->y - 1, neighbours);

    if (right && down) checkPosition(node, node->x - 1, node->y - 1, neighbours);
    if (right && up) checkPosition(node, node->x - 1, node->y + 1, neighbours);
    if (left && down) checkPosition(node, node->x + 1, node->y - 1, neighbours);
    if (left && up) checkPosition(node, node->x + 1, node->y + 1, neighbours);
}

bool AIController::checkPosition(Node* parent, unsigned int x, unsigned int y, std::vector<Node*>& neighbours)
{
    if (tilemap->isPositionFree(x, y))
    { 
        neighbours.push_back(new Node(x, y, parent));
        return true;
    }

    return false;
}

void AIController::constructPath(Node* node)
{
    if (node == nullptr)
        return;

    constructPath(node->parent);
    path.push_back(glm::vec3(node->x * 2.0f, node->y * 2.0f, 0.0f));
}

bool AIController::isPlayerInSight(GameObject* player)
{
    Transform* plrTransform = player->getComponent<Transform>();

    if (transform->distanceTo(plrTransform->getPosition()) < GLOBALS::ENEMY_ACTIVATION_DISTANCE)
    {
        b2Vec2 AIPos;
        AIPos.x = transform->getPosition().x;
        AIPos.y = transform->getPosition().y;

        b2Vec2 plrPos;
        plrPos.x = plrTransform->getPosition().x;
        plrPos.y = plrTransform->getPosition().y;

        RayCastCallback callBack;

        world->RayCast(&callBack, AIPos, plrPos);

        if (callBack.playerIsVisible)
        {
            glm::vec2 AIDir = transform->getDirVec();
            glm::vec2 plrDir = glm::normalize(glm::vec2(plrPos.x - AIPos.x, plrPos.y - AIPos.y));

            float dotProduct = glm::dot(AIDir, plrDir);

            if (dotProduct > 1.0f) dotProduct = 1.0f;

            if (glm::degrees(glm::acos(dotProduct)) < GLOBALS::ENEMY_ANGLE_OF_VISION) return true;
        }
    }

    return false;
}

void AIController::gotShot(glm::vec3 from)
{
    if (state != ATTACK)
        transform->lookAt(from);
}

float32 AIController::RayCastCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
{
    // Only walls dont have a pointer to a gameobject.
    // And this raycast should only fail when it hits a wall.
    // So we check if the body has a gameobject and act according to it.

    GameObject* gameObject = static_cast<GameObject*>(fixture->GetBody()->GetUserData());

    if (!gameObject)
    {
        playerIsVisible = false;
        return 0;
    }
    
    return -1;
}