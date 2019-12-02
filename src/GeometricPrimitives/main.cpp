/**
 * GeometricPrimitives
 *
 * Program to draw multiple geometric primitives with GUI to select which one to draw.
 * This includes
 *  - Line
 *  - Plane
 *  - Box
 *  - Sphere
 *  - Cylinder
 *  - Cone
 *
 */
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "lgl/lgl.h"
#include "Sphere.h"
#include "Gizmo.h"
#include "Line.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <cstring>

// we need vec3 rotation
#include "glm/gtx/vector_angle.hpp"

constexpr const float kEpsilon = std::numeric_limits<float>::epsilon();

struct Plane
{
    glm::vec3 pos;
    glm::vec3 normal;

    /// create a plane from known point on the plane, and normal vector
    /// normal will be automatically normalized
    Plane(glm::vec3 p, glm::vec3 n):
        pos(p),
        normal(glm::normalize(n))
    { }

    /// d is - (Ax * x1 + Ay * y1 + Az * z1) from following equation
    /// Ax * x + Ay * y + Az * z - (Ax * x1 + Ay * y1 + Az * z1) = 0
    float getD() const
    {
        return -(normal.x*pos.x + normal.y*pos.y + normal.z*pos.z);
    }
};

enum PrimitiveType
{
    LINE,
    PLANE,
    BOX,
    SPHERE,
    CYLINDER,
    CONE
};

int screenWidth = 800;
int screenHeight = 600;
GLFWwindow* window = nullptr;
double prevTicks;
double lastMouseX, lastMouseY;
PrimitiveType ptype = PrimitiveType::LINE;

////////////////////////
/// system callbacks
////////////////////////
void sys_mouseCB(GLFWwindow* window, double x, double y);

////////////////////////
/// glfw callback
////////////////////////
int initGLFW(int argc, char** argv);
void initGL();
void initImGUI();
void initImGUI();

void initMem();
void destroyMem();
void reshapeCB(GLFWwindow* window, int w, int h);
void keyboardCB(double deltaTime);
void mouseCB(GLFWwindow* window, double dx, double dy);
void mouseScrollCB(GLFWwindow* window, double dx, double dy);
void mouseButtonCB(GLFWwindow* window, int button, int action, int mods);
void update(double dt);
void render();
void renderGizmo();
void renderGUI();
void renderPlane_geometry(const Plane& p, const glm::mat3& orientation, const glm::vec3& color, const glm::vec3& normColor);
glm::mat3 rotateEulerAnglesXYZ(float angleX, float angleY, float angleZ);
void updateSelectedPrimitiveViewMatrix(const glm::mat4& v);
void updateSelectedPrimitiveProjectionMatrix(const glm::mat4& p);

////////////////////////
/// global variables
////////////////////////
glm::mat4 view, projection;
GLuint sharedVAO;       // shared vao for both drawing lines and planes for misc stuff
GLuint sharedVBO;       // shared vbo for both drawing lines and planes for misc stuff
lgl::Shader shader;
Sphere dot(20, 20, 0.03f);
Gizmo gizmo;

/// list of primitives we will draw
Line primitive_line(glm::vec3(-0.5f, -0.2f, -0.2f), glm::vec3(0.5f, 0.2f, 0.3f));
Sphere primitive_sphere(20, 20, 0.5f);

#define PLANE_SIZE_FACTOR 0.4f
Plane plane(glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

// quaternion maintained for blue-plane
#define SLERP_T 0.10
glm::quat planeQuat = glm::identity<glm::quat>();

glm::vec3 targetFacingDir = glm::vec3(0.0f, 0.0f, 1.0f);
glm::quat targetFacingQuat = glm::identity<glm::quat>();        // to be dynamically computed from configuring via GUI

glm::vec3 xAxis[2] = {
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
};
glm::vec3 yAxis[2] = {
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f)
};

