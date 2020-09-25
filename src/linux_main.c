#include <X11/Xlib.h>
#include <dlfcn.h>
#include "tk_platform.h"

typedef void type_game_init(struct game_memory*, struct game_init*);
typedef void type_game_update(struct game_memory*, struct game_input*);
type_game_init* game_init;
type_game_update* game_update;

b32 linux_game_lib_load()
{
    static void* game_lib = 0;

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
        XNextEvent(display, &ev);
    }

    XDestroyWindow(display, window);
    XFree(screen);
    XCloseDisplay(display);

    return 0;
}