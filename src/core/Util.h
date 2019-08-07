#ifndef _UTIL_H_
#define _UTIL_H_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "Bits.h"
#include "Wrapped_GL.h"
#include "Types.h"

// This is not thread-safe as with the nature of OpenGL's fetching error status.
OUTER_NAMESPACE(lgl)
namespace util
{
const int ERROR_BUFFER = 512;
extern GLint status;
extern char errLog[ERROR_BUFFER];

/**
 * Print error if there's any error from OpenGL's shader compilation operation.
 * Also return status code, negative number if failed, otherwise zero for success.
 *
 * This will check only via GL_COMPILE_STATUS.
 *
 * \param shader Shader object
 * \return Return error status code in negative if error occurs, otherwise return zero for success.
 */
inline GLint PrintGLShaderErrorIfAny(const GLuint shader)
{
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);  

    if (!status)
    {
        glGetShaderInfoLog(shader, ERROR_BUFFER, nullptr, errLog);
        std::cerr << "GL compilation error: " << errLog << std::endl;
        return status;
    }

    return LGL_SUCCESS;
}

/**
 * Print error if there's any error occurs from OpenGL's shader program linking operation.
 * Also return the status code, negative number if failed but with specific error meaning for each case,
 * otherwise zero for success.
 *
 * This will check via its GL_LINK_STATUS only.
 *
 * \param shaderProgram Shader program object
 * \return Return error status code in negative if error occurs, otherwise return zero for success.
 */
inline GLint PrintGLShaderProgramErrorIfAny(const GLuint shaderProgram)
{
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (!status)
    {
        glGetProgramInfoLog(shaderProgram, ERROR_BUFFER, nullptr, errLog);
        std::cerr << "GL shader program linking error: " << errLog << std::endl;
        return status;
    }

    return LGL_SUCCESS;
}

/* 
====================
File reader
====================
*/
class FileReader
{
public:
    FileReader() { };

    /**
     * Read Ascii text from file.
     *
     * \param filePath File path to read text from
     * \param error Error flag to be set if there's any error occur, otherwise it's set to false.
     * \return All read text
     */
    inline const std::string ReadAll(const char* filePath, bool& error) const
    {
        std::string line;
        std::ifstream file(filePath);
        std::stringstream ss;
        
        if (file.is_open())
        {
            while (std::getline(file, line))
            {
                ss << line << std::endl;
            }
            
            file.close();
            error = false;
        }
        else
        {
            error = true;
        }

        return ss.str();
    }
};

};
END_NAMESPACE(lgl)

#endif
