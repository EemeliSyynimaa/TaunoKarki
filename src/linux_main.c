#include <X11/Xlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

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

    // Todo: check if library modified since load

    if (game_lib)
    {
        printf("Close existing lib\n"); 
        dlclose(game_lib);
        game_lib = 0;
        game_init = 0;
        game_update = 0;
    }

    game_lib = dlopen("./game.so", RTLD_LAZY);

    if (game_lib)
    {
        game_init = dlsym(game_lib, "game_init");
        game_update = dlsym(game_lib, "game_update");

        return true;
    }

    return false;
}

// Todo: linux high performance timer get
// - linux_current_time_get
// - linux_elapsed_time_get

// Todo: linux file io
// - linux_file_size_get
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

    // Todo: see if this is required
    // Note: add zero to end
    if (*bytes_read < bytes_max)
    {
        data += *bytes_read;
        *data = '\0';
        (*bytes_read)++;
    }
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

u64 linux_ticks_current_get()
{
    u32 result = 0;

    // Todo: implement

    return result;
}

u64 linux_ticks_frequency_get()
{
    u32 result = 0;

    // Todo: implement

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
        KeymapStateMask);

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

    api.time.ticks_current_get = linux_ticks_current_get;
    api.time.ticks_frequency_get = linux_ticks_frequency_get;
    
    // Todo: allocate game memory
    struct game_memory memory = { 0 };
    memory.size = 1024*1024*1024;
    memory.base = mmap(0, memory.size, PROT_READ|PROT_WRITE, 
        MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    assert(memory.base);

    b32 ready = true;

    if (!linux_game_lib_load())
    {
        LOG("Library game not found\n");
        ready = false;
    }

    if (!game_init)
    {
        LOG("Function game init not found\n");
        ready = false;
    }

    if (!game_update)
    {
        LOG("Function game update not found\n");
        ready = false;
    }

    struct game_input old_input = { 0 };

    if (ready)
    {
        // Todo: fill struct game_init
        struct game_init init = { 0 };
        init.api = api;
        init.log = linux_log;
        init.screen_width = screen_width;
        init.screen_height = screen_height;

        // Todo: crashes for now
        game_init(&memory, &init);

        // struct game_input old_input = { 0 };
        
        b32 running = true;

        while (running)
        {
            struct game_input new_input = { 0 };

            s32 num_keys = sizeof(new_input.keys)/sizeof(new_input.keys[0]);

            for (s32 i = 0; i < num_keys; i++)
            {
                new_input.keys[i].key_down = old_input.keys[i].key_down;
                new_input.keys[i].transitions = old_input.keys[i].transitions;
            }

            new_input.enable_debug_rendering = old_input.enable_debug_rendering;
            new_input.pause = old_input.pause;

            // Todo: fill struct game_input
            // Todo: calculate delta time

            // Todo: read keyboard events
            // Todo: read mouse events
            // Todo: read other events?
            u32 events_num = 0;
            u32 events_processed = 0;

            // Todo: how to do this properly?
            // while ((events_num = XPending(display)))
            {
                LOG("Processing event %u of %u\n", ++events_processed,
                    events_num);
                XNextEvent(display, &ev);

                switch (ev.type)
                {
                    case KeymapNotify:
                    {
                        LOG("Keymap notify event\n");
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
                            LOG("Key %s pressed/released\n", str);

                            if (sym == XK_A || sym == XK_a)
                            {
                                LOG("A - %s\n", is_down ? "down" : "up");
                                linux_input_process(&new_input.move_left, 
                                    is_down);
                            }
                        }
                    } break;
                    default:
                    {
                        LOG("Unknown event\n");
                    } break;
                }
            }

            // Todo: recording
            game_update(&memory, &new_input);

            glXSwapBuffers(display, window);

            old_input = new_input;
        }
    }
    else
    {
        LOG("Initialization not ready, cannot run game\n");
    }


    return 0;
}
