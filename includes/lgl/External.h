#ifndef _EXTERNAL_H_
#define _EXTERNAL_H_

/**
 * to bring in stb_image, define LGL_EXTERNAL_STB_IMAGE_IMPLEMENTATION
 * before include this heade file.
 *
 * It's meant to be used inside implementation file, not header file.
 * This is to better for performance in compilation to bring in only what's need
 * in such implementation file.
 */
#ifdef LGL_EXTERNAL_STB_IMAGE_INCLUDE
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif


#endif // _EXTERNAL_H_
