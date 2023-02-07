#include <windows.h>
#include <assert.h>

#include "gl/gl.h"
#include "tk_platform.h"

#define WGL_DRAW_TO_WINDOW_ARB                 0x2001
#define WGL_SUPPORT_OPENGL_ARB                 0x2010
#define WGL_DOUBLE_BUFFER_ARB                  0x2011
#define WGL_ACCELERATION_ARB                   0x2003
#define WGL_FULL_ACCELERATION_ARB              0x2027
#define WGL_PIXEL_TYPE_ARB                     0x2013
#define WGL_TYPE_RGBA_ARB                      0x202B
#define WGL_COLOR_BITS_ARB                     0x2014
#define WGL_DEPTH_BITS_ARB                     0x2022
#define WGL_ALPHA_BITS_ARB                     0x201B
#define WGL_CONTEXT_FLAGS_ARB                  0x2094
#define WGL_CONTEXT_DEBUG_BIT_ARB              0x00000001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_MAJOR_VERSION_ARB          0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB          0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB           0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB       0x00000001

typedef HGLRC WINAPI type_wglCreateContextAttribsARB(HDC hDC, 
    HGLRC hShareContext, const int *attribList);
typedef BOOL WINAPI type_wglChoosePixelFormatARB(HDC hdc,
    const int *piAttribIList, const float *pfAttribFList, UINT nMaxFormats,
    int *piFormats, UINT *nNumFormats);

OPEN_GL_FUNCTION(wglCreateContextAttribsARB);
OPEN_GL_FUNCTION(wglChoosePixelFormatARB);

b32 running;

// Todo: clean usage
typedef void type_game_init(struct game_memory*, struct game_init*);
typedef void type_game_update(struct game_memory*, struct game_input*);
type_game_init* game_init;
type_game_update* game_update;

void win32_log(char* format, ...)
{
    // Todo: implement own printf functions
    char buffer[4096] = { 0 };
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, 4096, format, args);
    va_end(args);

    // Note: write log to both debug and stdout
    OutputDebugStringA(buffer);
    fprintf(stdout, buffer);
    fflush(stdout);
}

// Todo: this loads a new game lib twice everytime it's changed, why?
b32 win32_game_lib_load()
{
    static HMODULE game_lib = 0;
    static FILETIME game_lib_write_time_last = { 0 };
    char game_lib_name[] = "game.dll";
    char game_lib_name_temp[] = "game-run.dll";

    WIN32_FILE_ATTRIBUTE_DATA data;
    
    GetFileAttributesExA(game_lib_name, GetFileExInfoStandard, &data);
    
    FILETIME game_lib_write_time = data.ftLastWriteTime;

    if (CompareFileTime(&game_lib_write_time, 
        &game_lib_write_time_last) != 0)
    {
        game_lib_write_time_last = game_lib_write_time;

        LOG("Trying to load new game lib...");

        if (game_lib)
        {
            FreeLibrary(game_lib);
            game_init = 0;
            game_update = 0;
        }

        CopyFileA(game_lib_name, game_lib_name_temp, FALSE);

        game_lib = LoadLibraryA(game_lib_name_temp);

        // Todo: get rid of magic strings
        game_update = (type_game_update*)GetProcAddress(game_lib, 
            "game_update");
        game_init = (type_game_init*)GetProcAddress(game_lib, "game_init");

        LOG("done\n");

        return true;
    }

    return false;
}

// Todo: this is not win32 specific
void win32_input_process(struct key_state* state, b32 is_down)
{
    if (state->key_down != is_down)
    {
        state->key_down = is_down;
        state->transitions++;
    }
}

LARGE_INTEGER win32_current_time_get()
{
    LARGE_INTEGER result;

    QueryPerformanceCounter(&result);
    
    return result;
}

// Todo: maybe support using these functions after all, no rounding
u64 win32_ticks_current_get()
{
    LARGE_INTEGER result;

    QueryPerformanceCounter(&result);

    return result.QuadPart;
}

u64 win32_ticks_frequency_get()
{
    LARGE_INTEGER result;

    QueryPerformanceFrequency(&result);

    return result.QuadPart;
}

// Todo: global for now
u64 win32_ticks_frequency = 0;


// Todo: in nanoseconds, eh
u64 win32_ticks_get()
{
    u64 result = 0;

    result = win32_ticks_current_get();

    result *= 1000000000;
    result /= win32_ticks_frequency;

    return result;
}

