/**
 * Demonstration to
 *  constant * glm::vec3
 *  and
 *  glm::vec3 * constant
 *
 *  to inspect and see how glm::vec3 (imply others) implements its operator* with support for constant
 *  value multiplication.
 *
 *  At glm's file detail/type_vec3.hpp, there's bunch of definition regarding free function to support
 *  left and right multiplication with constant for glm::vec3. So to support this, it needs free
 *  function implementation.
 */
#include "lgl/lgl.h"
#include <iostream>

int main()
{
    float c = 5.0f;
    glm::vec3 v(1.0f);
    // constant * glm::vec3
    glm::vec3 r1 = c * v;
    std::cout << glm::to_string(r1) << std::endl;

    // glm::vec3 * constant
    glm::vec3 r2 = v * c;
    std::cout << glm::to_string(r2) << std::endl;

    assert(r1.x == r2.x);
    assert(r1.y == r2.y);
    assert(r1.z == r2.z);
    
    return 0;
}
