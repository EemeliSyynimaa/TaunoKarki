#define OPEN_GL_FUNCTION(name) type_##name* name

// typedef u64 GLsizeiptr;
// typedef u32 GLenum;
// typedef u32 GLbitfield;
// typedef u32 GLuint;
// typedef s32 GLint;
// typedef s32 GLsizei;
// typedef f32 GLfloat;
// typedef f32 GLclampf;
// typedef u16 GLushort;
// typedef s8 GLchar;
// typedef u8 GLboolean;
// typedef s8 GLbyte;
// typedef u8 GLubyte;
// typedef void GLvoid;

typedef s32 type_glGetUniformLocation(u32 program, const s8* name);
typedef u32 type_glCreateProgram(void);
typedef u32 type_glCreateShader(u32 type);
typedef void type_glShaderSource(u32 shader, s32 count,
    const s8* const* string, const s32* length);
typedef void type_glCompileShader(u32 shader);
typedef void type_glGetShaderiv(u32 shader, u32 pname, s32* params);
typedef void type_glAttachShader(u32 program, u32 shader);
typedef void type_glLinkProgram(u32 program);
typedef void type_glGetProgramiv(u32 program, u32 pname, s32* params);
typedef void type_glDeleteShader(u32 shader);
typedef void type_glDeleteProgram(u32 program);
typedef void type_glUseProgram(u32 program);
typedef void type_glDeleteBuffers(s32 n, const u32* buffers);
typedef void type_glBindBuffer(u32 target, u32 buffer);
typedef void type_glEnableVertexAttribArray(u32 index);
typedef void type_glDisableVertexAttribArray(u32 index);
typedef void type_glVertexAttribPointer(u32 index, s32 size, u32 type,
    u8 normalized, s32 stride, const void* pointer);
typedef void type_glUniform1i(s32 location, s32 v0);
typedef void type_glUniformMatrix4fv(s32 location, s32 count, u8 transpose, 
    const f32* value);
typedef void type_glGenBuffers(s32 n, u32* buffers);
typedef void type_glBufferData(u32 target, u64 size,
    const void *data, u32 usage);
typedef void type_glGenVertexArrays(s32 n, u32* arrays);
typedef void type_glBindVertexArray(u32 array);
typedef void type_glActiveTexture(u32 texture);
typedef void type_glGetIntegerv(u32 pname, s32* data);
typedef void type_glEnable(u32 cap);
typedef void type_glDisable(u32 cap);
typedef void type_glDepthFunc(u32 func);
typedef void type_glClearColor(f32 red, f32 green, f32 blue, f32 alpha);
typedef void type_glClear(u32 mask);

typedef struct opengl_functions
{
    OPEN_GL_FUNCTION(glGetUniformLocation);
    OPEN_GL_FUNCTION(glCreateProgram);
    OPEN_GL_FUNCTION(glCreateShader);
    OPEN_GL_FUNCTION(glShaderSource);
    OPEN_GL_FUNCTION(glCompileShader);
    OPEN_GL_FUNCTION(glGetShaderiv);
    OPEN_GL_FUNCTION(glAttachShader);
    OPEN_GL_FUNCTION(glLinkProgram);
    OPEN_GL_FUNCTION(glGetProgramiv);
    OPEN_GL_FUNCTION(glDeleteShader);
    OPEN_GL_FUNCTION(glDeleteProgram);
    OPEN_GL_FUNCTION(glUseProgram);
    OPEN_GL_FUNCTION(glDeleteBuffers);
    OPEN_GL_FUNCTION(glBindBuffer);
    OPEN_GL_FUNCTION(glEnableVertexAttribArray);
    OPEN_GL_FUNCTION(glDisableVertexAttribArray);
    OPEN_GL_FUNCTION(glVertexAttribPointer);
    OPEN_GL_FUNCTION(glUniform1i);
    OPEN_GL_FUNCTION(glUniformMatrix4fv);
    OPEN_GL_FUNCTION(glGenBuffers);
    OPEN_GL_FUNCTION(glBufferData);
    OPEN_GL_FUNCTION(glGenVertexArrays);
    OPEN_GL_FUNCTION(glBindVertexArray);
    OPEN_GL_FUNCTION(glActiveTexture);
    OPEN_GL_FUNCTION(glGetIntegerv);
    OPEN_GL_FUNCTION(glEnable);
    OPEN_GL_FUNCTION(glDisable);
    OPEN_GL_FUNCTION(glDepthFunc);
    OPEN_GL_FUNCTION(glClearColor);
    OPEN_GL_FUNCTION(glClear);
} opengl_functions;