f32 win32_elapsed_time_get(LARGE_INTEGER frequency, LARGE_INTEGER start,
    LARGE_INTEGER end)
{
    f32 result;

    result = (f32)(end.QuadPart - start.QuadPart) / (f32)frequency.QuadPart;

    return result;
}

void win32_file_open(file_handle* file, char* path, b32 read)
{
    HANDLE win32_handle;

    if (read)
    {
        win32_handle = CreateFileA((LPCSTR)path, GENERIC_READ, 0, 0,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    }
    else
    {
        win32_handle = CreateFileA((LPCSTR)path, GENERIC_WRITE, 0, 0,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    }

    if (win32_handle != INVALID_HANDLE_VALUE)
    {
        *file = (u64)win32_handle;
    }
    else
    {
        // Todo: return error value
        LOG("Could not open file: %s\n", path);
    }
}

void win32_file_close(file_handle* file)
{
    HANDLE* win32_handle = (HANDLE*)file;

    if (INVALID_HANDLE_VALUE != *win32_handle)
    {
        CloseHandle(*win32_handle);
    }
    else
    {
        // Todo: return error value
        LOG("Handle of invalid value\n");
    }
}

void win32_file_read(file_handle* file, s8* data, u64 bytes_max, 
    u64* bytes_read)
{
    *bytes_read = 0;

    HANDLE* win32_handle = (HANDLE*)file;

    if (INVALID_HANDLE_VALUE != win32_handle)
    {
        u64 num_bytes_read = 0;

        if (ReadFile(*win32_handle, data, bytes_max, (LPDWORD)&num_bytes_read,
            0))
        {
            LOG("Read %llu/%llu bytes\n", num_bytes_read, bytes_max);
        }
        else
        {
            // Todo: return error value
            LOG("Could not read from file\n");
        }

        *bytes_read = num_bytes_read;

        // Note: add zero to end
        if (num_bytes_read < bytes_max)
        {
            data += num_bytes_read;
            *data = '\0';
            (*bytes_read)++;
        }
    }
    else
    {
        // Todo: return error value
        LOG("Handle of invalid value\n");
    }
}

void win32_file_write(file_handle* file, s8* data, u64 bytes)
{
    HANDLE* win32_handle = (HANDLE*)file;

    if (INVALID_HANDLE_VALUE != win32_handle)
    {
        u64 num_bytes_written = 0;

        if (WriteFile(*win32_handle, data, bytes, (LPDWORD)&num_bytes_written,
            0))
        {
            LOG("Wrote %llu/%llu bytes\n", num_bytes_written, bytes);
        }
        else
        {
            // Todo: return error value
            LOG("Could not write to file\n");
        }
    }
    else
    {
        // Todo: return error value
        LOG("Handle of invalid value\n");
    }
}

void win32_file_size_get(file_handle* handle, u64* file_size)
{
    HANDLE* win32_handle = (HANDLE*)handle;

    if (INVALID_HANDLE_VALUE != win32_handle)
    {
        LARGE_INTEGER size = { 0 };
        if (GetFileSizeEx(*win32_handle, &size))
        {
            // Note: reserve one byte for zero
            *file_size = size.QuadPart + 1; 
        }
        else
        {
            // Todo: return error value
            LOG("Could not read file size\n");
        }
    }
    else
    {
        // Todo: return error value
        LOG("Handle of invalid value\n");
    }
}

bool win32_recorded_memory_read(struct game_memory* memory)
{
    file_handle file;
    u64 bytes_read = 0;
    win32_file_open(&file, "recorded_memory", true);

    if (file)
    {
        win32_file_read(&file, memory->base, memory->size, &bytes_read);
        win32_file_close(&file);

        assert(bytes_read == memory->size);

        return true;
    }

    return false;
}

void win32_recorded_memory_write(struct game_memory* memory)
{
    file_handle file;
    win32_file_open(&file, "recorded_memory", false);
    win32_file_write(&file, memory->base, memory->size);
    win32_file_close(&file);
}

#define RECORDED_INPUTS_MAX 4096
struct win32_recorded_inputs
{
    u32 count;
    u32 current;
    struct game_input inputs[RECORDED_INPUTS_MAX];
};

bool win32_recorded_inputs_read(struct win32_recorded_inputs* inputs)
{
    file_handle file;
    u64 bytes_read = 0;
    win32_file_open(&file, "recorded_inputs", true);

    if (file)
    {
        win32_file_read(&file, (s8*)inputs, 
            sizeof(struct win32_recorded_inputs), &bytes_read);
        win32_file_close(&file);

        assert(bytes_read == sizeof(struct win32_recorded_inputs));

        return true;
    }

    return false;
}

void win32_recorded_inputs_write(struct win32_recorded_inputs* inputs)
{
    file_handle file;
    win32_file_open(&file, "recorded_inputs", false);
    win32_file_write(&file, (s8*)inputs, sizeof(struct win32_recorded_inputs));
    win32_file_close(&file);
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

// Todo: get rid of globals
struct win32_recorded_inputs record;

#define OPEN_GL_FUNCTION_LOAD(name) api.gl.name = \
    (type_##name*)wglGetProcAddress(#name)

#define OPEN_WGL_FUNCTION_LOAD(name) name = \
    (type_##name*)wglGetProcAddress(#name)

#define OPEN_GL_FUNCTION_COPY(name) api.gl.name = name

s32 CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine, int nCmdShow)
{
    // Todo: clean function
    _log = win32_log;

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

    OPEN_WGL_FUNCTION_LOAD(wglCreateContextAttribsARB);
    OPEN_WGL_FUNCTION_LOAD(wglChoosePixelFormatARB);

    s32 pf = 0;
    s32 screen_width = 1280;
    s32 screen_height = 1280;
    
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
        WS_VISIBLE | WS_OVERLAPPEDWINDOW, 0, 0, 
        screen_width, screen_height, 0, 0, hInstance, 0);

    RECT client_area = { 0 };
    s32 client_width = screen_width;
    s32 client_height = screen_height;

    if (GetClientRect(hwnd, &client_area))
    {
        LOG("Read client area: %dx%d\n", client_area.right, client_area.bottom);
        client_width = client_area.right;
        client_height = client_area.bottom;
    }
    else
    {
        LOG("Reading client area failed, using %dx%d\n", screen_width, 
            screen_height);
    }

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

    struct api api = { 0 };

    OPEN_GL_FUNCTION_LOAD(glGetUniformLocation);
    OPEN_GL_FUNCTION_LOAD(glCreateProgram);
    OPEN_GL_FUNCTION_LOAD(glCreateShader);
    OPEN_GL_FUNCTION_LOAD(glShaderSource);
    OPEN_GL_FUNCTION_LOAD(glCompileShader);
    OPEN_GL_FUNCTION_LOAD(glGetShaderiv);
    OPEN_GL_FUNCTION_LOAD(glAttachShader);
    OPEN_GL_FUNCTION_LOAD(glLinkProgram);
    OPEN_GL_FUNCTION_LOAD(glGetProgramiv);
    OPEN_GL_FUNCTION_LOAD(glDeleteShader);
    OPEN_GL_FUNCTION_LOAD(glDeleteProgram);
    OPEN_GL_FUNCTION_LOAD(glUseProgram);
    OPEN_GL_FUNCTION_LOAD(glDeleteBuffers);
    OPEN_GL_FUNCTION_LOAD(glBindBuffer);
    OPEN_GL_FUNCTION_LOAD(glEnableVertexAttribArray);
    OPEN_GL_FUNCTION_LOAD(glDisableVertexAttribArray);
    OPEN_GL_FUNCTION_LOAD(glVertexAttribPointer);
    OPEN_GL_FUNCTION_LOAD(glVertexAttribIPointer);
    OPEN_GL_FUNCTION_LOAD(glUniform1i);
    OPEN_GL_FUNCTION_LOAD(glUniform4fv);
    OPEN_GL_FUNCTION_LOAD(glUniformMatrix4fv);
    OPEN_GL_FUNCTION_LOAD(glGenBuffers);
    OPEN_GL_FUNCTION_LOAD(glBufferData);
    OPEN_GL_FUNCTION_LOAD(glBufferSubData);
    OPEN_GL_FUNCTION_LOAD(glGenVertexArrays);
    OPEN_GL_FUNCTION_LOAD(glBindVertexArray);
    OPEN_GL_FUNCTION_LOAD(glActiveTexture);
    OPEN_GL_FUNCTION_LOAD(glVertexAttribDivisor);
    OPEN_GL_FUNCTION_LOAD(glDrawElementsInstanced);
    OPEN_GL_FUNCTION_LOAD(glTexImage3D);
    OPEN_GL_FUNCTION_LOAD(glTexSubImage3D);
    OPEN_GL_FUNCTION_LOAD(glGetUniformBlockIndex);
    OPEN_GL_FUNCTION_LOAD(glUniformBlockBinding);
    OPEN_GL_FUNCTION_LOAD(glBindBufferRange);

    OPEN_GL_FUNCTION_COPY(glGetIntegerv);
    OPEN_GL_FUNCTION_COPY(glEnable);
    OPEN_GL_FUNCTION_COPY(glDepthFunc);
    OPEN_GL_FUNCTION_COPY(glClearColor);
    OPEN_GL_FUNCTION_COPY(glClear);
    OPEN_GL_FUNCTION_COPY(glBindTexture);
    OPEN_GL_FUNCTION_COPY(glDrawElements);
    OPEN_GL_FUNCTION_COPY(glGenTextures);
    OPEN_GL_FUNCTION_COPY(glTexParameteri);
    OPEN_GL_FUNCTION_COPY(glTexImage2D);
    OPEN_GL_FUNCTION_COPY(glBlendFunc);
    OPEN_GL_FUNCTION_COPY(glGetError);

    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_context);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);

    assert(hrc);

    assert(wglMakeCurrent(hdc, hrc));
    
    ShowWindow(hwnd, nCmdShow);

    api.file.file_open = win32_file_open;
    api.file.file_close = win32_file_close;
    api.file.file_read = win32_file_read;
    api.file.file_size_get = win32_file_size_get;

    api.time.ticks_get = win32_ticks_get;

    struct game_init init = { 0 };
    init.api = api;
    init.log = win32_log;
    init.screen_width = client_width;
    init.screen_height = client_height;

    LARGE_INTEGER query_performance_frequency;
    QueryPerformanceFrequency(&query_performance_frequency);

    win32_ticks_frequency = query_performance_frequency.QuadPart;

    struct game_memory memory = { 0 };
    memory.size = GIGABYTES(1);
    memory.base = VirtualAlloc(NULL, memory.size, MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE);

    assert(memory.base);

    struct game_input old_input = { 0 };

    LARGE_INTEGER old_time = win32_current_time_get();

    init.init_time = old_time.QuadPart;

    running = true;

    b32 recording = false;
    b32 playing = false;

    while (running)
    {
        struct game_input new_input = { 0 };

        LARGE_INTEGER new_time = win32_current_time_get();

        f32 delta_time = win32_elapsed_time_get(query_performance_frequency,
            old_time, new_time);
        old_time = new_time;

        s32 num_keys = sizeof(new_input.keys)/sizeof(new_input.keys[0]);

        for (s32 i = 0; i < num_keys; i++)
        {
            new_input.keys[i].key_down = old_input.keys[i].key_down;
            new_input.keys[i].transitions = old_input.keys[i].transitions;
        }

        new_input.enable_debug_rendering = old_input.enable_debug_rendering;
        new_input.pause = old_input.pause;

        MSG msg;

        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            switch (msg.message)
            {
                case WM_QUIT:
                {
                    running = false;
                } break;
            
                case WM_MOUSEWHEEL:
                {
                    s32 delta = GET_WHEEL_DELTA_WPARAM(msg.wParam);
                    new_input.mouse_wheel_delta = delta;
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
                            // input_process(&new_input.back, is_down);
                            LOG("ESCAPE - %s\n", 
                                is_down ? "down" :"up");
                        }
                        else if (msg.wParam == VK_F1 && !was_down)
                        {
                            new_input.enable_debug_rendering =
                                !new_input.enable_debug_rendering;
                        }
                        else if (msg.wParam == 0x50 && !was_down)
                        {
                            new_input.pause = !new_input.pause;
                        }
                        else if (msg.wParam == VK_F5 && was_down)
                        {
                            if (!recording)
                            {
                                if (playing)
                                {
                                    playing = false;

                                    for (s32 i = 0; i < num_keys; i++)
                                    {
                                        new_input.keys[i].key_down = 0;
                                        new_input.mouse_x = 0;
                                        new_input.mouse_y = 0;
                                    }
                                    LOG("Stop playing\n");
                                }
                                else
                                {
                                    if (win32_recorded_memory_read(&memory) &&
                                        win32_recorded_inputs_read(&record))
                                    {
                                        LOG("Start playing\n");
                                        playing = true;
                                    }
                                }
                            }
                        }
                        else if (msg.wParam == VK_F6 && was_down)
                        {
                            if (!playing)
                            {
                                if (recording)
                                {
                                    recording = false;
                                    win32_recorded_inputs_write(&record);
                                    LOG("Stop recording\n");
                                }
                                else
                                {
                                    win32_recorded_memory_write(&memory);
                                    LOG("Start recording\n");
                                    recording = true;
                                    record.count = 0;
                                    record.current = 0;
                                }   
                            }
                        }
                        else if (!playing)
                        {
                            b32 debug = false;

                            // read input only if we are not playing
                            if (msg.wParam == 0x57)
                            {
                                if (debug)
                                {
                                    LOG("W - %s\n", is_down ? "down" : "up");
                                }
                                win32_input_process(&new_input.move_up, 
                                    is_down);
                            }
                            else if (msg.wParam == 0x41)
                            {
                                if (debug)
                                {
                                    LOG("A - %s\n", is_down ? "down" : "up");
                                }
                                win32_input_process(&new_input.move_left, 
                                    is_down);
                            }
                            else if (msg.wParam == 0x53)
                            {
                                if (debug)
                                {
                                    LOG("S - %s\n", is_down ? "down" : "up");
                                }
                                win32_input_process(&new_input.move_down, 
                                    is_down);
                            }
                            else if (msg.wParam == 0x44)
                            {
                                if (debug)
                                {
                                    LOG("D - %s\n", is_down ? "down" : "up");
                                }
                                win32_input_process(&new_input.move_right, 
                                    is_down);
                            }
                            else if (msg.wParam == 0x52)
                            {
                                if (debug)
                                {
                                    LOG("R - %s\n", is_down ? "down" : "up");
                                }
                                win32_input_process(&new_input.reload, 
                                    is_down);
                            }
                            else if (msg.wParam == 0x31)
                            {
                                if (debug)
                                {
                                    LOG("1 - %s\n", is_down ? "down" : "up");
                                }
                                win32_input_process(&new_input.weapon_slot_1,
                                    is_down);
                            }
                            else if (msg.wParam == 0x32)
                            {
                                if (debug)
                                {
                                    LOG("2 - %s\n", is_down ? "down" : "up");
                                }
                                win32_input_process(&new_input.weapon_slot_2,
                                    is_down);
                            }
                            else if (msg.wParam == 0x33)
                            {
                                if (debug)
                                {
                                    LOG("3 - %s\n", is_down ? "down" : "up");
                                }
                                win32_input_process(&new_input.weapon_slot_3,
                                    is_down);
                            }
                            else if (msg.wParam == 0x34)
                            {
                                if (debug)
                                {
                                    LOG("4 - %s\n", is_down ? "down" : "up");
                                }
                                win32_input_process(&new_input.weapon_slot_4,
                                    is_down);
                            }
                            else if (msg.wParam == 0x35)
                            {
                                if (debug)
                                {
                                    LOG("5 - %s\n", is_down ? "down" : "up");
                                }
                                win32_input_process(&new_input.weapon_slot_5,
                                    is_down);
                            }
                            else if (msg.wParam == 0x45)
                            {
                                if (debug)
                                {
                                    LOG("E - %s\n", is_down ? "down" : "up");
                                }
                                win32_input_process(&new_input.weapon_pick,
                                    is_down);
                            }
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

        if (win32_game_lib_load())
        {
            game_init(&memory, &init);
        }

        if (!playing)
        {
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(hwnd, &mouse);

            new_input.mouse_x = mouse.x;
            new_input.mouse_y = mouse.y;

            // Todo: we would like to know if the mouse was
            // pressed and released
            new_input.shoot.key_down = 
                (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        }

        if (playing)
        {
            if (record.current >= record.count)
            {
                record.current = 0;
                win32_recorded_memory_read(&memory);
            }

            new_input = record.inputs[record.current++];
        }
        
        new_input.delta_time = delta_time;
        
        if (recording)
        {
            record.inputs[record.count++] = new_input;

            if (record.count == RECORDED_INPUTS_MAX)
            {
                LOG("Recording limit reached\n");
                LOG("Stop recording\n");
                recording = false;
            }
        }

        game_update(&memory, &new_input);

        SwapBuffers(hdc);

        old_input = new_input;
    }

    return 0;
}
