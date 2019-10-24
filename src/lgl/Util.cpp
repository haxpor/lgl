#include "lgl/Util.h"
#include "lgl/Error.h"

// #define the following to bring in only what's needed for this implementation file
#define LGL_EXTERNAL_STB_IMAGE_INCLUDE
#include "lgl/External.h"

GLuint lgl::util::LoadTexture(const char *filepath)
{
    int width, height, nrChannels;
    return lgl::util::LoadTexture(filepath, &width, &height, &nrChannels);
}

GLuint lgl::util::LoadTexture(const char *filepath, int *width, int *height, int *nrChannels)
{
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filepath, width, height, nrChannels, 0);
    if (data)
    {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // set texture filtering on currently bound texture object
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLint internalformat = GL_RGB;
        GLenum format;
        // use pointer-dereference here multiple times not cache to some variable for readability
        // and concise of code
        if (*nrChannels == 1)
        {
            format = GL_RED;
        }
        else if (*nrChannels == 2)
        {
            format = GL_RG;
        }
        else if (*nrChannels == 3)
        {
            format = GL_RGB;
        }
        else if (*nrChannels == 4)
        {
            internalformat = GL_RGBA;
            format = GL_RGBA;
        }
        else
        {
            format = GL_RGB;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, *width, *height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        return texture;
    }
    else
    {
        lgl::error::ErrorWarn("Error loading ../data/container.jpg [%s]", stbi_failure_reason());
        return LGL_FAIL;
    }
}
