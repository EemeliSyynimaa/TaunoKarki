#include <windows.h>

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

void process_input(key_state* state, b32 is_down)
{
    if (state->key_down != is_down)
    {
        state->key_down = is_down;
        state->transitions++;
    }
}

LRESULT CALLBACK MainWindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    static PAINTSTRUCT ps = { 0 };

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
            glClear(GL_COLOR_BUFFER_BIT);
            glBegin(GL_TRIANGLES);
            
            // Top
            glColor3f(1.0f, 1.0f, 1.0f);
            glVertex2i(1, 1);
            glColor3f(1.0f, 1.0f, 1.0f);
            glVertex2i(-1, 1);
            glColor3f(1.0f, 1.0f, 1.0f);
            glVertex2i(0, 0);
            
            // Right
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex2i(1, 1);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex2i(0, 0);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex2i(1, -1);
            
            // Bottom
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex2i(1, -1);
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex2i(0, 0);
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex2i(-1, -1);
            
            // Left
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex2i(-1, -1);
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex2i(0, 0);
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex2i(-1, 1);
            
            glEnd();
            glFlush();

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
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
    }

    return 0;
}

int CALLBACK WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    int pf = 0;
    HWND hwnd = 0;
    HDC hdc = 0;
    HGLRC hrc = 0;
    WNDCLASSEXA wdx = { 0 };
    PIXELFORMATDESCRIPTOR pfd = { 0 };

    (void)hPrevInstance;
    (void)lpCmdLine;

    wdx.cbSize = sizeof(wdx);
    wdx.style = CS_HREDRAW | CS_VREDRAW;
    wdx.lpfnWndProc = MainWindowProc;
    wdx.hInstance = hInstance;
    wdx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wdx.lpszClassName = "TaunoKarkiWindowClass";

    RegisterClassExA(&wdx);

    hwnd = CreateWindowExA(
        0,
        wdx.lpszClassName,
        "TaunoKarki",
        WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        hInstance,
        0
    );

    hdc = GetDC(hwnd);

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    pf = ChoosePixelFormat(hdc, &pfd);

    SetPixelFormat(hdc, pf, &pfd);

    DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    hrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hrc);

    ShowWindow(hwnd, nCmdShow);

    running = 1;

    while (running)
    {
        MSG msg;

        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                running = 0;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    wglMakeCurrent(0, 0);
    wglDeleteContext(hrc);

    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);

    return 0;
}
