#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <vector>

class Scene;

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    void update(float deltaTime, tk_state_player_input_t* input);
    void draw();

    void push(Scene* scene);
    void pop();
    void change(Scene* scene);
private:

    std::vector<Scene*> scenes;
};

#endif