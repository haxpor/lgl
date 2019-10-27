/**
 * Prove of concept for Euler Angles to produce the same rotational effect if done manually
 * along each axis.
 *
 * Euler angles deep down is rotational matrix, thus its column (right-handed) elements are the same
 * and its result can be used to extract to get right, up, and directional vector for computing
 * view matrix further.
 *
 * The sequence order of euler angles' rotational matrix multiplication we chose here is Rz*Ry*Rx.
 * Anyway this can be in total of 6 possibilities.
 */
#include "lgl/lgl.h"
#include "glm/gtc/epsilon.hpp"
#include <iostream>
#include <cmath>
#include <limits>

#define X_ROT glm::radians(45.0f)
#define Y_ROT glm::radians(45.0f)
#define Z_ROT glm::radians(45.0f)

int main()
{
    glm::mat4 m1(1.0f), m2(1.0f);

    // normally form rotational matrix in chain
    m1 = glm::rotate(m1, Z_ROT, glm::vec3(0.0f, 0.0f, 1.0f));
    m1 = glm::rotate(m1, Y_ROT, glm::vec3(0.0f, 1.0f, 0.0f));
    m1 = glm::rotate(m1, X_ROT, glm::vec3(1.0f, 0.0f, 0.0f));
    std::cout << glm::to_string(m1) << '\n';

    // form via euler angles
    float sinA = std::sin(X_ROT), sinB = std::sin(Y_ROT), sinC = std::sin(Z_ROT);
    float cosA = std::cos(X_ROT), cosB = std::cos(Y_ROT), cosC = std::cos(Z_ROT);

    m2[0][0] = cosB * cosC;
    m2[0][1] = sinC * cosB;
    m2[0][2] = -sinB;
    m2[0][3] = 0.0f;

    m2[1][0] = -cosA*sinC + sinA*sinB*cosC;
    m2[1][1] = cosA*cosC + sinA*sinB*sinC;
    m2[1][2] = sinA*cosB;
    m2[1][3] = 0.0f;

    m2[2][0] = sinA*sinC + cosA*cosC*sinB;
    m2[2][1] = -sinA*cosC + sinC*sinB*cosA;
    m2[2][2] = cosB*cosA;
    m2[2][3] = 0.0f;

    m2[3][0] = 0.0f;
    m2[3][1] = 0.0f;
    m2[3][2] = 0.0f;
    m2[3][3] = 1.0f;
    std::cout << glm::to_string(m2) << '\n';

    // assert
    const float ep = std::numeric_limits<float>::epsilon();
    assert(glm::epsilonEqual(m1[0][0], m2[0][0], ep) == true);
    assert(glm::epsilonEqual(m1[0][1], m2[0][1], ep) == true);
    assert(glm::epsilonEqual(m1[0][2], m2[0][2], ep) == true);
    assert(glm::epsilonEqual(m1[0][3], m2[0][3], ep) == true);
    assert(glm::epsilonEqual(m1[1][0], m2[1][0], ep) == true);
    assert(glm::epsilonEqual(m1[1][1], m2[1][1], ep) == true);
    assert(glm::epsilonEqual(m1[1][2], m2[1][2], ep) == true);
    assert(glm::epsilonEqual(m1[1][3], m2[1][3], ep) == true);
    assert(glm::epsilonEqual(m1[2][0], m2[2][0], ep) == true);
    assert(glm::epsilonEqual(m1[2][1], m2[2][1], ep) == true);
    assert(glm::epsilonEqual(m1[2][2], m2[2][2], ep) == true);
    assert(glm::epsilonEqual(m1[2][3], m2[2][3], ep) == true);
    assert(glm::epsilonEqual(m1[3][0], m2[3][0], ep) == true);
    assert(glm::epsilonEqual(m1[3][1], m2[3][1], ep) == true);
    assert(glm::epsilonEqual(m1[3][2], m2[3][2], ep) == true);
    assert(glm::epsilonEqual(m1[3][3], m2[3][3], ep) == true);
    std::cout << "Passed - same" << std::endl;
   
    return 0;
}
