#include <windows.h>
#include <stdio.h>

#include "GL/glew.h"
#include "GL/wglew.h"

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

b32 running;
LARGE_INTEGER queryPerformanceFrequency;

void process_input(key_state* state, b32 is_down)
{
    if (state->key_down != is_down)
    {
        state->key_down = is_down;
        state->transitions++;
    }
}

LARGE_INTEGER get_current_time()
{
    LARGE_INTEGER result;

    QueryPerformanceCounter(&result);
    
    return result;
}

f32 get_elapsed_time(LARGE_INTEGER start, LARGE_INTEGER end)
{
    f32 result;

    result = (f32)(end.QuadPart - start.QuadPart) / (f32)queryPerformanceFrequency.QuadPart;

    return result;
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static PAINTSTRUCT ps = {};

    switch (uMsg)
    {
        case WM_CREATE:
        {
        } break;

        case WM_CLOSE:
        {
            running = 0;
        } break;

        case WM_DESTROY:
        {
            running = 0;
        } break;

        case WM_PAINT:
        {
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
        } break;

        case WM_KEYDOWN:
        {
            if (wParam == VK_ESCAPE)
            {
                running = 0;
            }
        } break;

        default:
        {
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
        }
    }

    return 0;
}

s32 CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Todo: clean code
    WNDCLASSA dummy_class = {};
    dummy_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    dummy_class.lpfnWndProc = DefWindowProcA;
    dummy_class.hInstance = hInstance;
    dummy_class.lpszClassName = "dummy_wgl_class";

    RegisterClassA(&dummy_class);

    HWND dummy_window = CreateWindowExA(0, dummy_class.lpszClassName, "Dummy OpenGl Window", 0, 
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, dummy_class.hInstance, 0);

    HDC dummy_dc = GetDC(dummy_window);

    PIXELFORMATDESCRIPTOR dummy_pfd = {};
    dummy_pfd.nSize = sizeof(dummy_pfd);
    dummy_pfd.nVersion = 1;
    dummy_pfd.iPixelType = PFD_TYPE_RGBA;
    dummy_pfd.cColorBits = 32;
    dummy_pfd.cAlphaBits = 8;
    dummy_pfd.cDepthBits = 24;
    dummy_pfd.iLayerType = PFD_MAIN_PLANE;
    dummy_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

    s32 dummy_pf = ChoosePixelFormat(dummy_dc, &dummy_pfd);

    assert(dummy_pf != 0);

    s32 dummy_result = SetPixelFormat(dummy_dc, dummy_pf, &dummy_pfd);

    assert(dummy_result);

    HGLRC dummy_context = wglCreateContext(dummy_dc);

    wglMakeCurrent(dummy_dc, dummy_context);

    glewExperimental = GL_TRUE;
    const GLenum glewResult = glewInit();
    assert(glewResult == GLEW_OK);
    
    assert(!glGetError());

    s32 pf = 0;
    s32 screen_width = 1280;
    s32 screen_height = 720;
    
    HWND hwnd = 0;
    HDC hdc = 0;
    HGLRC hrc = 0;
    WNDCLASSEXA wdx = {};
    PIXELFORMATDESCRIPTOR pfd = {};

    (void)hPrevInstance;
    (void)lpCmdLine;

    wdx.cbSize = sizeof(wdx);
    wdx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wdx.lpfnWndProc = MainWindowProc;
    wdx.hInstance = hInstance;
    wdx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wdx.lpszClassName = "tauno_karki_window_class";

    RegisterClassExA(&wdx);

    hwnd = CreateWindowExA(0, wdx.lpszClassName, "TaunoKarki", WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, screen_width, screen_height, 0, 0, hInstance, 0);

    hdc = GetDC(hwnd);

    s32 pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_ALPHA_BITS_ARB, 8,
        0
    };

    s32 pixel_format;
    u32 num_formats;
    
    assert(wglChoosePixelFormatARB(hdc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats));

    DescribePixelFormat(hdc, pixel_format, sizeof(pfd), &pfd);
    
    SetPixelFormat(hdc, pixel_format, &pfd);

    s32 attribs[] =
    {
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB | WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    hrc = wglCreateContextAttribsARB(hdc, 0, attribs);

    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);

    assert(hrc);

    assert(wglMakeCurrent(hdc, hrc));
    
    ShowWindow(hwnd, nCmdShow);

    QueryPerformanceFrequency(&queryPerformanceFrequency);

    s32 version_major = 0;
    s32 version_minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &version_major);
    glGetIntegerv(GL_MINOR_VERSION, &version_minor);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    fprintf(stderr, "OpenGL %i.%i\n", version_major, version_minor);

    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    AssetManager assets;

    Locator::init();
    Locator::provideAssetManager(&assets);

    assets.loadAssets();

    init_game(screen_width, screen_height);

    game_input old_input = {};

    LARGE_INTEGER old_time = get_current_time();
    running = true;

    while (running)
    {
        // Todo: add keyboard input
        // Todo: add mouse input
        game_input new_input = {};
        LARGE_INTEGER new_time = get_current_time();
        new_input.delta_time = get_elapsed_time(old_time, new_time);
        old_time = new_time;

        MSG msg;

        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = 0;
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }


        // s32 keys = sizeof(new_input.keys)/sizeof(new_input.keys[0]);

        // for (s32 i = 0; i < keys; i++)
        // {
        //     new_input.keys[i].key_down = old_input.keys[i].key_down;
        // }

        // SDL_Event event;

        // while (SDL_PollEvent(&event) == 1)
        // {
        //     switch (event.type)
        //     {
        //         case SDL_QUIT:
        //         {
        //             running = 0;
        //         } break;
        //         case SDL_KEYDOWN:
        //         case SDL_KEYUP:
        //         {
        //             if (!event.key.repeat)
        //             {
        //                 b32 is_down = (event.key.state == SDL_PRESSED);

        //                 switch(event.key.keysym.sym)
        //                 {
        //                     case SDLK_UP:
        //                     case SDLK_w:
        //                     {
        //                        process_input(&new_input.move_up, is_down);
        //                     } break;
        //                     case SDLK_RIGHT:
        //                     case SDLK_d:
        //                     {
        //                         process_input(&new_input.move_right, is_down);
        //                     } break;
        //                     case SDLK_DOWN:
        //                     case SDLK_s:
        //                     {
        //                         process_input(&new_input.move_down, is_down);
        //                     } break;
        //                     case SDLK_LEFT:
        //                     case SDLK_a:
        //                     {
        //                         process_input(&new_input.move_left, is_down);
        //                     } break;
        //                     case SDLK_r:
        //                     {
        //                         process_input(&new_input.reload, is_down);
        //                     } break;
        //                     case SDLK_ESCAPE:
        //                     {
        //                         process_input(&new_input.back, is_down);
        //                     } break;
        //                 }
        //             }
        //         } break;
        //     }
        // }

        // u32 state_mouse = SDL_GetMouseState(&new_input.mouse_x,
        //     &new_input.mouse_y);
        
        // new_input.shoot.key_down = state_mouse & SDL_BUTTON(SDL_BUTTON_LEFT);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        update_game(&new_input);

        SwapBuffers(hdc);

        old_input = new_input;
    }

    return 0;
}
