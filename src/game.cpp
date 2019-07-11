#include <cassert>
#include <iostream>
#include <algorithm>
#include "game.h"
#include "menuscene.h"
#include "locator.h"

SDL_Window* window;
SDL_GLContext context;

int32_t tk_current_time_get()
{
    int32_t result = SDL_GetTicks();

    return result;
}

int32_t tk_sound_play(tk_sound_sample* sample, int32_t channel, 
    int32_t loops)
{
    Mix_Chunk* chunk = (Mix_Chunk*)sample;
    return Mix_PlayChannel(channel, chunk, loops);
}

int32_t tk_sound_is_playing(int32_t channel)
{
    return Mix_Playing(channel);
}

void game_init(tk_game_state_t* state)
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

    window = SDL_CreateWindow("Tauno Kaerki", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        state->screen_width, state->screen_height, SDL_WINDOW_OPENGL );

    assert(window != nullptr);

    context = SDL_GL_CreateContext(window);

    assert(context != nullptr);

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
    Locator::provideAssetManager(&state->assets);

    state->assets.loadAssets();
    state->scenes.change(new MenuScene(state));
}

void game_deinit(tk_game_state_t* state)
{
    // Todo: necessary?
    glDeleteVertexArrays(1, &state->VAO);

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void game_run(tk_game_state_t* state)
{
    SDL_Event event;

    while (state->running)
    {
        float time_new = SDL_GetTicks() / 1000.0f;
        state->time_delta = std::min(
            time_new - state->time_current, 0.25f);
        state->time_current = time_new;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        while (SDL_PollEvent(&event) == 1)
        {
            if (event.type == SDL_QUIT)
            {
                state->running = 0;
            }
        }

        tk_state_player_input_t input = {};

        uint32_t state_mouse = SDL_GetMouseState(&input.mouse_x, &input.mouse_y);
        const uint8_t* state_keyboard = SDL_GetKeyboardState(NULL);

        input.player_move_up = state_keyboard[SDL_SCANCODE_W];
        input.player_move_down = state_keyboard[SDL_SCANCODE_S];
        input.player_move_right = state_keyboard[SDL_SCANCODE_D];
        input.player_move_left = state_keyboard[SDL_SCANCODE_A];
        input.player_reload = state_keyboard[SDL_SCANCODE_R];
        input.player_shoot = state_mouse & SDL_BUTTON(SDL_BUTTON_LEFT);

        input.menu_escape = state_keyboard[SDL_SCANCODE_ESCAPE];
        input.menu_up = state_keyboard[SDL_SCANCODE_W] ||
            state_keyboard[SDL_SCANCODE_UP];
        input.menu_down = state_keyboard[SDL_SCANCODE_S] || 
            state_keyboard[SDL_SCANCODE_DOWN];
        input.menu_confirm = state_keyboard[SDL_SCANCODE_RETURN] ||
            state_keyboard[SDL_SCANCODE_SPACE];

        state->scenes.update(state->time_delta, &input);
        state->scenes.draw();
        SDL_GL_SwapWindow(window);
    }
}