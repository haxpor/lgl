/**
 * Prove of concept that GLM is pre-multiplication matrix order.
 *
 * GLM matrix multiplication is pre-multiplication, thus the order of transformation call can be
 * thought as beginning from left to right e.g. translate rotate | deep down matrix multiplication
 * will be rotate * translate thus rotate happens first before translation.
 */

#include <iostream>
#include "lgl/lgl.h"

int main()
{
    // |  1  2  3  4 |
    // |  5  6  7  8 |
    // |  9 10 11 12 |
    // | 13 14 15 16 |
    // 
    // for reference in comment below

    // rotate then translate
    // this produces the rotation around center of itself, thus its position is not effected by rotation
    // notice 4, 8, 12 which are the same as what we've created translation matrix {2.0f, 3.0f, 4.0f}
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, glm::vec3(2.0f,  3.0f,  4.0f)); 
        m = glm::rotate(m, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        std::cout << glm::to_string(m) << std::endl;
    }

    // translate then rotate
    // this produces orbitting effect, thus affect the position of the object
    // notice translation elements at 4, 8, 12 which will be effected by rotation.
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::rotate(m, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m = glm::translate(m, glm::vec3(2.0f,  3.0f,  4.0f)); 
        std::cout << glm::to_string(m) << std::endl;
    }

    return 0;
}
