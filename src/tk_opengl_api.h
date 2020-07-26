#define OPEN_GL_FUNCTION(name) type_##name* name

#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER                   0x8892
#define GL_TEXTURE0                       0x84C0
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_STATIC_DRAW                    0x88E4
#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
#define GL_DEPTH_TEST                     0x0B71
#define GL_LESS                           0x0201
#define GL_CULL_FACE                      0x0B44
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_2_BYTES                        0x1407
#define GL_3_BYTES                        0x1408
#define GL_4_BYTES                        0x1409
#define GL_DOUBLE                         0x140A
#define GL_TRUE                           1
#define GL_FALSE                          0
#define GL_TEXTURE_2D                     0x0DE1
#define GL_TRIANGLES                      0x0004
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_RGBA                           0x1908

typedef u64 GLsizeiptr;
typedef u32 GLenum;
typedef u32 GLbitfield;
typedef u32 GLuint;
typedef s32 GLint;
typedef s32 GLsizei;
typedef f32 GLfloat;
typedef f32 GLclampf;
typedef u16 GLushort;
typedef s8 GLchar;
typedef u8 GLboolean;
typedef s8 GLbyte;
typedef u8 GLubyte;
typedef void GLvoid;

typedef GLint type_glGetUniformLocation(GLuint program, const GLchar* name);
typedef GLuint type_glCreateProgram(void);
typedef GLuint type_glCreateShader(GLenum type);
typedef void type_glShaderSource(GLuint shader, GLsizei count,
    const GLchar* const* string, const GLint* length);
typedef void type_glCompileShader(GLuint shader);
typedef void type_glGetShaderiv(GLuint shader, GLenum pname, GLint* params);
typedef void type_glAttachShader(GLuint program, GLuint shader);
typedef void type_glLinkProgram(GLuint program);
typedef void type_glGetProgramiv(GLuint program, GLenum pname, GLint* params);
typedef void type_glDeleteShader(GLuint shader);
typedef void type_glDeleteProgram(GLuint program);
typedef void type_glUseProgram(GLuint program);
typedef void type_glDeleteBuffers(GLsizei n, const GLuint* buffers);
typedef void type_glBindBuffer(GLenum target, GLuint buffer);
typedef void type_glEnableVertexAttribArray(GLuint index);
typedef void type_glDisableVertexAttribArray(GLuint index);
typedef void type_glVertexAttribPointer(GLuint index, GLint size,
    GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void type_glUniform1i(GLint location, GLint v0);
typedef void type_glUniform4fv(GLint location, GLsizei count,
    const GLfloat* value);
typedef void type_glUniformMatrix4fv(GLint location, GLsizei count,
    GLboolean transpose, const GLfloat* value);
typedef void type_glGenBuffers(GLsizei n, GLuint* buffers);
typedef void type_glBufferData(GLenum target, GLsizeiptr size,
    const void* data, GLenum usage);
typedef void type_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void type_glBindVertexArray(GLuint array);
typedef void type_glActiveTexture(GLenum texture);
typedef void type_glGetIntegerv(GLenum pname, GLint* data);
typedef void type_glEnable(GLenum cap);
typedef void type_glDisable(GLenum cap);
typedef void type_glDepthFunc(GLenum func);
typedef void type_glClearColor(GLfloat red, GLfloat green, GLfloat blue, 
    GLfloat alpha);
typedef void type_glClear(GLbitfield mask);
typedef void type_glBindTexture(GLenum target, GLuint texture);
typedef void type_glDrawElements(GLenum mode, GLsizei count, GLenum type, 
    const void* indices);
typedef void type_glGenTextures(GLsizei n, GLuint* textures);
typedef void type_glTexParameteri(GLenum target, GLenum pname, GLint param);
typedef void type_glTexImage2D(GLenum target, GLint level,
    GLint internalformat, GLsizei width, GLsizei height, GLint border, 
    GLenum format, GLenum type, const void* data);

struct opengl_functions
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
    OPEN_GL_FUNCTION(glUniform4fv);
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
    OPEN_GL_FUNCTION(glBindTexture);
    OPEN_GL_FUNCTION(glDrawElements);
    OPEN_GL_FUNCTION(glGenTextures);
    OPEN_GL_FUNCTION(glTexParameteri);
    OPEN_GL_FUNCTION(glTexImage2D);
};
