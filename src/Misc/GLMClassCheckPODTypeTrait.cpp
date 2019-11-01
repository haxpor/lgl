/**
 * Check whether GLM class i.e. vec is POD (plain old data) type.
 * As we might use such class to submit to OpenGL's vertex buffer directly, and no need for
 * refactoring / serializing data structure out.
 *
 * Compile with make.sh.
 *
 * Result:
 *  - all glm types are POD type
 *  - sizeof(T) is exactly size of elements for such type (x,y,z,w) thus we can use such type to hold
 *    vertex data then submit to OpenGL directly.
 */
#include "lgl/lgl.h"
#include <iostream>
#include <type_traits>

template <typename T>
void isPod(const char* msg)
{
    std::cout << msg << ": " << std::boolalpha << std::is_pod<T>::value << " [" << sizeof(T) << "]" << std::endl;
}

int main()
{
    // directly refer to actual type prototype of vec
    //isPod<glm::vec<3, float, glm::defaultp>>("vec");

    isPod<glm::vec1>("vec1");
    isPod<glm::vec2>("vec2");
    isPod<glm::vec3>("vec3");
    isPod<glm::vec4>("vec4");
    isPod<glm::mat2x2>("mat2x2");
    isPod<glm::mat2x3>("mat2x3");
    isPod<glm::mat2x4>("mat2x4");
    isPod<glm::mat3x2>("mat3x2");
    isPod<glm::mat3x3>("mat3x3");
    isPod<glm::mat3x4>("mat3x4");
    isPod<glm::mat4x2>("mat4x2");
    isPod<glm::mat4x3>("mat4x3");
    isPod<glm::mat4x4>("mat4x4");
    isPod<glm::quat>("quat");
    isPod<glm::fquat>("fquat");
    isPod<glm::f32quat>("f32quat");
    isPod<glm::dquat>("dquat");
    isPod<glm::f64quat>("f64quat");
    isPod<glm::dualquat>("dualquat");

    return 0;    
}
