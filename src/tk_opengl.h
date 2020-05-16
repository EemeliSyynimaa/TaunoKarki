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

void opengl_functions_set(struct opengl_functions* gl)
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