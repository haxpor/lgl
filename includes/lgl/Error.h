#ifndef _ERROR_H_
#define _ERROR_H_

#include "Bits.h"
#include <iostream>
#include <cstring>
#include <cstdarg>
#include "Wrapped_GL.h"

// based on checking status code equals to 0 or not
// for other variant, we need more of these
#define LGL_ERROR_WARN(ret, msg) if (ret != 0) { lgl::error::ErrorWarn("%s [%s:%d]", msg, __FILE__, __LINE__); }
#define LGL_ERROR_DUMP(ret, msg) if (ret != 0) { lgl::error::ErrorDump("%s [%s:%d]", msg, __FILE__, __LINE__); }
#define LGL_ERROR_QUIT(ret, msg) if (ret != 0) { lgl::error::ErrorExit("%s [%s:%d]", msg, __FILE__, __LINE__); }

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
    static inline void PrintOpenglErrorIfAny()
    {
        GLenum err = glGetError();
        // ref: see https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetError.xhtml for list of error message for all error code
        switch (err)
        {
            case GL_INVALID_ENUM:
                ErrorWarn("An unacceptable value is specified for an enumerated. The offending command is ignored and has no side effect than to set the error flag.");
                break;
            case GL_INVALID_VALUE:
                ErrorWarn("A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.");
                break;
            case GL_INVALID_OPERATION:
                ErrorWarn("The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.");
                break;
            case GL_OUT_OF_MEMORY:
                ErrorWarn("There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded");
                break;
            case GL_STACK_UNDERFLOW:
                ErrorWarn("An attempt has been made to perform an operation that would cause an internal stack to underflow.");
                break;
            case GL_STACK_OVERFLOW:
                ErrorWarn("An attempt has been made to perform an operation that would cause an internal stack to overflow.");
                break;
        }
    }

private:
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
