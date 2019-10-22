#ifndef _ERROR_H_
#define _ERROR_H_

#include "lgl/Bits.h"
#include <iostream>
#include <cstring>
#include <cstdarg>
#include "lgl/Wrapped_GL.h"
#include "lgl/Types.h"

// based on checking status code equals to 0 or not
// for other variant, we need more of these
#ifndef LGL_NODEBUG
#define LGL_ERROR_WARN(ret, msg) if (ret != LGL_SUCCESS) { lgl::error::ErrorWarn("%s [%s:%d]", msg, __FILE__, __LINE__); }
#define LGL_ERROR_DUMP(ret, msg) if (ret != LGL_SUCCESS) { lgl::error::ErrorDump("%s [%s:%d]", msg, __FILE__, __LINE__); }
#define LGL_ERROR_QUIT(ret, msg) if (ret != LGL_SUCCESS) { lgl::error::ErrorExit("%s [%s:%d]", msg, __FILE__, __LINE__); }
#else
#define LGL_ERROR_WARN(ret, msg)
#define LGL_ERROR_DUMP(ret, msg)
#define LGL_ERROR_QUIT(ret, msg)
#endif

#ifndef LGL_NODEBUG
#define LGL_AnyGLErrorMsgOnly lgl::error::AnyGLError();
#else
#define LGL_AnyGLErrorMsgOnly
#endif

namespace lgl
{

/*
====================
Error handler
====================
*/
class error
{
public:
    static const int ERROR_BUFFER = 1024;

    static inline void ErrorWarn(const char *fmt, ...)
    {
        va_list ap;

        va_start(ap, fmt);
        error::ErrorDoIt(fmt, ap);
        va_end(ap);
    }

    static inline void ErrorDump(const char *fmt, ...)
    {
        va_list ap;

        va_start(ap, fmt);
        error::ErrorDoIt(fmt, ap);
        va_end(ap);
        std::abort();   // dump core and terminate
    }

    static inline void ErrorExit(const char *fmt, ...)
    {
        va_list ap;

        va_start(ap, fmt);
        error::ErrorDoIt(fmt, ap);
        va_end(ap);
        std::exit(1);
    }

    /**
     * Print relevant error message related to OpelGL since the last operation.
     * If it's GL_NO_ERROR then nothing will be done, otherwise error message will be printed
     * with ErrorWarn() function.
     *
     * It's meant to be called on-demand after calling relevant OpenGL function.
     */
    static inline GLint AnyGLError()
    {
        GLenum err = glGetError();
        // ref: see https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetError.xhtml for list of error message for all error code
        switch (err)
        {
            case GL_INVALID_ENUM:
#ifndef LGL_NODEBUG
                ErrorWarn("An unacceptable value is specified for an enumerated. The offending command is ignored and has no side effect than to set the error flag.");
#endif
                return static_cast<GLint>(GL_INVALID_ENUM);
            case GL_INVALID_VALUE:
#ifndef LGL_NODEBUG
                ErrorWarn("A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.");
#endif
                return static_cast<GLint>(GL_INVALID_VALUE);
            case GL_INVALID_OPERATION:
#ifndef LGL_NODEBUG
                ErrorWarn("The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.");
#endif
                return static_cast<GLint>(GL_INVALID_OPERATION);
            case GL_OUT_OF_MEMORY:
#ifndef LGL_NODEBUG
                ErrorWarn("There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded");
#endif
                return static_cast<GLint>(GL_OUT_OF_MEMORY);
            case GL_STACK_UNDERFLOW:
#ifndef LGL_NODEBUG
                ErrorWarn("An attempt has been made to perform an operation that would cause an internal stack to underflow.");
#endif
                return static_cast<GLint>(GL_STACK_UNDERFLOW);
            case GL_STACK_OVERFLOW:
#ifndef LGL_NODEBUG
                ErrorWarn("An attempt has been made to perform an operation that would cause an internal stack to overflow.");
#endif
                return static_cast<GLint>(GL_STACK_OVERFLOW);
        }

        // return success
        // success code from glGetError() is the same as of 0
        return LGL_SUCCESS;
    }

    /**
     * Return status code and print error if there's any error from OpenGL's shader compilation operation.
     * Also return status code, negative number if failed, otherwise zero for success.
     *
     * This will check only via GL_COMPILE_STATUS.
     *
     * \param shader Shader object
     * \return Return error status code in negative if error occurs, otherwise return zero for success.
     */
    static inline GLint AnyGLShaderError(const GLuint shader)
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);  

        if (!status)
        {
#ifndef LGL_NODEBUG
            glGetShaderInfoLog(shader, ERROR_BUFFER, nullptr, errLog);
            ErrorWarn("GL compilation error: %s",errLog);
#endif
            return status;
        }

        return LGL_SUCCESS;
    }

    /**
     * Return status code, and print error if there's any error occurs from OpenGL's shader program linking operation.
     * Also return the status code, negative number if failed but with specific error meaning for each case,
     * otherwise zero for success.
     *
     * This will check via its GL_LINK_STATUS only.
     *
     * \param shaderProgram Shader program object
     * \return Return error status code in negative if error occurs, otherwise return zero for success.
     */
    static inline GLint AnyGLShaderProgramError(const GLuint shaderProgram)
    {
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
        if (!status)
        {
#ifndef LGL_NODEBUG
            glGetProgramInfoLog(shaderProgram, ERROR_BUFFER, nullptr, errLog);
            ErrorWarn("GL shader program linking error: %s", errLog);
#endif
            return status;
        }

        return LGL_SUCCESS;
    }

private:
    static GLint status;
    static char errLog[ERROR_BUFFER];

    static inline void ErrorDoIt(const char *fmt, va_list ap)
    {
        char buf[ERROR_BUFFER + 1];

        int errnoSave = errno;
        std::vsnprintf( buf, ERROR_BUFFER, fmt, ap );     // don't make sense to still check if snprintf is available

        int n = strlen( buf );
        // get error string if it related to system call error
        if (!errnoSave) {
            std::snprintf( buf + n, ERROR_BUFFER - n + 1, ": %s", std::strerror( errnoSave ) );
        }
        std::strncat( buf, "\n", 1 );

        // flush relavant output streams
        std::fflush( stdout );
        std::fputs( buf, stderr );
        std::fflush( stderr );
    }
};

}

#endif // _ERROR_H_