// plane vertices for geometry way of calculation
// order defined here is based on GL_TRIANGLE_STRIP
// in which the previous two vertices after the first triangle defined will be used in the next
// triangle definition.
glm::vec3 planeVertices[4] = {
    glm::vec3(1.0f*PLANE_SIZE_FACTOR, 1.0f*PLANE_SIZE_FACTOR, 0.0f),
    glm::vec3(-1.0f*PLANE_SIZE_FACTOR, 1.0f*PLANE_SIZE_FACTOR, 0.0f),
    glm::vec3(1.0f*PLANE_SIZE_FACTOR, -1.0f*PLANE_SIZE_FACTOR, 0.0f),
    glm::vec3(-1.0f*PLANE_SIZE_FACTOR, -1.0f*PLANE_SIZE_FACTOR, 0.0f)
};
// shared variables used to draw plane and its accessories
glm::vec3 planeNormalLineVertices[2];
glm::vec3 planeUpLineVertices[2];
glm::vec3 planeLeftLineVertices[2];

glm::vec3 dotVertex;

glm::vec3 camPos; 
const glm::vec3 kInitialCamPos = glm::vec3(0.0f, 0.0f, 2.0f);
const glm::vec3 kCamLookAtPos = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 kCamUp = glm::vec3(0.0f, 1.0f, 0.0f);
float camYaw = 0.0f, camPitch = 0.0f;   // compared to CameraImgui, camYaw here is initially set to 0.0f
                                        // as now we build up rotation accumulately from the initial reference point
float camFov = 45.0f;
bool isLeftMousePressed = false;
bool wireframeMode = false;

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
    glfwSetMouseButtonCallback(window, mouseButtonCB);

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
    
    // create shader
    const char* vertexShaderStr = R"(#version 330 core
#extension GL_ARB_explicit_uniform_location : require
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
})";
    const char* fragmentShaderStr = R"(#version 330 core
