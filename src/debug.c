#include <stdio.h>
#include <stdarg.h>

// Todo: implement as a macro
void debug_log(s8* format, ...)
{
    // Todo: implement own vfprintf function
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}