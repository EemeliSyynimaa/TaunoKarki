#include <X11/Xlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define GL_GLEXT_PROTOTYPES

#include "GL/gl.h"
#include "GL/glx.h"
#include "GL/glext.h"
#include "tk_platform.h"

typedef void type_game_init(struct game_memory*, struct game_init*);
typedef void type_game_update(struct game_memory*, struct game_input*);
type_game_init* game_init;
type_game_update* game_update;

void linux_log(char* format, ...)
{
    // Todo: implement own printf functions
    char buffer[4096] = { 0 };
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, 4096, format, args);
    va_end(args);

    fprintf(stdout, "%s", buffer);
    fflush(stdout);
}

b32 linux_game_lib_load()
{
    static void* game_lib = 0;
    static struct timespec game_lib_write_time_last = { 0 };
    char game_lib_name[] = "./game.so";
    struct timespec game_lib_write_time = { 0 };

    s32 fd = open(game_lib_name, O_RDONLY);
    if (!fd)
    {
        s32 error = errno;
        LOG("Error: cannot open file: %d (%s)\n", error, strerror(error));
    }
    else
    {
        struct stat statbuf;
        fstat(fd, &statbuf);
        game_lib_write_time = statbuf.st_mtim;
        close(fd);
    }

    if (game_lib_write_time.tv_sec != game_lib_write_time_last.tv_sec ||
        game_lib_write_time.tv_nsec != game_lib_write_time_last.tv_nsec)
    {
        game_lib_write_time_last = game_lib_write_time;

        LOG("Trying to load new game lib...");

        if (game_lib)
        {
            dlclose(game_lib);
            game_lib = 0;
            game_init = 0;
            game_update = 0;
        }

        game_lib = dlopen(game_lib_name, RTLD_LAZY);

        if (game_lib)
        {
            game_init = dlsym(game_lib, "game_init");
            game_update = dlsym(game_lib, "game_update");

            LOG("done\n");

            return true;
        }
    }

    return false;
}

void linux_file_open(file_handle* file, char* path, b32 read)
{
    s32 fd;

    if (read)
    {
        fd = open(path, O_RDONLY);
    }
    else
    {
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    }

    *file = (u64)fd;
}

void linux_file_close(file_handle* file)
{
    s32* fd = (s32*)file;

    close(*fd);
}

void linux_file_read(file_handle* file, s8* data, u64 bytes_max,
    u64* bytes_read)
{
    *bytes_read = 0;
    s32* fd = (s32*)file;

    *bytes_read = read(*fd, data, bytes_max);

    LOG("Read %llu/%llu bytes\n", *bytes_read, bytes_max)
}

void linux_file_write(file_handle* file, s8* data, u64 bytes)
{
    s32* fd = (s32*)file;

    u64 num_bytes_written = write(*fd, data, bytes);

    LOG("Wrote %llu/%llu bytes\n", num_bytes_written, bytes)
}

void linux_file_size_get(file_handle* file, u64* file_size)
{
    s32* fd = (s32*)file;

    struct stat statbuf;

    fstat(*fd, &statbuf);

    *file_size = (u64)statbuf.st_size;
}

// Todo: this is not linux specific
void linux_input_process(struct key_state* state, b32 is_down)
{
    if (state->key_down != is_down)
    {
        state->key_down = is_down;
        state->transitions++;
    }
}

u64 linux_ticks_get()
{
    u64 result = 0;

    struct timespec tp = { 0 };

    // Todo: currently in nano resolution, is it necessary?
    if (!clock_gettime(CLOCK_REALTIME, &tp))
    {
        result = tp.tv_sec;
        result *= 1000000000;
        result += tp.tv_nsec;
    }

    return result;
}

// Todo: linux recording
// - linux_recorded_memory_read
// - linux_recorded_memory_write
// - struct linux_recorded_inputs
// - linux_recorded_inputs_read
// - linux_recorded_inputs_write

#define OPEN_GL_FUNCTION_COPY(name) api.gl.name = name