uniform vec3 color;
out vec4 fsColor;
void main()
{
    fsColor = vec4(color, 1.0); 
}
)";
    int result = shader.BuildFromSrc(vertexShaderStr, fragmentShaderStr);
    LGL_ERROR_QUIT(result, "Error creating shader");

    glEnable(GL_DEPTH_TEST);

    // create buffer objects
    glGenVertexArrays(1, &sharedVAO);
    glGenBuffers(1, &sharedVBO);

    // prepare for line vao
    glBindVertexArray(sharedVAO);
        // a single VBO shares both vertices from both p and q lines to render on screen
        // although having dedicated VBO for each line should be higher performance, but this is also
        // another testbed setup for this program
        //
        // so we supply 'data` parameter as nullptr and we will do buffer update in render loop
        glBindBuffer(GL_ARRAY_BUFFER, sharedVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
        glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    shader.Use();
    view = glm::lookAt(kInitialCamPos, kCamLookAtPos, kCamUp);
    projection = glm::perspective(glm::radians(camFov), screenWidth*1.0f/screenHeight, 0.1f, 100.0f);
    glUniformMatrix4fv(shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    lgl::error::AnyGLError();

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
    lgl::error::AnyGLError();

    // build up vertex buffers of dot (Sphere)
    dot.build();
    // setup Sphere (dot)'s shader
    dot.shader.Use();
    dot.updateProjectionMatrix(projection);
    dot.updateViewMatrix(view);
    dot.updateModelMatrix(model);
    lgl::error::AnyGLError();
    
    // build up gizmo
    gizmo.build();
    gizmo.updateViewMatrix(view);

    // build all primitives
    // build line
    primitive_line.build();
    primitive_line.shader.Use();
    primitive_line.setLineColor(1.0f, 1.0f, 0.0f);
    primitive_line.updateProjectionMatrix(projection);
    primitive_line.updateViewMatrix(view);
    primitive_line.updateModelMatrix(model);
    lgl::error::AnyGLError();

    // build sphere
    primitive_sphere.build();
    primitive_sphere.shader.Use();
    primitive_sphere.setColor(1.0f, 1.0f, 0.0f);
    primitive_sphere.updateProjectionMatrix(projection);
    primitive_sphere.updateViewMatrix(view);
    primitive_sphere.updateModelMatrix(model);

    std::cout << glfwGetVersionString() << std::endl;;
    initImGUI();
}

void initImGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void update(double dt)
{
}

/// create a combined rotational matrix via euler angles method in order of XYZ
glm::mat3 rotateEulerAnglesXYZ(float angleX, float angleY, float angleZ)
{
    float c1 = std::cos(angleX); 
    float s1 = std::sin(angleX);
    float c2 = std::cos(angleY);
    float s2 = std::sin(angleY);
    float c3 = std::cos(angleZ);
    float s3 = std::sin(angleZ);

    return glm::mat3(c2*c3,         s1*s2*c3 + c1*s3,       -c1*s2*c3 + s1*s3,
                     -c2*s3,        -s1*s2*s3 + c1*c3,      c1*s2*s3 + s1*c3,
                     s2,            -s1*c2,                 c1*c2);     
}

// required: 'shader' is active
void renderPlane_geometry(const Plane& p, const glm::mat3& orientation, const glm::vec3& color, const glm::vec3& normColor)
{
    // this will automatically fill the last column vector to be [0,0,0,1]
    glm::mat4 model = glm::mat4(orientation);
    // manually set positional components
    model[3][0] = p.pos.x;
    model[3][1] = p.pos.y;
    model[3][2] = p.pos.z;
    glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(shader.GetUniformLocation("color"), color.x, color.y, color.z);

    // 1. render plane
    // invalidate entire buffer (orphan)
    // invalidate will put old storage on the free list once there is no other rendering commands
    // using it. Same goes for other lines.
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, nullptr, GL_DYNAMIC_DRAW);
    // update buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, planeVertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // (reset matrix back to normal)
    glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

    glDepthFunc(GL_ALWAYS);
    // 2. render plane normal
    glm::vec3 lineDir = glm::cross(orientation[0], orientation[1]);
    planeNormalLineVertices[0] = p.pos;
    planeNormalLineVertices[1] = p.pos + lineDir*PLANE_SIZE_FACTOR*1.3f;
    glUniform3f(shader.GetUniformLocation("color"), normColor.x, normColor.y, normColor.z);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, planeNormalLineVertices, GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, 2);

    // 3. render plane's up vector
    // get up vector from 2nd column vector of lookAt matrix as we constructed it from above
    // note: use `model` matrix here to validate that using resultant matrix from multiple operations works
    lineDir = orientation[1];
    // use plane's position as the beginning point then extend into lineDir direction 
    planeUpLineVertices[0] = p.pos + lineDir*PLANE_SIZE_FACTOR*1.3f;
    planeUpLineVertices[1] = p.pos;
    glUniform3f(shader.GetUniformLocation("color"), 1.0f, 1.0f, 0.0f);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, planeUpLineVertices, GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, 2);

    // 4. (extra) render plane's left vector
    // get left vector from 1st column vector of lookAt matrix as we constructed it from above
    lineDir = orientation[0];
    // use plane's position as the beginning point then extend into lineDir direction
    planeLeftLineVertices[0] = p.pos + lineDir*PLANE_SIZE_FACTOR*1.3f;
    planeLeftLineVertices[1] = p.pos;
    glUniform3f(shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, planeLeftLineVertices, GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, 2);

    glDepthFunc(GL_LESS);
}

void updateSelectedPrimitiveViewMatrix(const glm::mat4& v)
{
    switch (ptype)
    {
    case PrimitiveType::LINE:
        primitive_line.shader.Use();
        primitive_line.updateViewMatrix(v);
        break;
    case PrimitiveType::SPHERE:
        primitive_sphere.shader.Use();
        primitive_sphere.updateViewMatrix(v);
        break;
    }
}

void updateSelectedPrimitiveProjectionMatrix(const glm::mat4& p)
{
    switch (ptype)
    {
    case PrimitiveType::LINE:
        primitive_line.shader.Use();
        primitive_line.updateProjectionMatrix(p);
        break;
    case PrimitiveType::SPHERE:
        primitive_sphere.shader.Use();
        primitive_sphere.updateProjectionMatrix(p);
        break;
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    if (wireframeMode)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    shader.Use();

    glBindVertexArray(sharedVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sharedVBO);

        // x-axis
        glUniform3f(shader.GetUniformLocation("color"), 1.0f, 1.0f, 1.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, xAxis, GL_STREAM_DRAW);
        glDrawArrays(GL_LINES, 0, 2);

        // y-axis
        glUniform3f(shader.GetUniformLocation("color"), 1.0f, 1.0f, 1.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, yAxis, GL_STREAM_DRAW);
        glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);

    // draw the selected primitive on screen
    switch (ptype)
    {
    case PrimitiveType::LINE:
        primitive_line.shader.Use();
        primitive_line.draw();      
        break;
    case PrimitiveType::SPHERE:
        primitive_sphere.shader.Use();
        primitive_sphere.draw();
        break;
    }
}

