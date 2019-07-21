#include <windows.h>
#include <stdint.h>

#include "GL/glew.h"
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_events.h"

#include "game.cpp"
#include "mesh.cpp"
#include "shaderprogram.cpp"
#include "texture.cpp"
#include "aicontroller.cpp"
#include "ammobar.cpp"
#include "boxcollider.cpp"
#include "circlecollider.cpp"
#include "collectible.cpp"
#include "health.cpp"
#include "healthbar.cpp"
#include "menubutton.cpp"
#include "meshrenderer.cpp"
#include "playercontroller.cpp"
#include "rigidbody.cpp"
#include "staticbody.cpp"
#include "transform.cpp"
#include "assetmanager.cpp"
#include "audio.cpp"
#include "scenemanager.cpp"
#include "camera.cpp"
#include "collisionhandler.cpp"
#include "component.cpp"
#include "gameobject.cpp"
#include "gameobjectmanager.cpp"
#include "locator.cpp"
#include "lodepng.cpp"
#include "scene.cpp"
#include "tilemap.cpp"
#include "gamescene.cpp"
#include "menuscene.cpp"
#include "machinegun.cpp"
#include "pistol.cpp"
#include "shotgun.cpp"

#include "Box2D/Collision/Shapes/b2ChainShape.cpp"
#include "Box2D/Collision/Shapes/b2CircleShape.cpp"
#include "Box2D/Collision/Shapes/b2EdgeShape.cpp"
#include "Box2D/Collision/Shapes/b2PolygonShape.cpp"
#include "Box2D/Collision/b2BroadPhase.cpp"
#include "Box2D/Collision/b2Collision.cpp"
#include "Box2D/Collision/b2CollideCircle.cpp"
#include "Box2D/Collision/b2CollideEdge.cpp"
#include "Box2D/Collision/b2CollidePolygon.cpp"
#include "Box2D/Collision/b2Distance.cpp"
#include "Box2D/Collision/b2DynamicTree.cpp"
#include "Box2D/Collision/b2TimeOfImpact.cpp"
#include "Box2D/Dynamics/Contacts/b2ChainAndCircleContact.cpp"
#include "Box2D/Dynamics/Contacts/b2ChainAndPolygonContact.cpp"
#include "Box2D/Dynamics/Contacts/b2CircleContact.cpp"
#include "Box2D/Dynamics/Contacts/b2Contact.cpp"
#include "Box2D/Dynamics/Contacts/b2ContactSolver.cpp"
#include "Box2D/Dynamics/Contacts/b2EdgeAndCircleContact.cpp"
#include "Box2D/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp"
#include "Box2D/Dynamics/Contacts/b2PolygonAndCircleContact.cpp"
#include "Box2D/Dynamics/Contacts/b2PolygonContact.cpp"
#include "Box2D/Dynamics/Joints/b2DistanceJoint.cpp"
#include "Box2D/Dynamics/Joints/b2FrictionJoint.cpp"
#include "Box2D/Dynamics/Joints/b2GearJoint.cpp"
#include "Box2D/Dynamics/Joints/b2Joint.cpp"
#include "Box2D/Dynamics/Joints/b2MotorJoint.cpp"
#include "Box2D/Dynamics/Joints/b2MouseJoint.cpp"
#include "Box2D/Dynamics/Joints/b2PrismaticJoint.cpp"
#include "Box2D/Dynamics/Joints/b2PulleyJoint.cpp"
#include "Box2D/Dynamics/Joints/b2RevoluteJoint.cpp"
#include "Box2D/Dynamics/Joints/b2RopeJoint.cpp"
#include "Box2D/Dynamics/Joints/b2WeldJoint.cpp"
#include "Box2D/Dynamics/Joints/b2WheelJoint.cpp"
#include "Box2D/Dynamics/b2Body.cpp"
#include "Box2D/Dynamics/b2ContactManager.cpp"
#include "Box2D/Dynamics/b2Fixture.cpp"
#include "Box2D/Dynamics/b2Island.cpp"
#include "Box2D/Dynamics/b2World.cpp"
#include "Box2D/Dynamics/b2WorldCallbacks.cpp"
#include "Box2D/Rope/b2Rope.cpp"
#include "Box2D/Common/b2BlockAllocator.cpp"
#include "Box2D/Common/b2Draw.cpp"
#include "Box2D/Common/b2Math.cpp"
#include "Box2D/Common/b2Settings.cpp"
#include "Box2D/Common/b2StackAllocator.cpp"
#include "Box2D/Common/b2Timer.cpp"

tk_game_state_t state;

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
            state.running = 0;
        } break;

        case WM_DESTROY:
        {
            state.running = 0;
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
                state.running = 0;
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

    state.running = 1;

    while (state.running)
    {
        MSG msg;

        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                state.running = 0;
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
