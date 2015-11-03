#include "menuscene.h"
#include "game.h"
#include "gamescene.h"
#include "menubutton.h"
#include <iostream>

MenuScene::MenuScene(Game& game) : Scene(game), gameObjectManager(game.getAssetManager(), camera)
{
	std::cout << "MENUSCENE ALIVE" << std::endl;

	camera.createNewPerspectiveMatrix(90.0f, (float)game.getScreenWidth(), (float)game.getScreenHeight(), 0.1f, 100.0f);
	camera.createNewOrthographicMatrix((float)game.getScreenWidth(), (float)game.getScreenHeight());
	camera.setPosition(glm::vec3(0.0f, 0.0f, 30.0f));
	camera.setOffset(0.0f, 0.0f, 0.0f);
	camera.follow(glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 0.0f));

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

	while (accumulator >= step)
	{
		accumulator -= step;
		gameObjectManager.update();
	}

	gameObjectManager.interpolate(accumulator / step);

	if (MenuButton::startGame)
		game.getSceneManager().change(new GameScene(game, 1, nullptr));
	else if (MenuButton::exitGame)
		game.stop();
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
			game.stop();
	}
}