void renderGizmo()
{
    gizmo.draw();
}

void renderGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

#define IMGUI_WINDOW_WIDTH 210
#define IMGUI_WINDOW_HEIGHT 100
#define IMGUI_WINDOW_MARGIN 5
    ImGui::SetNextWindowSize(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT));
    ImGui::SetNextWindowSizeConstraints(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT), ImVec2(IMGUI_WINDOW_WIDTH,IMGUI_WINDOW_HEIGHT));
    ImGui::Begin("Select Primitive");
        //static bool mm = true;
        //ImGui::ShowDemoWindow(&mm);

        // select which primitive to be drew on screen
        int ptypeInt = static_cast<int>(ptype);
        if (ImGui::Combo("Type", &ptypeInt, "Line\0Plane\0Box\0Sphere\0Cylinder\0Cone\0"))
        {
            // keep updating the value
            ptype = static_cast<PrimitiveType>(ptypeInt);
        }

        ImGui::Separator();

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Misc");

        // wireframe mode
        // TODO: migrate to use shader to draw wireframe instead of using fixed-function ... 
        ImGui::Checkbox("Wireframe mode", &wireframeMode); 
            
    ImGui::End();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
    if (isLeftMousePressed && !ImGui::IsAnyWindowHovered() && !ImGui::IsAnyItemHovered())
    {
        camYaw = std::fmod(camYaw + x*2.5f, 360.0f);
        camPitch += y*2.5f;

        if (camPitch > 89.0f)
            camPitch = 89.0f;
        if (camPitch < -89.0f)
            camPitch = -89.0f;

        // rotate around y-axis from initial cam-pos
        camPos = glm::rotate(kInitialCamPos, glm::radians(-camYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        // rotate around x-axis from previous result accumulately
        // note: we still can use +y-axis as up vector as the very first rotation operation based
        // on the initial cam position which is kInitialCamPos, then we build up rotation from there
        // step by step.
        glm::vec3 right = glm::cross(kCamLookAtPos - camPos, glm::vec3(0.0f, 1.0f, 0.0f));
        camPos = glm::rotate(camPos, glm::radians(camPitch), right);
        view = glm::lookAt(camPos, kCamLookAtPos, kCamUp);

        shader.Use();
        glUniformMatrix4fv(shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

        dot.shader.Use();
        glUniformMatrix4fv(dot.shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

        updateSelectedPrimitiveViewMatrix(view);

        gizmo.updateViewMatrix(view);
    }
}

void mouseScrollCB(GLFWwindow* window, double dx, double dy)
{
    if (!ImGui::IsAnyWindowHovered() && !ImGui::IsAnyItemHovered())
    {
        if (camFov >= 1.0f && camFov <= 60.0f)
            camFov -= dy;
        if (camFov < 1.0f)
            camFov = 1.0f;
        if (camFov > 60.0f)
            camFov = 60.0f;

        shader.Use();
        projection = glm::perspective(glm::radians(camFov), screenWidth * 1.0f / screenHeight, 0.1f, 100.0f);
        glUniformMatrix4fv(shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

        dot.shader.Use();
        glUniformMatrix4fv(shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

        updateSelectedPrimitiveProjectionMatrix(projection);
    }
}

void mouseButtonCB(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        isLeftMousePressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        isLeftMousePressed = false;
    }
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
    glDeleteBuffers(1, &sharedVAO);
    glDeleteBuffers(1, &sharedVBO);
    shader.Destroy();
    dot.destroyGLObjects();
    primitive_line.destroyGLObjects();
    primitive_sphere.destroyGLObjects();
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
        renderGizmo();
        renderGUI();

        glfwSwapBuffers(window);
    }

    destroyMem();
    glfwTerminate();

    return 0;
}
