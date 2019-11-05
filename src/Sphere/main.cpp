/**
 * Sphere class to manage defines sphere's vertices and indices specification included
 * rendering function for flexibility in use in testbed.
 *
 * See Sphere.h/Sphere.cpp files for more detail on implementation.
 *
 */
#include "lgl/lgl.h"
#include "Sphere.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>

int screenWidth = 800;
int screenHeight = 600;
GLFWwindow* window = nullptr;
double prevTicks;
double lastMouseX, lastMouseY;

////////////////////////
/// system callbacks
////////////////////////
void sys_mouseCB(GLFWwindow* window, double x, double y);

////////////////////////
/// glfw callback
////////////////////////
int initGLFW(int argc, char** argv);
void initGL();

void initMem();
void destroyMem();
void reshapeCB(GLFWwindow* window, int w, int h);
void keyboardCB(double deltaTime);
void mouseCB(GLFWwindow* window, double dx, double dy);
void mouseScrollCB(GLFWwindow* window, double dx, double dy);
void update(double dt);
void render();

////////////////////////
/// global variables
////////////////////////
Sphere sphere(30, 30, 1.0f);

////////////////////////
// implementations
////////////////////////
int initGLFW(int argc, char** argv)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(screenWidth, screenHeight, argv[0], nullptr, nullptr);
    if (window == nullptr)
    {
        lgl::error::ErrorWarn("Failed to create GLFW window");
        std::exit(1);
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, reshapeCB);
    glfwSetCursorPosCallback(window, sys_mouseCB);
    glfwSetScrollCallback(window, mouseScrollCB);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        lgl::error::ErrorWarn("Failed to initalize GLAD");
        std::exit(1);
        return -1;
    }

    glViewport(0, 0, screenWidth, screenHeight);

    return 0;
}

void initGL()
{
    GLint majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    std::cout << "OpenGL version in use: " << majorVersion << "." << minorVersion << std::endl;

    glEnable(GL_DEPTH_TEST);
    
    // build sphere's vertices and indices specs
    sphere.build();

    // with no other vertex attribute data, just position, indices has more size than vertices size
    std::cout << "Total size of sphere's vertices = " << (sizeof(sphere.getVertices().data()) * sphere.getVertices().size()) << " bytes" << std::endl;
    std::cout << "Total size of sphere's indices = " << (sizeof(sphere.getIndices().data()) * sphere.getIndices().size()) << " bytes" << std::endl;

    // set uniform values
    sphere.shader.Use();

    // compute projection * view matrix
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 pv = glm::perspective(glm::radians(60.0f), screenWidth * 1.0f / screenHeight, 0.1f, 100.0f);
    pv = pv * view;
    glUniformMatrix4fv(sphere.shader.GetUniformLocation("pv"), 1, GL_FALSE, glm::value_ptr(pv));

    glUniformMatrix4fv(sphere.shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
    glUniform3f(sphere.shader.GetUniformLocation("color"), 1.0f, 1.0f, 1.0f);

    // render in wireframe to see how vertices of sphere connected and inspect correctness
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    std::cout << glfwGetVersionString() << std::endl;;
}

void update(double dt)
{

}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    sphere.shader.Use();

    // each sphere object can be configured its model matrix through statically shared shader variable)
    const float angle = 50.0f;
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, static_cast<float>(glfwGetTime()) * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    glUniformMatrix4fv(sphere.shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));

    // or directly use sphere.draw()
    sphere.drawBatchBegin();
        sphere.drawBatchDraw();
    sphere.drawBatchEnd();
}

void reshapeCB(GLFWwindow* window, int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    
    glViewport(0, 0, static_cast<GLsizei>(screenWidth), static_cast<GLsizei>(screenHeight));
}

void sys_mouseCB(GLFWwindow* window, double x, double y)
{
    static bool firstMouse = true;
    if (firstMouse)
    {
        lastMouseX = x;
        lastMouseY = y;
        firstMouse = false;
    }

    float xoffset = x - lastMouseX;
    float yoffset = lastMouseY - y;
    lastMouseX = x;
    lastMouseY = y;

    static float kSensitivity = 0.05f;
    xoffset *= kSensitivity;
    yoffset *= kSensitivity;

    mouseCB(window, xoffset, yoffset);
}

void mouseCB(GLFWwindow* window, double x, double y)
{
}

void mouseScrollCB(GLFWwindow* window, double dx, double dy)
{
}

void keyboardCB(double deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
        return;
    }
}

void initMem()
{
}

void destroyMem()
{
    // this is a tricky issue to properly manage
    // to destroy OpenGL objects properly, we need to do so in time of OpenGL context is still around.
    // thus Sphere class provides this function to destroy related OpenGL objects in time of our control.
    // Other than that for heap variables, or container variables to clean itself, it will be automatically
    // done when out of scope normally.
    //
    // You can move all code inside Sphere::destroyGLObjects() into its destructor method to see
    // the segfault crash with its not so clear of backtrace of call-stack.
    sphere.destroyGLObjects();
}

int main(int argc, char** argv)
{
    initMem();
    initGLFW(argc, argv);
    initGL();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        double delta = glfwGetTime() - prevTicks;
        prevTicks = glfwGetTime();

        keyboardCB(delta);
        update(delta);
        render();

        glfwSwapBuffers(window);
    }

    destroyMem();
    glfwTerminate();

    return 0;
}
