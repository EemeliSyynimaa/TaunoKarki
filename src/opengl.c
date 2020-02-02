#define GL_VERTEX_SHADER                       0x8B31
#define GL_FRAGMENT_SHADER                     0x8B30
#define GL_COMPILE_STATUS                      0x8B81
#define GL_LINK_STATUS                         0x8B82
#define GL_ELEMENT_ARRAY_BUFFER                0x8893
#define GL_ARRAY_BUFFER                        0x8892
#define GL_TEXTURE0                            0x84C0
#define GL_DYNAMIC_DRAW                        0x88E8
#define GL_STATIC_DRAW                         0x88E4
#define GL_MAJOR_VERSION                       0x821B
#define GL_MINOR_VERSION                       0x821C

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

#define OpenGLFunction(name) type_##name* name;
#define LoadOpenGLFunction(name) name = \
    (type_##name*)wglGetProcAddress(#name);

// typedef signed   long long int khronos_intptr_t;
// typedef unsigned long long int khronos_uintptr_t;
// typedef signed   long long int khronos_ssize_t;
// typedef unsigned long long int khronos_usize_t;

typedef long long int GLsizeiptr;
typedef char GLchar;

typedef HGLRC WINAPI type_wglCreateContextAttribsARB(HDC hDC, 
    HGLRC hShareContext, const int *attribList);
typedef BOOL WINAPI type_wglChoosePixelFormatARB(HDC hdc,
    const int *piAttribIList, const float *pfAttribFList, UINT nMaxFormats,
    int *piFormats, UINT *nNumFormats);

typedef GLint WINAPI type_glGetUniformLocation (GLuint program,
    const GLchar *name);
typedef GLuint WINAPI type_glCreateProgram (void);
typedef GLuint WINAPI type_glCreateShader (GLenum type);
typedef void WINAPI type_glShaderSource (GLuint shader, GLsizei count,
    const GLchar *const*string, const GLint *length);
typedef void WINAPI type_glCompileShader (GLuint shader);
typedef void WINAPI type_glGetShaderiv (GLuint shader, GLenum pname,
    GLint *params);
typedef void WINAPI type_glAttachShader (GLuint program, GLuint shader);
typedef void WINAPI type_glLinkProgram (GLuint program);
typedef void WINAPI type_glGetProgramiv (GLuint program, GLenum pname,
    GLint *params);
typedef void WINAPI type_glDeleteShader (GLuint shader);
typedef void WINAPI type_glDeleteProgram (GLuint program);
typedef void WINAPI type_glUseProgram (GLuint program);
typedef void WINAPI type_glDeleteBuffers (GLsizei n, const GLuint *buffers);
typedef void WINAPI type_glBindBuffer (GLenum target, GLuint buffer);
typedef void WINAPI type_glEnableVertexAttribArray (GLuint index);
typedef void WINAPI type_glDisableVertexAttribArray (GLuint index);
typedef void WINAPI type_glVertexAttribPointer (GLuint index, GLint size,
    GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void WINAPI type_glUniform1i (GLint location, GLint v0);
typedef void WINAPI type_glUniformMatrix4fv (GLint location, GLsizei count,
    GLboolean transpose, const GLfloat *value);
typedef void WINAPI type_glGenBuffers (GLsizei n, GLuint *buffers);
typedef void WINAPI type_glBufferData (GLenum target, GLsizeiptr size,
    const void *data, GLenum usage);
typedef void WINAPI type_glGenVertexArrays (GLsizei n, GLuint *arrays);
typedef void WINAPI type_glBindVertexArray (GLuint array);
typedef void WINAPI type_glActiveTexture (GLenum texture);

OpenGLFunction(wglCreateContextAttribsARB);
OpenGLFunction(wglChoosePixelFormatARB);
OpenGLFunction(glGetUniformLocation);
OpenGLFunction(glCreateProgram);
OpenGLFunction(glCreateShader);
OpenGLFunction(glShaderSource);
OpenGLFunction(glCompileShader);
OpenGLFunction(glGetShaderiv);
OpenGLFunction(glAttachShader);
OpenGLFunction(glLinkProgram);
OpenGLFunction(glGetProgramiv);
OpenGLFunction(glDeleteShader);
OpenGLFunction(glDeleteProgram);
OpenGLFunction(glUseProgram);
OpenGLFunction(glDeleteBuffers);
OpenGLFunction(glBindBuffer);
OpenGLFunction(glEnableVertexAttribArray);
OpenGLFunction(glDisableVertexAttribArray);
OpenGLFunction(glVertexAttribPointer);
OpenGLFunction(glUniform1i);
OpenGLFunction(glUniformMatrix4fv);
OpenGLFunction(glGenBuffers);
OpenGLFunction(glBufferData);
OpenGLFunction(glGenVertexArrays);
OpenGLFunction(glBindVertexArray);
OpenGLFunction(glActiveTexture);
