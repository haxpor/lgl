/**
 * Prove of concept that GLM is pre-multiplication matrix order.
 *
 * GLM matrix multiplication is pre-multiplication, thus the order of transformation call can be
 * thought as beginning from left to right e.g.
 *  m = translate operation
 *  m = rotate operation
 *
 * deep down GLM will do translate_matrix * rotation_matrix. So the last operation will be executed first.
 */

#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"

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

        assert(m[3][0] == 2.0f &&
               m[3][1] == 3.0f &&
               m[3][2] == 4.0f && "Translation components must not change");
    }

    // translate then rotate
    // this produces orbitting effect, thus affect the position of the object
    // notice translation elements at 4, 8, 12 which will be affected by rotation.
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::rotate(m, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m = glm::translate(m, glm::vec3(2.0f,  3.0f,  4.0f)); 

        assert(m[3][1] != 3.0f &&
               m[3][2] != 4.0f && "Translation components must changed except x component");
    }

    // another example
    {
        // build the matrix from sequence of operations to be based as comparison
        glm::mat4 m1 = glm::mat4(1.0f);
        m1 = glm::translate(m1, glm::vec3(2.0f,  3.0f,  4.0f)); 
        m1 = glm::rotate(m1, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m1 = glm::scale(m1, glm::vec3(1.0f, 1.0f, 1.0f));

        // equally to this, each individual tranformation matrix will operate based on top of identity matrix
        glm::mat4 iden = glm::mat4(1.0f);
        glm::mat4 m2 = glm::translate(iden, glm::vec3(2.0f,  3.0f,  4.0f)) * glm::rotate(iden, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(iden, glm::vec3(1.0f, 1.0f, 1.0f));

        assert(m1[0][0] == m2[0][0] && "Two matrices must be equal");
    }

    return 0;
}
