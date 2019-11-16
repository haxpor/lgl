/**
 * Inspect how the resultant class data components will be when create a higher dimension from
 * input lower dimension classes in GLM.
 */
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"      // for glm::to_string

int main()
{
    // this will produce all 0.0f components
    glm::mat3 mt3 = glm::mat3();
    std::cout << glm::to_string(mt3) << std::endl;

    // this will produce identity matrix
    glm::mat3 mt3Identity = glm::mat3(1.0f);
    std::cout << glm::to_string(mt3Identity) << std::endl;

    // create higher dimension from lower one
    // the last column will be [0,0,0,1] even though input is all zeros
    {
    glm::mat4 mt4 = glm::mat4(mt3);
    std::cout << glm::to_string(mt4) << std::endl;
    }

    // create higher dimension from lower one
    // the last column will be [0,0,0,1]
    {
    glm::mat4 mt4 = glm::mat4(mt3Identity);
    std::cout << glm::to_string(mt4) << std::endl;
    }

    // create higher dimension from lower via 4 column vectors
    // note: vector has no function provided to automatically create higher dimension from lower one
    {
    glm::mat4 mt4 = glm::mat4(glm::vec4(mt3Identity[0], 2.0f), glm::vec4(mt3Identity[1], 2.0f), glm::vec4(mt3Identity[2], 2.0f),
            glm::vec4(1.0f, 2.0f, 3.0f, 4.0f));
    std::cout << glm::to_string(mt4) << std::endl;
    }

    return 0;
}
