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

#define OPEN_GL_FUNCTION(name) type_##name* name;

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

typedef GLint type_glGetUniformLocation(GLuint program,
    const GLchar *name);
typedef GLuint type_glCreateProgram(void);
typedef GLuint type_glCreateShader(GLenum type);
typedef void type_glShaderSource(GLuint shader, GLsizei count,
    const GLchar *const*string, const GLint *length);
typedef void type_glCompileShader(GLuint shader);
typedef void type_glGetShaderiv(GLuint shader, GLenum pname,
    GLint *params);
typedef void type_glAttachShader(GLuint program, GLuint shader);
typedef void type_glLinkProgram(GLuint program);
typedef void type_glGetProgramiv(GLuint program, GLenum pname,
    GLint *params);
typedef void type_glDeleteShader(GLuint shader);
typedef void type_glDeleteProgram(GLuint program);
typedef void type_glUseProgram(GLuint program);
typedef void type_glDeleteBuffers(GLsizei n, const GLuint *buffers);
typedef void type_glBindBuffer(GLenum target, GLuint buffer);
typedef void type_glEnableVertexAttribArray(GLuint index);
typedef void type_glDisableVertexAttribArray(GLuint index);
typedef void type_glVertexAttribPointer(GLuint index, GLint size,
    GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void type_glUniform1i(GLint location, GLint v0);
typedef void type_glUniformMatrix4fv(GLint location, GLsizei count,
    GLboolean transpose, const GLfloat *value);
typedef void type_glGenBuffers(GLsizei n, GLuint *buffers);
typedef void type_glBufferData(GLenum target, GLsizeiptr size,
    const void *data, GLenum usage);
typedef void type_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void type_glBindVertexArray(GLuint array);
typedef void type_glActiveTexture(GLenum texture);

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
