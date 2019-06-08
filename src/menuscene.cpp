#include "menuscene.h"
#include "game.h"
#include "gamescene.h"
#include "menubutton.h"
#include <iostream>

MenuScene::MenuScene(game_state_t* state) : Scene(state), gameObjectManager(state->assets, camera)
{
    std::cout << "MENUSCENE ALIVE" << std::endl;

    camera.createNewPerspectiveMatrix(90.0f, (float)state->screen_width, (float)state->screen_height, 0.1f, 100.0f);
    camera.createNewOrthographicMatrix((float)state->screen_width, (float)state->screen_height);
    camera.setPosition(glm::vec3(0.0f, 0.0f, 30.0f));
    camera.setOffset(0.0f, 0.0f, 0.0f);
    camera.follow(glm::vec2(0.0f, 0.0f));

    char title1[] = {
        "#####...#####...#...#...#...#...#####"
        "..#.....#...#...#...#...##..#...#...#"
        "..#.....#####...#...#...#.#.#...#...#"
        "..#.....#...#...#...#...#..##...#...#"
        "..#.....#...#...#####...#...#...#####"
    };

    char title2[] = {
        ".......#.....#..................."
        "#...#...#####...#####...#...#...#"
        "#..#....#...#...#...#...#..#....#"
        "###.....#####...####....###.....#"
        "#..#....#...#...#...#...#..#....#"
        "#...#...#...#...#...#...#...#...#"
    };

    char startGame[] = {
        "#####...#####...#####...#####...#####"
        "#.........#.....#...#...#...#.....#.."
        "#####.....#.....#####...####......#.."
        "....#.....#.....#...#...#...#.....#.."
        "#####.....#.....#...#...#...#.....#.."
    };

    char exitGame[] = {
        "#####...#...#...#...#####"
        "#........#.#....#.....#.."
        "#####.....#.....#.....#.."
        "#........#.#....#.....#.."
        "#####...#...#...#.....#.."
    };

    writeText(title1, 37, 5, glm::vec2(0.0f, 17.0f), true);
    writeText(title2, 33, 6, glm::vec2(0.0f, 4.0f), true);
    writeText(startGame, 37, 5, glm::vec2(0.0f, -10.0f), false, 0);
    writeText(exitGame, 25, 5, glm::vec2(0.0f, -17.0f), false, 1);

    glClearColor(0.5f, 0.0f, 0.5f, 0.0f);
}

MenuScene::~MenuScene()
{
    std::cout << "MENUSCENE DIE" << std::endl;
}

void MenuScene::writeText(char* text, int width, int height, glm::vec2 pos, bool title, int id)
{
    if (title)
    {
        pos.x -= (width * 2.0f / 2);
        pos.y += (height * 2.0f / 2);
    }
    else
    {
        pos.x -= width  / 2;
        pos.y += height / 2;
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (text[y*width + x] == '#')
                if (title)
                    gameObjectManager.createTitleBlock(glm::vec3(pos.x + float(x) * 2.0f, pos.y - float(y) * 2.0f, 0.0f));
                else
                    gameObjectManager.createMenuBlock(glm::vec3(pos.x + float(x), pos.y - float(y), 0.0f), id);
        }
    }
}

void MenuScene::update(float deltaTime)
{
    accumulator += deltaTime;

    while (accumulator >= state->step)
    {
        accumulator -= state->step;
        gameObjectManager.update();
    }

    gameObjectManager.interpolate(accumulator / state->step);

    if (MenuButton::startGame)
        state->scenes.change(new GameScene(state, 1, nullptr));
    else if (MenuButton::exitGame)
        state->running = 0;
}

void MenuScene::draw()
{
    gameObjectManager.draw();
}

void MenuScene::handleEvent(SDL_Event& event)
{
    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.sym == SDLK_ESCAPE)
            state->running = 0;
    }
}