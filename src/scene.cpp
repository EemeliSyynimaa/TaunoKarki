#include "scene.h"
#include "game.h"

Scene::Scene(Game& game) : game(game), step(game.getStep()), accumulator(0.0f) 
{
}