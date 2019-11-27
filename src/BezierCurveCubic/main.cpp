/**
 * BezierCurveCubic
 *
 * Based on BezierCurveQuadratic, but expanded to cubic bezier curve.
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
// we need vec3 rotation
#include "glm/gtx/vector_angle.hpp"

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
void traceCubicBezierCurve(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, glm::vec3 outCurve[], int numSegment);

////////////////////////
/// global variables
////////////////////////

// more, more smooth
#define CURVE_NUM_SEGMENT 100
glm::vec3 curve[CURVE_NUM_SEGMENT];
glm::vec3 p0 = glm::vec3(-0.5f, -0.5f, 0.5f);
glm::vec3 p1 = glm::vec3(-0.3f, 0.0f, 0.0f);
glm::vec3 p2 = glm::vec3(0.3f, 0.2f, -0.1f);
glm::vec3 p3 = glm::vec3(0.5f, 0.5f, -0.5f);

glm::mat4 view, projection;
/// sharedVBO will share buffer object for dot, normal line, and smooth curve (bezier)
/// thus during drawing-time, we update just subset of buffer object for line, but fully
/// update the whole buffer for the curve. See setting up code of sharedVBO below in which
/// we use CURVE_NUM_SEGMENT as number of object.
GLuint sharedVAO;
GLuint sharedVBO;
lgl::Shader shader;
Sphere dot(20, 20, 0.03f);
Gizmo gizmo;

glm::vec3 xAxis[2] = {
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
};
glm::vec3 yAxis[2] = {
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f)
};

glm::vec3 camPos; 
const glm::vec3 kInitialCamPos = glm::vec3(0.0f, 0.0f, 2.0f);
const glm::vec3 kCamLookAtPos = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 kCamUp = glm::vec3(0.0f, 1.0f, 0.0f);
float camYaw = 0.0f, camPitch = 0.0f;   // compared to CameraImgui, camYaw here is initially set to 0.0f
                                        // as now we build up rotation accumulately from the initial reference point
float camFov = 45.0f;
bool isLeftMousePressed = false;

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
        glBindBuffer(GL_ARRAY_BUFFER, sharedVBO);
        // we draw a line with GL_LINE_STRIP, so for N connected curve, we need N glm::vec3 to hold.
        // +2 because of to hold more static line at the end to prevent of invalidate our curve line staying on GPU side
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * (CURVE_NUM_SEGMENT + 2), nullptr, GL_STREAM_DRAW);
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

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    
    shader.Use();

    glBindVertexArray(sharedVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sharedVBO);

        // x-axis
        glUniform3f(shader.GetUniformLocation("color"), 1.0f, 1.0f, 1.0f);
        glBufferSubData(GL_ARRAY_BUFFER, (CURVE_NUM_SEGMENT-1) * sizeof(glm::vec3), sizeof(glm::vec3) * 2, xAxis);
        // the static x and y axes now live at the two elements at the back of the array
        glDrawArrays(GL_LINE_STRIP, CURVE_NUM_SEGMENT-1, 2);

        // y-axis
        glBufferSubData(GL_ARRAY_BUFFER, (CURVE_NUM_SEGMENT-1) * sizeof(glm::vec3), sizeof(glm::vec3) * 2, yAxis);
        glDrawArrays(GL_LINE_STRIP, CURVE_NUM_SEGMENT-1, 2);

        // bezier curve
        traceCubicBezierCurve(p0, p1, p2, p3, curve, CURVE_NUM_SEGMENT);
        glUniform3f(shader.GetUniformLocation("color"), 1.0f, 1.0f, 0.0f);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * CURVE_NUM_SEGMENT, curve);
        glDrawArrays(GL_LINE_STRIP, 0, CURVE_NUM_SEGMENT);

    // dot for p0, p1, and p2
    dot.shader.Use();
    dot.drawBatchBegin();
        // p0
        glUniform3f(dot.shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
        glUniformMatrix4fv(dot.shader.GetUniformLocation("model"), 1, GL_FALSE,
                glm::value_ptr(glm::translate(glm::mat4(1.0f), p0)));
        dot.drawBatchDraw();

        // p3
        glUniform3f(dot.shader.GetUniformLocation("color"), 0.0f, 0.0f, 1.0f);
        glUniformMatrix4fv(dot.shader.GetUniformLocation("model"), 1, GL_FALSE,
                glm::value_ptr(glm::translate(glm::mat4(1.0f), p3)));
        dot.drawBatchDraw();

        // p1 (control point)
        glUniform3f(dot.shader.GetUniformLocation("color"), 1.0f, 1.0f, 1.0f);
        glUniformMatrix4fv(dot.shader.GetUniformLocation("model"), 1, GL_FALSE,
                glm::value_ptr(glm::translate(glm::mat4(1.0f), p1)));
        dot.drawBatchDraw();

        // p2 (control point)
        glUniform3f(dot.shader.GetUniformLocation("color"), 0.6f, 0.6f, 0.6f);
        glUniformMatrix4fv(dot.shader.GetUniformLocation("model"), 1, GL_FALSE,
                glm::value_ptr(glm::translate(glm::mat4(1.0f), p2)));
        dot.drawBatchDraw();

    dot.drawBatchEnd();
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
#define IMGUI_WINDOW_HEIGHT 415
#define IMGUI_WINDOW_MARGIN 5
    ImGui::SetNextWindowSize(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT));
    ImGui::SetNextWindowSizeConstraints(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT), ImVec2(IMGUI_WINDOW_WIDTH,IMGUI_WINDOW_HEIGHT));
    //ImGui::SetNextWindowPos(ImVec2(screenWidth - IMGUI_WINDOW_WIDTH - IMGUI_WINDOW_MARGIN, IMGUI_WINDOW_MARGIN));
    ImGui::Begin("Configurations");
        //static bool mm = true;
        //ImGui::ShowDemoWindow(&mm);
        
        // control point (p1)
        // tip: in ImGui use ## postfixed at the name of such UI element to differentiate from other in naming ID
        // so this will allow multiple UI elements to have the same name as users will see on screen
        // p1
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "p1 (control point)");
        ImGui::SliderFloat("X##1", &p1.x, -1.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Y##1", &p1.y, -1.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Z##1", &p1.z, -1.0f, 1.0f, "%.2f");

        // p2
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "p2 (control point)");
        ImGui::SliderFloat("X##2", &p2.x, -1.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Y##2", &p2.y, -1.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Z##2", &p2.z, -1.0f, 1.0f, "%.2f");

        // p0
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "p0 (starting point)");
        ImGui::SliderFloat("X##0", &p0.x, -1.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Y##0", &p0.y, -1.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Z##0", &p0.z, -1.0f, 1.0f, "%.2f");

        // p3
        ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "p3 (ending point)");
        ImGui::SliderFloat("X##3", &p3.x, -1.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Y##3", &p3.y, -1.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Z##3", &p3.z, -1.0f, 1.0f, "%.2f");

        if (ImGui::Button("Reset"))
        {
            p0 = glm::vec3(-0.5f, -0.5f, 0.5f);
            p1 = glm::vec3(-0.3f, 0.0f, 0.0f);
            p2 = glm::vec3(0.5f, 0.5f, -0.5f);
        }

        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("Configure two control points, included starting or ending point to influence how final bezier curve might look like.");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
            
    ImGui::End();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/// trace to produce cubic bezier curve
/// p0 is the starting position, p3 is the ending position
/// p1, and p2 are control point
/// numSegment is number of stepping to trace, the more value is, more smooth of the curve line is.
void traceCubicBezierCurve(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, glm::vec3 outCurve[], int numSegment)
{
    // from closed form of ((1-t)^3)*P0 + 3*((1-t)^2)*t*P1 + 3*(1-t)*(t^2)*P2 + (t^3)*P3
    // to come up with such terms multiplying with P0, P1, P2, and P3, look further in
    //  - Bernstein Polynomial (https://en.wikipedia.org/wiki/Bernstein_polynomial)
    //  - Binomial coefficient (https://en.wikipedia.org/wiki/Binomial_coefficient)
    for (int i=0; i<numSegment; ++i)
    {
        float t = (i+1.0f)/numSegment;
        outCurve[i] = std::pow(1-t,3.0f)*p0 + 3*(1-t)*(1-t)*t*p1 + 3*(1-t)*t*t*p2 + std::pow(t,3.0f)*p3;
    }
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
