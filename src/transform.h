#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "glm\glm.hpp"
#include "component.h"

class Transform : public Component
{
public:
    Transform(GameObject* owner, float x = 0.0f, float y = 0.0f, float z = 0.0f);
    Transform(GameObject* owner, glm::vec3 position);
    ~Transform();

    void setPosition(glm::vec3& vec);
    void setRotation(float r, glm::vec3& axis);
    void setScale(glm::vec3& vec);
    void lookAt(const glm::vec3& target);
    void translate(glm::vec3& vec);
    void rotate(float r, glm::vec3& axis);

    void update(game_input* input);
    const glm::vec3 getPosition() const { return position; }
    const glm::mat4 getMatrix() const { return transform * rotation * scale; }
    const glm::vec2 getDirVec() const { return dirVec; }
    float distanceTo(glm::vec3 position);
private:
    glm::vec3 position;
    glm::mat4 transform;
    glm::mat4 scale;
    glm::mat4 rotation;
    glm::vec2 dirVec;
};

#endif