/**
 * Inspect the result of model * view matrix.
 *
 * Note:
 *  - glm::to_string() will print each column-elements one by one thus
 *
 *  let say the following is the matrix element layout
 *  [1 5 9  13]
 *  [2 6 10 14]
 *  [3 7 11 15]
 *  [4 8 12 16]
 *
 *  so for glm::mat2 it will print
 *
 *  mat2x2((1,2,3,4), (5,6,7,8), (9,10,11,12), (13,14,15,16))
 */

#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"      // for glm::to_string

int main()
{
    // build model matrix
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, glm::vec3(2.0f,  3.0f,  4.0f)); 
    m = glm::rotate(m, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(1.0f, 1.0f, 1.0f));
    std::cout << glm::to_string(m) << std::endl;

    // build view matrix
    glm::mat4 v = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    std::cout << glm::to_string(v) << std::endl;

    // view * model
    glm::mat4 modelview = v * m;
    std::cout << glm::to_string(modelview) << std::endl;

    return 0;
}
