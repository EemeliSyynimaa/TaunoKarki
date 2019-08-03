#ifndef COMPONENT_H
#define COMPONENT_H

class GameObject;

class Component
{
public:
    Component(GameObject* owner);
    virtual ~Component();

    virtual void update(game_input*) = 0;

    GameObject* getOwner() { return owner; }
protected:
    GameObject* owner;
};

#endif