#ifndef COLLISIONHANDLER_H
#define COLLISIONHANDLER_H

class GameObject;

class CollisionHandler : public b2ContactListener
{
public:
    CollisionHandler();
    ~CollisionHandler();

    void BeginContact(b2Contact* contact);
    void EndContact(b2Contact* contact);

private:
    void handleCollision(GameObject* gameObjectA, GameObject* gameObjectB);
};

#endif