#ifndef _SHADER_H_
#define _SHADER_H_

#include "Bits.h"
#include "Wrapped_GL.h"
#include "Error.h"
#include <unordered_map>
#include <utility>

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

    /** Uniform setting functions **/
    inline void SetUniform(GLint location, GLint value) const
    {
        glUniform1i(location, value);
        LGL_AnyGLErrorMsgOnly
    }

    inline void SetUniform(GLint location, GLfloat value) const
    {
        glUniform1f(location, value);
        LGL_AnyGLErrorMsgOnly
    }

    /**
     * Set uniform value from uniform variable name 'vname'.
     */
    inline void SetUniform(const char *vname, GLint value) const
    {
        GLint location = glGetUniformLocation(program, vname);
        glUniform1i(location, value);
        LGL_AnyGLErrorMsgOnly
    }

    /**
     * Set uniform value from uniform variable name vname.
     * It will save vanem into hashmap if not exist yet for later more efficient updating
     * value into the same uniform variable again.
     */
    inline void SetUniform(const char *vname, GLfloat value)
    {
        // try to get such vname's location from hashmap first
        const auto e = vnamesHashmap.find(vname);
        if (e != vnamesHashmap.end())
        {
            // FOUND!
            // directly use cached value to update to uniform variable
            glUniform1f(e->second, value);
            LGL_AnyGLErrorMsgOnly
        }
        else
        {
            // find such location via OpenGL
            GLint location = glGetUniformLocation(program, vname);
            if (location == -1)
            {
#ifndef LGL_NODEBUG
                lgl::error::ErrorWarn("Cannot find uniform variable location for %s of shader %u", vname, program);
#endif
            }
            else
            {
                // save into hashmap
                vnamesHashmap.emplace(std::make_pair(vname, location));
                glUniform1f(location, value);
                LGL_AnyGLErrorMsgOnly
            }
        }
    }

    /**
     * Get cached location values from input vname for uniform variable.
     * Returns LGL_FAIL if there is no such location from input vname, otherwise return its location.
     */
    inline const GLint GetUniformLocation(const char *vname)
    {
        const auto e = vnamesHashmap.find(vname);
        if (e != vnamesHashmap.end())
        {
            // FOUND!
            return e->second;
        }
        else
        {
            // NOT FOUND!
            // try finding via opengl function first
            GLint location = glGetUniformLocation(program, vname);
            if (location == -1)
            {
                // NOT FOUND
#ifndef LGL_NODEBUG
                lgl::error::ErrorWarn("Cannot find uniform variable location for %s of shader %u", vname, program);
#endif
                return LGL_FAIL;
            }
            else
            {
                // save into hashmap
                vnamesHashmap.emplace(std::make_pair(vname, location));
                return location;
            }
        }
    }

private:
    GLuint program;

    // hashmap to store input variable names mapping to its computed location
    // for fast execution and no need to call glGetUniformLocation() everytime
    // users need to update its value
    std::unordered_map<std::string, GLint> vnamesHashmap;
};

};

#endif
