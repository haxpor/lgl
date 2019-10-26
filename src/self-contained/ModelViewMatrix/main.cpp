/**
 * Inspect the result of model * view matrix.
 */

#include <iostream>
#include "lgl/lgl.h"

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
