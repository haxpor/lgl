#include "Util.h"

// #define the following to bring in only what's needed for this implementation file
#define LGL_EXTERNAL_STB_IMAGE_INCLUDE
#include "External.h"

using namespace lgl;

GLint util::status = 0;
char util::errLog[util::ERROR_BUFFER];
