#include "tk_platform.h"
#include <X11/Xlib.h>

int main(int argc, char *argv[]) {
  Display *display;
  Window window;
  Screen *screen;
  s32 screen_id;
  XEvent ev;

  display = XOpenDisplay(NULL);
  if (!display) {
    return 1;
  }

  screen = DefaultScreenOfDisplay(display);
  screen_id = DefaultScreen(display);

  window = XCreateSimpleWindow(display, RootWindowOfScreen(screen), 0, 0, 1280,
                               720, 1, BlackPixel(display, screen_id),
                               WhitePixel(display, screen_id));

  XClearWindow(display, window);
  XMapRaised(display, window);

  while (true) {
    XNextEvent(display, &ev);
  }

  XDestroyWindow(display, window);
  XFree(screen);
  XCloseDisplay(display);

  return 0;
}