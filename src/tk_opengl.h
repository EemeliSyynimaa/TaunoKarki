#include "tk_opengl_api.h"

#define GL_VERTEX_SHADER                        0x8B31
#define GL_FRAGMENT_SHADER                      0x8B30
#define GL_COMPILE_STATUS                       0x8B81
#define GL_LINK_STATUS                          0x8B82
#define GL_ELEMENT_ARRAY_BUFFER                 0x8893
#define GL_ARRAY_BUFFER                         0x8892
#define GL_TEXTURE0                             0x84C0
#define GL_DYNAMIC_DRAW                         0x88E8
#define GL_STATIC_DRAW                          0x88E4
#define GL_MAJOR_VERSION                        0x821B
#define GL_MINOR_VERSION                        0x821C
#define GL_DEPTH_TEST                           0x0B71
#define GL_LESS                                 0x0201
#define GL_CULL_FACE                            0x0B44
#define GL_COLOR_BUFFER_BIT                     0x00004000
#define GL_DEPTH_BUFFER_BIT                     0x00000100
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
OPEN_GL_FUNCTION(glBindTexture);
OPEN_GL_FUNCTION(glDrawElements);
OPEN_GL_FUNCTION(glGenTextures);
OPEN_GL_FUNCTION(glTexParameteri);
OPEN_GL_FUNCTION(glTexImage2D);

#define OPEN_GL_FUNCTION_COPY(name) name = gl->##name

void opengl_functions_set(opengl_functions* gl)
{
    LOG("Copying opengl functions...");
    OPEN_GL_FUNCTION_COPY(glGetUniformLocation);
    OPEN_GL_FUNCTION_COPY(glCreateProgram);
    OPEN_GL_FUNCTION_COPY(glCreateShader);
    OPEN_GL_FUNCTION_COPY(glShaderSource);
    OPEN_GL_FUNCTION_COPY(glCompileShader);
    OPEN_GL_FUNCTION_COPY(glGetShaderiv);
    OPEN_GL_FUNCTION_COPY(glAttachShader);
    OPEN_GL_FUNCTION_COPY(glLinkProgram);
    OPEN_GL_FUNCTION_COPY(glGetProgramiv);
    OPEN_GL_FUNCTION_COPY(glDeleteShader);
    OPEN_GL_FUNCTION_COPY(glDeleteProgram);
    OPEN_GL_FUNCTION_COPY(glUseProgram);
    OPEN_GL_FUNCTION_COPY(glDeleteBuffers);
    OPEN_GL_FUNCTION_COPY(glBindBuffer);
    OPEN_GL_FUNCTION_COPY(glEnableVertexAttribArray);
    OPEN_GL_FUNCTION_COPY(glDisableVertexAttribArray);
    OPEN_GL_FUNCTION_COPY(glVertexAttribPointer);
    OPEN_GL_FUNCTION_COPY(glUniform1i);
    OPEN_GL_FUNCTION_COPY(glUniformMatrix4fv);
    OPEN_GL_FUNCTION_COPY(glGenBuffers);
    OPEN_GL_FUNCTION_COPY(glBufferData);
    OPEN_GL_FUNCTION_COPY(glGenVertexArrays);
    OPEN_GL_FUNCTION_COPY(glBindVertexArray);
    OPEN_GL_FUNCTION_COPY(glActiveTexture);
    OPEN_GL_FUNCTION_COPY(glGetIntegerv);
    OPEN_GL_FUNCTION_COPY(glEnable);
    OPEN_GL_FUNCTION_COPY(glDisable);
    OPEN_GL_FUNCTION_COPY(glDepthFunc);
    OPEN_GL_FUNCTION_COPY(glClearColor);
    OPEN_GL_FUNCTION_COPY(glClear);
    OPEN_GL_FUNCTION_COPY(glBindTexture);
    OPEN_GL_FUNCTION_COPY(glDrawElements);
    OPEN_GL_FUNCTION_COPY(glGenTextures);
    OPEN_GL_FUNCTION_COPY(glTexParameteri);
    OPEN_GL_FUNCTION_COPY(glTexImage2D);
    LOG("done\n");
}