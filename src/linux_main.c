#include <X11/Xlib.h>
#include <sys/mman.h>
#include <dlfcn.h>
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
// - linux_file_open
// - linux_file_close
// - linux_file_read
// - linux_file_size_get
void linux_file_open(file_handle* file, char* path, b32 read)
{
}

void linux_file_close(file_handle* file)
{
}

void linux_file_read(file_handle* file, s8* data, u64 bytes_max,
    u64* bytes_read)
{
}

void linux_file_size_get(file_handle* file, u64* file_size)
{
}

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

    file.file_open = linux_file_open;
    file.file_close = linux_file_close;
    file.file_read = linux_file_read;
    file.file_size_get = linux_file_size_get;
    
    // Todo: fill struct game_init
    struct game_init init = { 0 };
    init.file = &file;
    init.gl = &gl;
    init.log = linux_log;
    init.screen_width = 0;
    init.screen_height = 0;

    // Todo: allocate game memory
    struct game_memory memory = { 0 };
    memory.size = 1024*1024*1024;
    memory.base = mmap(0, memory.size, PROT_READ|PROT_WRITE, 
        MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    game_init(&memory, &init);

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

    // struct game_input old_input = { 0 };
    
    while (true)
    {
        struct game_input new_input = { 0 };

        // Todo: fill struct game_input
        // Todo: calculate delta time
        // Todo: read keyboard events
        // Todo: read mouse events
        // Todo: read other events?

        // Todo: recording

        // Todo: game_update
        XNextEvent(display, &ev);

        game_update(&memory, &new_input);
    }

    return 0;
}