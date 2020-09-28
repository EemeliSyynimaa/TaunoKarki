#include <X11/Xlib.h>
#include <dlfcn.h>
#include "tk_platform.h"

typedef void type_game_init(struct game_memory*, struct game_init*);
typedef void type_game_update(struct game_memory*, struct game_input*);
type_game_init* game_init;
type_game_update* game_update;

// Todo: linux logging
// - linux_log

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
// - linux_file_open
// - linux_file_close
// - linux_file_read
// - linux_file_size_get

// Todo: linux recording
// - linux_recorded_memory_read
// - linux_recorded_memory_write
// - struct linux_recorded_inputs
// - linux_recorded_inputs_read
// - linux_recorded_inputs_write

// Todo: get rid of globals
struct file_functions file;
struct opengl_functions gl;

s32 main(s32 argc, char *argv[])
{
    Display* display;
    Window window;
    Screen* screen;
    s32 screen_id;
    XEvent ev;

    display = XOpenDisplay(NULL);

    if (!display) 
    {
        return 1;
    }

    screen = DefaultScreenOfDisplay(display);
    screen_id = DefaultScreen(display);

    window = XCreateSimpleWindow(display, RootWindowOfScreen(screen), 0, 0,
        1280, 720, 1, BlackPixel(display, screen_id), 
        WhitePixel(display, screen_id));

    XClearWindow(display, window);
    XMapRaised(display, window);

    // Todo: copy opengl function addresses
    // Todo: copy file io addresses
    
    // Todo: fill struct game_init
    // struct game_init init = { 0 };
    // init.file = 0;
    // init.gl = 0;
    // init.log = 0;
    // init.screen_width = 0;
    // init.screen_height = 0;

    // Todo: allocate game memory

    if (linux_game_lib_load())
    {
        printf("GAME LIB LOADED\n");
    }
    else
    {
        printf("GAME LIB NOT LOADED\n");
    }

    if (game_init)
    {
        printf("Game init found yeees\n");
    }

    if (game_update)
    {
        printf("Game update found yeees\n");
    }
    
    while (true)
    {
        // Todo: fill struct game_input
        // Todo: calculate delta time
        // Todo: read keyboard events
        // Todo: read mouse events
        // Todo: read other events?

        // Todo: recording

        // Todo: game_update
        XNextEvent(display, &ev);
    }

    return 0;
}