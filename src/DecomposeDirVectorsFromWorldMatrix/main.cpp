/**
 * UpVectorFromMatrix
 *
 * This example tries to prove that 2nd column vector from transformation matrix in world space
 * represents up vector of the object in the scene even after perform a certain rotation (or in fact
 * any combination of transformation on it).
 *
 * Use a plane as a target object in the scene.
 * A yellow line drew representing up vector for such object at any time.
 * A red line drew presenting left vector for such object at any time (extra to add).
 *
 * This example code is based on 2PlanesIntersection with modifed and works up on it.
 *
 */
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "lgl/lgl.h"
#include "Sphere.h"
#include "Gizmo.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <cstring>

// we need vec3 rotation
#include "glm/gtx/vector_angle.hpp"

constexpr const float kEpsilon = std::numeric_limits<float>::epsilon();

struct Line
{
    glm::vec3 dir;
    glm::vec3 pos;

    Line(): Line(glm::vec3(0.0f), glm::vec3(0.0f)) {}
    Line(glm::vec3 e0, glm::vec3 e1):
        dir(e1 - e0),
        pos(e0)
    {}
};

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

/// render plane in geometry approach
/// geometry approach in this case involves pre-define plane's vertice with orientation into certain
/// direction (for us, it's into +z-axis). Then calculate two angles (yaw, and pitch) with no need for
/// roll angle to be computed to create a transform matrix to update to OpenGL's uniform.
///
/// Plane's normal needs to be normalized.
/// color is normalized color value between 0.0f and 1.0f (inclusive).
/// normColor is the same.
void renderPlane_geometry(const Plane& p, const glm::vec3& color, const glm::vec3& normColor);

/// compute lookAt matrix to orient object to look into `target` position
glm::mat4 computeLookAtForObject(const glm::vec3& pos, const glm::vec3& target);

/// compute plane's 4 corners then return it via `outCorners`
/// `outCorners` will return 4 positions of plane's corners starting from the top-right, top-left,
/// bottom-left, then bottom-right.
void computePlaneCorners(const Plane& p, glm::vec3 outCorners[4]);

////////////////////////
/// global variables
////////////////////////
glm::mat4 view, projection;
GLuint sharedVAO;       // shared vao for both drawing lines and planes
GLuint sharedVBO;       // shared vbo for both drawing lines and planes
lgl::Shader shader;
Sphere dot(20, 20, 0.03f);
Sphere planeDot(10,10, 0.007f);
Gizmo gizmo;

