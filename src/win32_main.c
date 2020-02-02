#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include "gl/gl.h"

#include "platform.h"
#include "opengl.c"
#include "game.c"

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

    result = (f32)(end.QuadPart - start.QuadPart) / 
    (f32)queryPerformanceFrequency.QuadPart;

    return result;
}

void load_file(s8* path, s8* data, u64 max_bytes, u64* read_bytes)
{
    *read_bytes = 0;
    
    HANDLE file;

    file = CreateFileA((LPCSTR)path, GENERIC_READ, 0, 0, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, 0);

    if (INVALID_HANDLE_VALUE != file)
    {
        u64 num_bytes_read = 0;

        if (ReadFile(file, data, max_bytes, (LPDWORD)&num_bytes_read, 0))
        {
            fprintf(stderr, "Read %llu/%llu bytes from %s\n", num_bytes_read,
                max_bytes, path);
        }
        else
        {
            fprintf(stderr, "Could not read from file: %s\n", path);
        }

        *read_bytes = num_bytes_read;

        CloseHandle(file);
    }
    else
    {
        fprintf(stderr, "File not found: %s\n", path);
    }
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
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
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
        } break;

        default:
        {
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
        }
    }

    return 0;
}

s32 CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSA dummy_class = { 0 };
    dummy_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    dummy_class.lpfnWndProc = DefWindowProcA;
    dummy_class.hInstance = hInstance;
    dummy_class.lpszClassName = "dummy_wgl_class";

    RegisterClassA(&dummy_class);

    HWND dummy_window = CreateWindowExA(0, dummy_class.lpszClassName,
        "Dummy OpenGl Window", 0, CW_USEDEFAULT, CW_USEDEFAULT, 
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, dummy_class.hInstance, 0);

    HDC dummy_dc = GetDC(dummy_window);

    PIXELFORMATDESCRIPTOR dummy_pfd = { 0 };
    dummy_pfd.nSize = sizeof(dummy_pfd);
    dummy_pfd.nVersion = 1;
    dummy_pfd.iPixelType = PFD_TYPE_RGBA;
    dummy_pfd.cColorBits = 32;
    dummy_pfd.cAlphaBits = 8;
    dummy_pfd.cDepthBits = 24;
    dummy_pfd.iLayerType = PFD_MAIN_PLANE;
    dummy_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | 
        PFD_DOUBLEBUFFER;

    s32 dummy_pf = ChoosePixelFormat(dummy_dc, &dummy_pfd);

    assert(dummy_pf != 0);

    s32 dummy_result = SetPixelFormat(dummy_dc, dummy_pf, &dummy_pfd);

    assert(dummy_result);

    HGLRC dummy_context = wglCreateContext(dummy_dc);

    wglMakeCurrent(dummy_dc, dummy_context);

    LoadOpenGLFunction(wglCreateContextAttribsARB);
    LoadOpenGLFunction(wglChoosePixelFormatARB);

    s32 pf = 0;
    s32 screen_width = 1920;
    s32 screen_height = 1080;
    
    HWND hwnd = 0;
    HDC hdc = 0;
    HGLRC hrc = 0;
    WNDCLASSEXA wdx = { 0 };
    PIXELFORMATDESCRIPTOR pfd = { 0 };

    (void)hPrevInstance;
    (void)lpCmdLine;

    wdx.cbSize = sizeof(wdx);
    wdx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wdx.lpfnWndProc = MainWindowProc;
    wdx.hInstance = hInstance;
    wdx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wdx.lpszClassName = "tauno_karki_window_class";

    RegisterClassExA(&wdx);

    hwnd = CreateWindowExA(0, wdx.lpszClassName, "TaunoKarki", 
        WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        screen_width, screen_height, 0, 0, hInstance, 0);

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
    
    assert(wglChoosePixelFormatARB(hdc, pixel_format_attribs, 0, 1,
        &pixel_format, &num_formats));

    DescribePixelFormat(hdc, pixel_format, sizeof(pfd), &pfd);
    
    SetPixelFormat(hdc, pixel_format, &pfd);

    s32 attribs[] =
    {
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB | 
            WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    hrc = wglCreateContextAttribsARB(hdc, 0, attribs);

    LoadOpenGLFunction(glGetUniformLocation);
    LoadOpenGLFunction(glCreateProgram);
    LoadOpenGLFunction(glCreateShader);
    LoadOpenGLFunction(glShaderSource);
    LoadOpenGLFunction(glCompileShader);
    LoadOpenGLFunction(glGetShaderiv);
    LoadOpenGLFunction(glAttachShader);
    LoadOpenGLFunction(glLinkProgram);
    LoadOpenGLFunction(glGetProgramiv);
    LoadOpenGLFunction(glDeleteShader);
    LoadOpenGLFunction(glDeleteProgram);
    LoadOpenGLFunction(glUseProgram);
    LoadOpenGLFunction(glDeleteBuffers);
    LoadOpenGLFunction(glBindBuffer);
    LoadOpenGLFunction(glEnableVertexAttribArray);
    LoadOpenGLFunction(glDisableVertexAttribArray);
    LoadOpenGLFunction(glVertexAttribPointer);
    LoadOpenGLFunction(glUniform1i);
    LoadOpenGLFunction(glUniformMatrix4fv);
    LoadOpenGLFunction(glGenBuffers);
    LoadOpenGLFunction(glBufferData);
    LoadOpenGLFunction(glGenVertexArrays);
    LoadOpenGLFunction(glBindVertexArray);
    LoadOpenGLFunction(glActiveTexture);

    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);

    assert(hrc);

    assert(wglMakeCurrent(hdc, hrc));
    
    ShowWindow(hwnd, nCmdShow);

    QueryPerformanceFrequency(&queryPerformanceFrequency);

    init_game(screen_width, screen_height);

    game_input old_input = { 0 };

    LARGE_INTEGER old_time = get_current_time();
    running = true;

    while (running)
    {
        game_input new_input = { 0 };
        LARGE_INTEGER new_time = get_current_time();
        new_input.delta_time = get_elapsed_time(old_time, new_time);
        old_time = new_time;

        s32 keys = sizeof(new_input.keys)/sizeof(new_input.keys[0]);

        for (s32 i = 0; i < keys; i++)
        {
            new_input.keys[i].key_down = old_input.keys[i].key_down;
        }

        MSG msg;

        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            switch (msg.message)
            {
                case WM_QUIT:
                {
                    running = false;
                } break;
            
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                {
                    u32 key = (u32)msg.wParam;

                    b32 is_down = (msg.lParam & (1 << 31)) == 0;
                    b32 was_down = (msg.lParam & (1 << 30)) != 0;

                    if (was_down != is_down)
                    {
                        if (msg.wParam == VK_ESCAPE)
                        {
                            running = false;
                            // process_input(&new_input.back, is_down);
                            fprintf(stderr, "ESCAPE - %s\n", 
                                is_down ? "down" :"up");
                        }
                        else if (msg.wParam == 0x57)
                        {
                            process_input(&new_input.move_up, is_down);
                            fprintf(stderr, "W - %s\n",
                                is_down ? "down" : "up");
                        }
                        else if (msg.wParam == 0x41)
                        {
                            fprintf(stderr, "A - %s\n",
                                is_down ? "down" : "up");
                            process_input(&new_input.move_left, is_down);
                        }
                        else if (msg.wParam == 0x53)
                        {
                            fprintf(stderr, "S - %s\n",
                                is_down ? "down" : "up");
                            process_input(&new_input.move_down, is_down);
                        }
                        else if (msg.wParam == 0x44)
                        {
                            fprintf(stderr, "D - %s\n",
                                is_down ? "down" : "up");
                            process_input(&new_input.move_right, is_down);
                        }
                        else if (msg.wParam == 0x52)
                        {
                            fprintf(stderr, "R - %s\n",
                                is_down ? "down" : "up");
                            process_input(&new_input.reload, is_down);
                        }
                    }
                } break;

                default:
                {
                    TranslateMessage(&msg);
                    DispatchMessageA(&msg);
                }   
            }
        }

        POINT mouse;
        RECT window_rect;

        GetWindowRect(hwnd, &window_rect);
        GetCursorPos(&mouse);

        mouse.x -= window_rect.left;
        mouse.y -= window_rect.top;

        if (mouse.x < 0)
        {
            mouse.x = 0;
        }
        else if (mouse.x > screen_width)
        {
            mouse.x = screen_width;
        }

        if (mouse.y < 0)
        {
            mouse.y = 0;
        }
        else if (mouse.y > screen_height)
        {
            mouse.y = screen_height;
        }

        new_input.mouse_x = mouse.x;
        new_input.mouse_y = mouse.y;

        new_input.shoot.key_down = 
            (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

        update_game(&new_input);

        SwapBuffers(hdc);

        old_input = new_input;
    }

    return 0;
}
