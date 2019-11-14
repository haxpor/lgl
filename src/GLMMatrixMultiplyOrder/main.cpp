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

void assertMatrixEqual(const glm::mat4& a, const glm::mat4& b)
{
    assert(a[0][0] == b[0][0] && "Two matrices must be equal");
    assert(a[0][1] == b[0][1] && "Two matrices must be equal");
    assert(a[0][2] == b[0][2] && "Two matrices must be equal");
    assert(a[0][3] == b[0][3] && "Two matrices must be equal");
    
    assert(a[1][0] == b[1][0] && "Two matrices must be equal");
    assert(a[1][1] == b[1][1] && "Two matrices must be equal");
    assert(a[1][2] == b[1][2] && "Two matrices must be equal");
    assert(a[1][3] == b[1][3] && "Two matrices must be equal");

    assert(a[2][0] == b[2][0] && "Two matrices must be equal");
    assert(a[2][1] == b[2][1] && "Two matrices must be equal");
    assert(a[2][2] == b[2][2] && "Two matrices must be equal");
    assert(a[2][3] == b[2][3] && "Two matrices must be equal");

    assert(a[3][0] == b[3][0] && "Two matrices must be equal");
    assert(a[3][1] == b[3][1] && "Two matrices must be equal");
    assert(a[3][2] == b[3][2] && "Two matrices must be equal");
    assert(a[3][3] == b[3][3] && "Two matrices must be equal");
}

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
        std::cout << "Passed 1\n";
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
        std::cout << "Passed 2\n";
    }

    // another example
    {
        // build the matrix from sequence of operations to be based as comparison
        glm::mat4 m1 = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f,  3.0f,  4.0f)); 
        m1 = glm::rotate(m1, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m1 = glm::scale(m1, glm::vec3(2.0f, 2.0f, 2.0f));

        // equally to this, each individual tranformation matrix will operate based on top of identity matrix
        glm::mat4 iden = glm::mat4(1.0f);
        glm::mat4 m2 = glm::translate(iden, glm::vec3(2.0f,  3.0f,  4.0f)) *
                       glm::rotate(iden, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
                       glm::scale(iden, glm::vec3(2.0f, 2.0f, 2.0f));

        assertMatrixEqual(m1, m2);
        std::cout << "Passed 3\n";
    }

    // multiple way to perform matrix multiplications in glm
    {
        // `m1` as input to glm::translate or glm::rotate or glm::scale will be left-multiply to the
        // resultant transformation matrix as created via such function so we can accumulately perform
        // each matrix transformation on top of each other
        glm::mat4 m1 = glm::mat4(1.0f);
        m1 = glm::translate(m1, glm::vec3(1.0f, 2.0f, 3.0f));
        m1 = glm::rotate(m1, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m1 = glm::scale(m1, glm::vec3(2.0f, 2.0f, 2.0f));

        glm::mat4 m2 = glm::mat4(1.0f) *
            glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 3.0f)) *
            glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));

        assertMatrixEqual(m1, m2);
        std::cout << "Passed 4\n";

        // we can manually do left-multiply in this way putting `m3` in front of the new transformation
        // to be created via glm's functions
        glm::mat4 m3 = glm::mat4(1.0f);
        m3 = m3 * glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 3.0f));
        m3 = m3 * glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m3 = m3 * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));

        assertMatrixEqual(m1, m3);
        std::cout << "Passed 5\n";

        // use operator *= which is the save as manually left-multiply above
        glm::mat4 m4 = glm::mat4(1.0f);
        m4 *= glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 3.0f));
        m4 *= glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        m4 *= glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));

        assertMatrixEqual(m1, m4);
        std::cout << "Passed 6\n";
    }

    std::cout.flush();

    return 0;
}
