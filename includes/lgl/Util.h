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

/**
 * Load texture with pre-filled of default texture filtering.
 *
 * \param filepath Filepath to texture asset to load
 * \return Texture object for loaded texture, error reported by opengl so check it via lgl::error::AnyGLError().
 */
GLuint LoadTexture(const char* filepath);

/**
 * Load texture with pre-filled of default texture filtering.
 *
 * \param filepath Filepath to texture asset to load
 * \param texture To be filled texture object after loading
 * \param width To be filled with width of loaded texture if load successfully, otherwise no change. width cannot be NULL.
 * \param height To be filled with height of loaded texture if load successfully, otherwise no change. height cannot be NULL.
 * \param nrChannels To be filled with number of channels of loaded texture if load successfully, otherwise no change. nrChannels cannot be NULL.
 * \return Texture object for loaded texture, if there's any error, it will be reported by OpenGL so check it via lgl::error::AnyGLError().
 */
GLuint LoadTexture(const char* filepath, int *width, int *height, int *nrChannels);

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

}
END_NAMESPACE(lgl)

#endif
