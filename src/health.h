#ifndef HEALTH_H
#define HEALTH_H

#include "component.h"

class Health : public Component
{
public:
    Health(GameObject* owner, float max);
    ~Health();

    void update();
    void change(float amount);

    float getCurrent() { return current; }
    float getMax() { return max; }
private:
    float current;
    float max;
};

#endif