#define PLANE_SIZE_FACTOR 0.4f
Plane plane1(glm::vec3(0.0f, 0.1f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

// to hold modified planes' normal and not immediate affect ImGui's UI
glm::vec3 plane1NotNeccessaryNormalized = plane1.normal;
// to hold planes' corners for drawing 4 dots
glm::vec3 plane1CornersVertices[4];

glm::vec3 xAxis[2] = {
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
};
glm::vec3 yAxis[6] = {
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
    int result = shader.Build("data2/trans.vert", "data2/color.frag");
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
        glBindBuffer(GL_ARRAY_BUFFER, sharedVAO);
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
    glUniformMatrix4fv(dot.shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(dot.shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    lgl::error::AnyGLError();
    glUniformMatrix4fv(dot.shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
    lgl::error::AnyGLError();

    // build up vertex buffers of planeDot (Sphere, smaller with less LOD)
    planeDot.build();
    planeDot.shader.Use();
    glUniformMatrix4fv(planeDot.shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(planeDot.shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
    lgl::error::AnyGLError();
    glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
    lgl::error::AnyGLError();
    
    // build up gizmo
    gizmo.build();
    gizmo.updateViewMatrix(view);

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

/// compute transformation matrix to orient object to look at the input `target` based on the current
/// position of `pos`.
glm::mat4 computeLookAtForObject(const glm::vec3& pos, const glm::vec3& target)
{
    glm::vec3 forward = glm::normalize(target - pos);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // handle if forward is nearly the same as up vector
    // then we choose another direction for forward
    if (std::abs(forward.x) < kEpsilon && std::abs(forward.z) < kEpsilon)
    {
        if (forward.y > 0.0f)
        {
            up.x = 0.0f;
            up.y = 0.0f;
            up.z = -1.0f;
        }
        else
        {
            up.x = 0.0f;
            up.y = 0.0f;
            up.z = 1.0f;
        }
    }

    glm::vec3 left = glm::normalize(glm::cross(up, forward));
    up = glm::cross(forward, left);

    glm::mat4 m = glm::mat4(1.0f);
    m[0] = glm::vec4(left, 0.0f);
    m[1] = glm::vec4(up, 0.0f);
    m[2] = glm::vec4(forward, 0.0f);
    return m;
}

void computePlaneCorners(const Plane& p, glm::vec3 outCorners[4])
{
    // compute rotational matrix for plane to look into its own normal vector
    // what we need is not the actual matrix, but its 3 column elements inside
    // each represents left, up, and forward vector that will help us finally solve
    // finding corner positions
    glm::mat4 m = computeLookAtForObject(p.pos, p.pos+p.normal);

    // convert from vec4 to vec3
    glm::vec3 left = glm::vec3(m[0]);
    glm::vec3 up = glm::vec3(m[1]);

    // size of resultant vector in diagonal direction
    const float kSize = std::sqrt(PLANE_SIZE_FACTOR * PLANE_SIZE_FACTOR + PLANE_SIZE_FACTOR*PLANE_SIZE_FACTOR);
    // top-right
    outCorners[0] = p.pos + glm::normalize(up+left)*kSize;
    // top-left
    outCorners[1] = p.pos + glm::normalize(up-left)*kSize;
    // bottom-left
    outCorners[2] = p.pos + glm::normalize(-up-left)*kSize;
    // bottom-right
    outCorners[3] = p.pos + glm::normalize(-up+left)*kSize;
}

// required: 'shader' is active
void renderPlane_geometry(const Plane& p, const glm::vec3& color, const glm::vec3& normColor)
{
    // instead of manually rotate the matrix in sequence to orient the object to look into plane's normal
    // we compute lookAt matrix (similarly to camera's lookAt but direction vector is pointing into target position,
    // and ignore positional inforation, and rotational matrix is not transpose).
    glm::mat4 lookAt = computeLookAtForObject(p.pos, p.pos + p.normal);     // <--- this is the look at matrix we want to prove here
    glm::mat4 model = glm::translate(glm::mat4(1.0f), p.pos) * lookAt;
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
    planeNormalLineVertices[0] = p.pos;
    planeNormalLineVertices[1] = p.pos + 0.5f*p.normal;
    glUniform3f(shader.GetUniformLocation("color"), normColor.x, normColor.y, normColor.z);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, planeNormalLineVertices, GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, 2);

    // 3. render plane's up vector
    // get up vector from 2nd column vector of lookAt matrix as we constructed it from above
    // note: use `model` matrix here to validate that using resultant matrix from multiple operations works
    glm::vec3 lineDir = model[1];
    // use plane's position as the beginning point then extend into lineDir direction 
    planeUpLineVertices[0] = p.pos + lineDir*PLANE_SIZE_FACTOR*1.3f;
    planeUpLineVertices[1] = p.pos;
    glUniform3f(shader.GetUniformLocation("color"), 1.0f, 1.0f, 0.0f);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, planeUpLineVertices, GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, 2);

    // 4. (extra) render plane's left vector
    // get left vector from 1st column vector of lookAt matrix as we constructed it from above
    lineDir = model[0];
    // use plane's position as the beginning point then extend into lineDir direction
    planeLeftLineVertices[0] = p.pos + lineDir*PLANE_SIZE_FACTOR*1.3f;
    planeLeftLineVertices[1] = p.pos;
    glUniform3f(shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, planeLeftLineVertices, GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, 2);

    glDepthFunc(GL_LESS);
}

// required: dotShader needs to already began
void drawDotAtPlaneCorners()
{
    // top-right plane-corner
    dotVertex = plane1CornersVertices[0];
    glUniform3f(planeDot.shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
    glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
            glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
    planeDot.drawBatchDraw();

    // top-left plane-corner
    dotVertex = plane1CornersVertices[1];
    glUniform3f(planeDot.shader.GetUniformLocation("color"), 0.0f, 1.0f, 0.0f);
    glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
            glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
    planeDot.drawBatchDraw();

    // bottom-left plane-corner
    dotVertex = plane1CornersVertices[2];
    glUniform3f(planeDot.shader.GetUniformLocation("color"), 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
            glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
    planeDot.drawBatchDraw();

    // bottom-right plane-corner
    dotVertex = plane1CornersVertices[3];
    glUniform3f(planeDot.shader.GetUniformLocation("color"), 0.0f, 1.0f, 1.0f);
    glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
            glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
    planeDot.drawBatchDraw();
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
        // plane
        renderPlane_geometry(plane1, glm::vec3(0.0f, 0.6f, 0.7f), glm::vec3(0.0f, 0.8f, 1.0f));

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

        // compute plane's corners for both use in
        // 1. (within) plane intersection
        // 2. debugging draw for spheres on all plane's corners
        computePlaneCorners(plane1, plane1CornersVertices);

        planeDot.shader.Use();
        planeDot.drawBatchBegin();
            drawDotAtPlaneCorners();
        planeDot.drawBatchEnd();
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
#define IMGUI_WINDOW_HEIGHT 170
#define IMGUI_WINDOW_MARGIN 5
    ImGui::SetNextWindowSize(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT));
    ImGui::SetNextWindowSizeConstraints(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT), ImVec2(IMGUI_WINDOW_WIDTH,IMGUI_WINDOW_HEIGHT));
    //ImGui::SetNextWindowPos(ImVec2(screenWidth - IMGUI_WINDOW_WIDTH - IMGUI_WINDOW_MARGIN, IMGUI_WINDOW_MARGIN));
    ImGui::Begin("Up Vector from Matrix");
        //static bool mm = true;
        //ImGui::ShowDemoWindow(&mm);

        // plane 1 normal
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Plane 1");
        if (ImGui::SliderFloat("Norm x##1", &plane1NotNeccessaryNormalized.x, -1.0f, 1.0f, "%.2f"))
            plane1.normal = glm::normalize(plane1NotNeccessaryNormalized);
        if (ImGui::SliderFloat("Norm y##2", &plane1NotNeccessaryNormalized.y, -1.0f, 1.0f, "%.2f"))
            plane1.normal = glm::normalize(plane1NotNeccessaryNormalized);
        if (ImGui::SliderFloat("Norm z##3", &plane1NotNeccessaryNormalized.z, -1.0f, 1.0f, "%.2f"))
            plane1.normal = glm::normalize(plane1NotNeccessaryNormalized);

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

        planeDot.shader.Use();
        glUniformMatrix4fv(planeDot.shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

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

        planeDot.shader.Use();
        glUniformMatrix4fv(shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
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
    planeDot.destroyGLObjects();
}

bool twoPlaneIntersect(const Plane& p1, const Plane& p2, Line& intersectedLine)
{
    // calculate vector as normal from both plane p1, and p2 normal
    // v = n1 X n2
    glm::vec3 v = glm::normalize(glm::cross(p1.normal, p2.normal));

    // define raw matrix elements in column-wise order to satisfy glm
    float rawMatrix[9] = {
        p1.normal.x, p2.normal.x, v.x,
        p1.normal.y, p2.normal.y, v.y,
        p1.normal.z, p2.normal.z, v.z
    };
    float detA = glm::determinant(glm::make_mat3(rawMatrix));

    if (std::abs(detA) < kEpsilon)
        return false;
    
    glm::vec3 a = p1.normal;
    glm::vec3 b = p2.normal;
    glm::vec3 c = v;

    // note: use mat's constructor to directly define matrix from all individual elements
    // defined element order is in row-based specifically for this case!
    // note: choose 0.0f as D constant for v vector
    intersectedLine.pos = glm::mat3(b.y*c.z - b.z*c.y, b.z*c.x - b.x*c.z, b.x*c.y - b.y*c.x,
                               a.z*c.y - a.y*c.z, a.x*c.z - a.z*c.x, a.y*c.x - a.x*c.y,
                               a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x)
                               * glm::vec3(-p1.getD(), -p2.getD(), 0.0f) / detA;
    intersectedLine.dir = v;

    return true;
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
