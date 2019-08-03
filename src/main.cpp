
#include "platform.h"

#include "mesh.cpp"
#include "shaderprogram.cpp"
#include "texture.cpp"
#include "aicontroller.cpp"
#include "guibar.cpp"
#include "ammobar.cpp"
#include "healthbar.cpp"
#include "collider.cpp"
#include "boxcollider.cpp"
#include "circlecollider.cpp"
#include "collectible.cpp"
#include "health.cpp"
#include "meshrenderer.cpp"
#include "playercontroller.cpp"
#include "rigidbody.cpp"
#include "staticbody.cpp"
#include "transform.cpp"
#include "assetmanager.cpp"
#include "camera.cpp"
#include "collisionhandler.cpp"
#include "component.cpp"
#include "game.cpp"
#include "gameobject.cpp"
#include "gameobjectmanager.cpp"
#include "locator.cpp"
#include "lodepng.cpp"
#include "tilemap.cpp"
#include "weapon.cpp"
#include "machinegun.cpp"
#include "pistol.cpp"
#include "shotgun.cpp"

void process_input(key_state* state, b32 is_down)
{
    if (state->key_down != is_down)
    {
        state->key_down = is_down;
        state->transitions++;
    }
}

int main(int argc, char** argv)
{
    SDL_Window* window;
    SDL_GLContext context;

    s32 screen_width = 1280;
    s32 screen_height = 720;

    s32 sdl_result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

    window = SDL_CreateWindow("Tauno Kaerki", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, screen_width, screen_height,
        SDL_WINDOW_OPENGL);

    context = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    const GLenum glewResult = glewInit();
    assert(glewResult == GLEW_OK);
    glGetError();

    s32 version_major = 0;
    s32 version_minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &version_major);
    glGetIntegerv(GL_MINOR_VERSION, &version_minor);
    printf("OpenGL context version: %d.%d\n", 
        version_major, version_minor);

    s32 samples = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    printf("Multisampling samples: %d\n", samples);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    AssetManager assets;

    Locator::init();
    Locator::provideAssetManager(&assets);

    assets.loadAssets();

    init_game(screen_width, screen_height);

    game_input old_input = {};

    f32 old_time = 0.0f;
    b32 running = 1;

    while (running)
    {
        game_input new_input = {};
        f32 new_time = SDL_GetTicks() / 1000.0f;
        new_input.delta_time = new_time - old_time;
        new_input.current_time = old_time = new_time;

        s32 keys = sizeof(new_input.keys)/sizeof(new_input.keys[0]);

        for (s32 i = 0; i < keys; i++)
        {
            new_input.keys[i].key_down = old_input.keys[i].key_down;
        }

        SDL_Event event;

        while (SDL_PollEvent(&event) == 1)
        {
            switch (event.type)
            {
                case SDL_QUIT:
                {
                    running = 0;
                } break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    if (!event.key.repeat)
                    {
                        b32 is_down = (event.key.state == SDL_PRESSED);

                        switch(event.key.keysym.sym)
                        {
                            case SDLK_UP:
                            case SDLK_w:
                            {
                                process_input(&new_input.move_up, is_down);
                            } break;
                            case SDLK_RIGHT:
                            case SDLK_d:
                            {
                                process_input(&new_input.move_right, is_down);
                            } break;
                            case SDLK_DOWN:
                            case SDLK_s:
                            {
                                process_input(&new_input.move_down, is_down);
                            } break;
                            case SDLK_LEFT:
                            case SDLK_a:
                            {
                                process_input(&new_input.move_left, is_down);
                            } break;
                            case SDLK_r:
                            {
                                process_input(&new_input.reload, is_down);
                            } break;
                            case SDLK_ESCAPE:
                            {
                                process_input(&new_input.back, is_down);
                            } break;
                        }
                    }
                } break;
            }
        }

        u32 state_mouse = SDL_GetMouseState(&new_input.mouse_x,
            &new_input.mouse_y);
        
        new_input.shoot.key_down = state_mouse & SDL_BUTTON(SDL_BUTTON_LEFT);

        // Todo: do proper mouse button

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        update_game(&new_input);

        SDL_GL_SwapWindow(window);

        old_input = new_input;
    }

    return 0;
}