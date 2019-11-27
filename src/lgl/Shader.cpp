#include "lgl/Shader.h"
#include "lgl/Wrapped_GL.h"
#include "lgl/Util.h"
#include "lgl/PBits.h"

using namespace lgl;

Shader::Shader()
{
}

int Shader::BuildFromSrc(const char* vertexShaderStr, const char* fragmentShaderStr)
{
    GLint glError = 0;

    // VERTEX SHADER
    // read in vertex shader from source
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // compile vertex shader
    glShaderSource(vertexShader, 1, &vertexShaderStr, NULL);
    glCompileShader(vertexShader);
    glError = error::AnyGLShaderError(vertexShader);
    if (glError != 0)
    {
#ifndef LGL_NODEBUG
        lgl::error::ErrorWarn("%s has error", vertexShader);
#endif
        return -1;
    }

    // FRAGMENT SHADER
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // compile fragment shader
    glShaderSource(fragmentShader, 1, &fragmentShaderStr, NULL);
    glCompileShader(fragmentShader);

    glError = error::AnyGLShaderError(fragmentShader);
    if (glError != 0)
    {
#ifndef LGL_NODEBUG
        lgl::error::ErrorWarn("%s has error", fragmentShader);
#endif
        return -1;
    }

    // link all shaders together
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glError = error::AnyGLShaderProgramError(program);
    if (glError != 0)
    {
        return -1;
    }

    // delete un-needed shader objects
    // note: in fact, it will mark them for deletion after our usage of shader program is done
    // they will be deleted after that
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return 0;
}

int Shader::Build(const char* vertexPath, const char* fragmentPath)
{
    util::FileReader fileReader;
    bool error = false;
    GLint glError = 0;

    // VERTEX SHADER
    // read in vertex shader from source
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vsCode = fileReader.ReadAll(vertexPath, error);
    if (error)
    {
        return -1;
    }

    // compile vertex shader
    const char* t = vsCode.c_str();
    glShaderSource(vertexShader, 1, &t, NULL);
    glCompileShader(vertexShader);
    glError = error::AnyGLShaderError(vertexShader);
    if (glError != 0)
    {
#ifndef LGL_NODEBUG
        lgl::error::ErrorWarn("%s has error", vertexPath);
#endif
        return -1;
    }

    // FRAGMENT SHADER
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fsCode = fileReader.ReadAll(fragmentPath, error);
    if (error)
    {
        return -1;
    }

    // compile fragment shader
    t = fsCode.c_str();
    glShaderSource(fragmentShader, 1, &t, NULL);
    glCompileShader(fragmentShader);

    glError = error::AnyGLShaderError(fragmentShader);
    if (glError != 0)
    {
#ifndef LGL_NODEBUG
        lgl::error::ErrorWarn("%s has error", fragmentShader);
#endif
        return -1;
    }

    // link all shaders together
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glError = error::AnyGLShaderProgramError(program);
    if (glError != 0)
    {
        return -1;
    }

    // delete un-needed shader objects
    // note: in fact, it will mark them for deletion after our usage of shader program is done
    // they will be deleted after that
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return 0;
}

void Shader::Use() const
{
    glUseProgram(program);    
}

void Shader::Destroy()
{
    glDeleteProgram(program);
    vnamesHashmap.clear();
}
