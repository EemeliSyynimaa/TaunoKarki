#include <cassert>
#include <iostream>
#include <algorithm>
#include "game.h"
#include "menuscene.h"
#include "locator.h"

void game_init(game_state_t* state)
{
    state->screen_width = 1280;
    state->screen_height = 720;
    state->running = 1;
    state->step = 1.0f / 60.0f;

    int32_t SDLResult = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    assert(SDLResult == 0);

    int32_t mixResult = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024);
    assert(mixResult == 0);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

    state->window = SDL_CreateWindow("Tauno Kaerki", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        state->screen_width, state->screen_height, SDL_WINDOW_OPENGL );

    assert(state->window != nullptr);

    state->context = SDL_GL_CreateContext(state->window);

    assert(state->context != nullptr);

    glewExperimental = GL_TRUE;
    const GLenum glewResult = glewInit();
    assert(glewResult == GLEW_OK);
    glGetError();

    int32_t version_major = 0;
    int32_t version_minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &version_major);
    glGetIntegerv(GL_MINOR_VERSION, &version_minor);
    printf("OpenGL context version: %d.%d\n", version_major, version_minor);

    int32_t samples = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    printf("Multisampling samples: %d\n", samples);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    glGenVertexArrays(1, &state->VAO);
    glBindVertexArray(state->VAO);

    Mix_Init(MIX_INIT_MOD | MIX_INIT_MP3);
    Mix_Volume(-1, MIX_MAX_VOLUME / 2);
    Mix_AllocateChannels(128);

    Locator::init();
    Locator::provideAudio(&state->audio);
    Locator::provideAssetManager(&state->assets);

    state->assets.loadAssets();
    state->scenes.change(new MenuScene(state));
}

void game_deinit(game_state_t* state)
{
    // Todo: necessary?
    Mix_CloseAudio();
    Mix_Quit();

    glDeleteVertexArrays(1, &state->VAO);

    SDL_GL_DeleteContext(state->context);
    SDL_DestroyWindow(state->window);
    SDL_Quit();
}

void game_run(game_state_t* state)
{
    float time_delta = 0.0f;
    float time_new = 0.0f;
    float time_current = SDL_GetTicks() / 1000.0f;
    SDL_Event event;

    while (state->running)
    {
        time_new = SDL_GetTicks() / 1000.0f;
        time_delta = std::min(time_new - time_current, 0.25f);
        time_current = time_new;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        while (SDL_PollEvent(&event) == 1)
        {
            if (event.type == SDL_QUIT)
            {
                state->running = 0;
            }

            state->scenes.handleEvent(event);
        }

        state->scenes.update(time_delta);
        state->scenes.draw();
        SDL_GL_SwapWindow(state->window);
    }
}