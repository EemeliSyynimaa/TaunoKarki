#include "gl/gl.h"
#include "opengl_api.h"

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

// Todo:
// - siirrä opengl_functions opengl.h
// - linkkaa se tähän headeriin
// - lisää glGetIntegerv yms.
// - aseta funktioiden pointterit win32_mainissa
// - lähetä pointterit gamelle
// - game ottaa ne käyttöön