s32 main(s32 argc, char *argv[])
{
    _log = linux_log;

    Display* display;
    Screen* screen;
    XVisualInfo* visual;
    Window window;
    s32 screen_id;
    XEvent ev;

    display = XOpenDisplay(NULL);

    if (!display) 
    {
        LOG("No display, terminating\n");
        return 1;
    }

    screen = DefaultScreenOfDisplay(display);

    if (!screen)
    {
        LOG("No screen, terminating\n");
        return 1;
    }

    screen_id = DefaultScreen(display);

    s32 glx_version_major = 0;
    s32 glx_version_minor = 0;

    glXQueryVersion(display, &glx_version_major, &glx_version_minor);

    LOG("GLX: %d.%d\n", glx_version_major, glx_version_minor);

    s32 visual_attribs[] =
    {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_DEPTH_SIZE, 24,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_SAMPLE_BUFFERS, 0,
        GLX_SAMPLES, 0, None
    };

    visual = glXChooseVisual(display, screen_id, visual_attribs);

    if (!visual)
    {
        LOG("No visual, terminating\n");
        return 1;
    }

    s32 screen_width = 1280;
    s32 screen_height = 720;

    // Todo: confirm these attributes
    XSetWindowAttributes window_attribs = { 0 };
    window_attribs.border_pixel = BlackPixel(display, screen_id);
    window_attribs.background_pixel = WhitePixel(display, screen_id);
    window_attribs.override_redirect = True;
    window_attribs.event_mask = ExposureMask;
    window_attribs.colormap = XCreateColormap(display,
        RootWindowOfScreen(screen), visual->visual, AllocNone);

    window = XCreateWindow(display, RootWindowOfScreen(screen), 0, 0,
        screen_width, screen_height, 0, visual->depth, InputOutput,
        visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask,
        &window_attribs);

    GLXContext context = glXCreateContext(display, visual, NULL, GL_TRUE);
    glXMakeCurrent(display, window, context);

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask |
        KeymapStateMask | ButtonPressMask | ButtonReleaseMask |
        PointerMotionMask);

    XClearWindow(display, window);
    XMapRaised(display, window);
    XStoreName(display, window, "tk");

    struct api api;

    OPEN_GL_FUNCTION_COPY(glGetUniformLocation);
    OPEN_GL_FUNCTION_COPY(glCreateProgram);
    OPEN_GL_FUNCTION_COPY(glCreateShader);
    OPEN_GL_FUNCTION_COPY(glShaderSource);
    OPEN_GL_FUNCTION_COPY(glCompileShader);
    OPEN_GL_FUNCTION_COPY(glGetShaderiv);
    OPEN_GL_FUNCTION_COPY(glAttachShader);
    OPEN_GL_FUNCTION_COPY(glLinkProgram);
    OPEN_GL_FUNCTION_COPY(glGetProgramiv);
    OPEN_GL_FUNCTION_COPY(glDeleteShader);
    OPEN_GL_FUNCTION_COPY(glDeleteProgram);
    OPEN_GL_FUNCTION_COPY(glUseProgram);
    OPEN_GL_FUNCTION_COPY(glDeleteBuffers);
    OPEN_GL_FUNCTION_COPY(glBindBuffer);
    OPEN_GL_FUNCTION_COPY(glEnableVertexAttribArray);
    OPEN_GL_FUNCTION_COPY(glDisableVertexAttribArray);
    OPEN_GL_FUNCTION_COPY(glVertexAttribPointer);
    OPEN_GL_FUNCTION_COPY(glVertexAttribIPointer);
    OPEN_GL_FUNCTION_COPY(glUniform1i);
    OPEN_GL_FUNCTION_COPY(glUniform4fv);
    OPEN_GL_FUNCTION_COPY(glUniformMatrix4fv);
    OPEN_GL_FUNCTION_COPY(glGenBuffers);
    OPEN_GL_FUNCTION_COPY(glBufferData);
    OPEN_GL_FUNCTION_COPY(glBufferSubData);
    OPEN_GL_FUNCTION_COPY(glGenVertexArrays);
    OPEN_GL_FUNCTION_COPY(glBindVertexArray);
    OPEN_GL_FUNCTION_COPY(glActiveTexture);
    OPEN_GL_FUNCTION_COPY(glVertexAttribDivisor);
    OPEN_GL_FUNCTION_COPY(glDrawElementsInstanced);
    OPEN_GL_FUNCTION_COPY(glTexImage3D);
    OPEN_GL_FUNCTION_COPY(glTexSubImage3D);
    OPEN_GL_FUNCTION_COPY(glGetUniformBlockIndex);
    OPEN_GL_FUNCTION_COPY(glUniformBlockBinding);
    OPEN_GL_FUNCTION_COPY(glBindBufferRange);

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

    api.file.file_open = linux_file_open;
    api.file.file_close = linux_file_close;
    api.file.file_read = linux_file_read;
    api.file.file_size_get = linux_file_size_get;

    api.time.ticks_get = linux_ticks_get;
    
    // Todo: allocate game memory
    struct game_memory memory = { 0 };
    memory.size = 1024*1024*1024;
    memory.base = mmap(0, memory.size, PROT_READ|PROT_WRITE, 
        MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    assert(memory.base);

    // Todo: fill struct game_init
    struct game_init init = { 0 };
    init.api = api;
    init.log = linux_log;
    init.screen_width = screen_width;
    init.screen_height = screen_height;
    init.init_time = linux_ticks_get();

    struct game_input old_input = { 0 };
    u64 old_time = linux_ticks_get();

    b32 running = true;

    while (running)
    {
        struct game_input new_input = { 0 };

        u64 new_time = linux_ticks_get();
        f32 delta_time = (new_time - old_time) / 1000000000.0f;

        old_time = new_time;

        s32 num_keys = sizeof(new_input.keys)/sizeof(new_input.keys[0]);

        for (s32 i = 0; i < num_keys; i++)
        {
            new_input.keys[i].key_down = old_input.keys[i].key_down;
            new_input.keys[i].transitions = old_input.keys[i].transitions;
        }

        new_input.enable_debug_rendering = old_input.enable_debug_rendering;
        new_input.pause = old_input.pause;
        new_input.mouse_x = old_input.mouse_x;
        new_input.mouse_y = old_input.mouse_y;

        u32 events_num = 0;

        while ((events_num = XPending(display)))
        {
            XNextEvent(display, &ev);

            switch (ev.type)
            {
                case KeymapNotify:
                {
                } break;
                case ButtonPress:
                case ButtonRelease:
                {
                    b32 is_down = ev.type == ButtonPress;

                    if (ev.xbutton.button == Button1)
                    {
                        LOG("Mouse left - %s\n", is_down ? "down" : "up");
                        linux_input_process(&new_input.shoot, is_down);
                    }
                    else if (ev.xbutton.button == Button4)
                    {
                        LOG("Mouse scroll up\n");
                        new_input.mouse_wheel_delta += 1;
                    }
                    else if (ev.xbutton.button == Button5)
                    {
                        LOG("Mouse scroll down\n");
                        new_input.mouse_wheel_delta -= 1;
                    }
                } break;
                case MotionNotify:
                {
                    new_input.mouse_x = ev.xmotion.x;
                    new_input.mouse_y = ev.xmotion.y;
                } break;
                case KeyPress:
                case KeyRelease:
                {
                    b32 is_down = ev.type == KeyPress;
                    KeySym sym = 0;
                    char str[25] = { 0 };

                    if (XLookupString(&ev.xkey, str, 25, &sym, NULL))
                    {
                        // Todo: continue key processing
                        if (sym == XK_Escape)
                        {
                            LOG("ESCAPE - %s\n", is_down ? "down" : "up");
                            running = false;
                        }
                        else if (sym == XK_A || sym == XK_a)
                        {
                            LOG("A - %s\n", is_down ? "down" : "up");
                            linux_input_process(&new_input.move_left, is_down);
                        }
                        else if (sym == XK_S || sym == XK_s)
                        {
                            LOG("S - %s\n", is_down ? "down" : "up");
                            linux_input_process(&new_input.move_down, is_down);
                        }
                        else if (sym == XK_D || sym == XK_d)
                        {
                            LOG("D - %s\n", is_down ? "down" : "up");
                            linux_input_process(&new_input.move_right, is_down);
                        }
                        else if (sym == XK_W || sym == XK_w)
                        {
                            LOG("W - %s\n", is_down ? "down" : "up");
                            linux_input_process(&new_input.move_up, is_down);
                        }
                        else if (sym == XK_R || sym == XK_r)
                        {
                            LOG("R - %s\n", is_down ? "down" : "up");
                            linux_input_process(&new_input.reload, is_down);
                        }
                        else if (sym == XK_KP_1)
                        {
                            LOG("1 - %s\n", is_down ? "down" : "up");
                            linux_input_process(&new_input.weapon_slot_1,
                                is_down);
                        }
                        else if (sym == XK_KP_2)
                        {
                            LOG("2 - %s\n", is_down ? "down" : "up");
                            linux_input_process(&new_input.weapon_slot_2,
                                is_down);
                        }
                        else if (sym == XK_KP_3)
                        {
                            LOG("3 - %s\n", is_down ? "down" : "up");
                            linux_input_process(&new_input.weapon_slot_3,
                                is_down);
                        }
                        else if (sym == XK_KP_4)
                        {
                            LOG("4 - %s\n", is_down ? "down" : "up");
                            linux_input_process(&new_input.weapon_slot_4,
                                is_down);
                        }
                        else if (sym == XK_KP_5)
                        {
                            LOG("5 - %s\n", is_down ? "down" : "up");
                            linux_input_process(&new_input.weapon_slot_5,
                                is_down);
                        }
                        else if (sym == XK_E || sym == XK_e)
                        {
                            LOG("E - %s\n", is_down ? "down" : "up");
                            linux_input_process(&new_input.weapon_pick,
                                is_down);
                        }
                    }
                } break;
            }
        }

        if (linux_game_lib_load())
        {
            game_init(&memory, &init);
        }

        new_input.delta_time = delta_time;

        game_update(&memory, &new_input);

        glXSwapBuffers(display, window);

        old_input = new_input;
    }

    return 0;
}
