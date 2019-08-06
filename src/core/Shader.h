#ifndef _SHADER_H_
#define _SHADER_H_

#include "Bits.h"
#include "GL_Types.h"

namespace lgl
{

class Shader
{
public:
    Shader();

    /**
     * Build shader program for this shader.
     * \return Return 0 for success, otherwise error occurs.
     */
    int Build(const char* vertexPath, const char* fragmentPath);

    /**
     * Tell OpenGL to use this shader.
     */
    void Use() const;

private:
    GL_uint program;
};

};

#endif
