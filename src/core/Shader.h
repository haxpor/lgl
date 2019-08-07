#ifndef _SHADER_H_
#define _SHADER_H_

#include "Bits.h"
#include "Wrapped_GL.h"

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

    /**
     * Destory states and clean up memory used by this shader.
     * Only call this after shader has been build successfully, otherwise undefine behavior as program object number can be anything.
     */
    void Destroy();

private:
    GLuint program;
};

};

#endif
