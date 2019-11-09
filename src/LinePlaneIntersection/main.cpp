/**
 * Line-Plane Intersection
 *
 * Based on top of LineIntersection3D code, and modifed to support line-plane intersection.
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
void renderPlane_geometry(const Plane& p);

/// find intersection between line-plane
/// return intersected position
bool linePlaneIntersect(const Line& l, const Plane& p, glm::vec3& intersectedPos);
/// finer intersection between line-plane and limit intersection to be within
/// plane itself.
bool linePlaneIntersectWithinPlane(const Line& l, const Plane& p, const glm::vec3 planeCorners[4], glm::vec3& intersectedPos);

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
GLuint vao[2];
GLuint vbo[2];
lgl::Shader shader;
Sphere dot(20, 20, 0.03f);
Sphere planeDot(10,10, 0.007f);
Gizmo gizmo;

glm::vec3 pVertices[2] = {
    glm::vec3(-0.5f, -0.5f, 0.5f),
    glm::vec3(0.5f, 0.47f, -0.5f)
};
glm::vec3 vl_pVertices[4];

#define PLANE_SIZE_FACTOR 0.4f
Plane plane(glm::vec3(0.0f, 0.1f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
glm::vec3 planeNotNecessaryNormalizedNormal = plane.normal;
glm::vec3 planeCornersVertices[4];
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

glm::vec3 dotVertex;
glm::vec3 tmpIntersectedPos;

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
    glGenVertexArrays(2, vao);
    glGenBuffers(2, vbo);

    // prepare for line vao
    glBindVertexArray(vao[0]);
        // a single VBO shares both vertices from both p and q lines to render on screen
        // although having dedicated VBO for each line should be higher performance, but this is also
        // another testbed setup for this program
        //
        // so we supply 'data` parameter as nullptr and we will do buffer update in render loop
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
        glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // prepare for plane vao
    glBindVertexArray(vao[1]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, nullptr, GL_DYNAMIC_DRAW);
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
void renderPlane_geometry(const Plane& p)
{
    // instead of manually rotate the matrix in sequence to orient the object to look into plane's normal
    // we compute lookAt matrix (similarly to camera's lookAt but direction vector is pointing into target position,
    // and ignore positional inforation, and rotational matrix is not transpose).
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, p.pos);
    model = model * computeLookAtForObject(p.pos, p.pos + p.normal);
    glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(shader.GetUniformLocation("color"), 0.0f, 0.6f, 0.7f);

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

    // 2. render plane normal
    planeNormalLineVertices[0] = p.pos;
    planeNormalLineVertices[1] = p.pos + 0.5f*p.normal;
    glUniform3f(shader.GetUniformLocation("color"), 1.0f, 1.0f, 0.0f);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, planeNormalLineVertices, GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, 2);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    
    shader.Use();

    glBindVertexArray(vao[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        // plane
        renderPlane_geometry(plane);

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

        // virtual line p
        glUniform3f(shader.GetUniformLocation("color"), 0.5f, 0.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
        {
            glm::vec3 dir = glm::normalize((pVertices[1] - pVertices[0]));
            // determine which which tip-end is positive based on computed direction
            float dotProduct0 = glm::dot(pVertices[0], dir);
            float dotProduct1 = glm::dot(pVertices[1], dir);
            float dirFactor = 1.0f; // initially positive end is at pVertices[1]
            if (dotProduct0 > dotProduct1)
            {
                // positive end is at pVertices[0] instead
                dirFactor = -1.0f;
            }

            // start from both of the tips of the line and go both way of negative and positive
            // for amount of 1.0f
            vl_pVertices[0] = pVertices[0];
            vl_pVertices[1] = pVertices[0] + dir*(-dirFactor);
            vl_pVertices[2] = pVertices[1];
            vl_pVertices[3] = pVertices[1] + dir*dirFactor;
        }
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, vl_pVertices, GL_STREAM_DRAW);
        glDrawArrays(GL_LINES, 0, 2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, vl_pVertices + 2, GL_STREAM_DRAW);
        glDrawArrays(GL_LINES, 0, 2);

        // line p
        glUniform3f(shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, nullptr, GL_STREAM_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, pVertices, GL_STREAM_DRAW);
        glDrawArrays(GL_LINES, 0, 2);

    // compute plane's corners for both use in
    // 1. (within) plane intersection
    // 2. debugging draw for spheres on all plane's corners
    computePlaneCorners(plane, planeCornersVertices);

    dot.shader.Use();
    dot.drawBatchBegin();
        // line p - tipping point 0
        dotVertex = pVertices[0];
        glUniform3f(dot.shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
        glUniformMatrix4fv(dot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
                glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
        dot.drawBatchDraw();
        
        // line p - tipping point 1
        dotVertex = pVertices[1];
        glUniform3f(dot.shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
        glUniformMatrix4fv(dot.shader.GetUniformLocation("model"), 1, GL_FALSE,
                glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
        dot.drawBatchDraw();

        // intersected point
        if (linePlaneIntersectWithinPlane(Line(pVertices[0], pVertices[1]), plane, planeCornersVertices, tmpIntersectedPos))
        {
            dotVertex = tmpIntersectedPos;
            glUniform3f(dot.shader.GetUniformLocation("color"), 0.0f, 0.0f, 0.0f);
            glUniformMatrix4fv(dot.shader.GetUniformLocation("model"), 1, GL_FALSE,
                    glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
            dot.drawBatchDraw();
        }
    dot.drawBatchEnd();

    planeDot.shader.Use();
    planeDot.drawBatchBegin();
        // top-right plane-corner
        dotVertex = planeCornersVertices[0];
        glUniform3f(planeDot.shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
        glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
                glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
        planeDot.drawBatchDraw();

        dotVertex = planeCornersVertices[1];
        glUniform3f(planeDot.shader.GetUniformLocation("color"), 0.0f, 1.0f, 0.0f);
        glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
                glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
        planeDot.drawBatchDraw();

        dotVertex = planeCornersVertices[2];
        glUniform3f(planeDot.shader.GetUniformLocation("color"), 0.0f, 0.0f, 1.0f);
        glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
                glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
        planeDot.drawBatchDraw();

        dotVertex = planeCornersVertices[3];
        glUniform3f(planeDot.shader.GetUniformLocation("color"), 0.0f, 1.0f, 1.0f);
        glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
                glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
        planeDot.drawBatchDraw();
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

#define IMGUI_WINDOW_WIDTH 200
#define IMGUI_WINDOW_HEIGHT 290
#define IMGUI_WINDOW_MARGIN 5
    ImGui::SetNextWindowSize(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT));
    ImGui::SetNextWindowSizeConstraints(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT), ImVec2(IMGUI_WINDOW_WIDTH,IMGUI_WINDOW_HEIGHT));
    //ImGui::SetNextWindowPos(ImVec2(screenWidth - IMGUI_WINDOW_WIDTH - IMGUI_WINDOW_MARGIN, IMGUI_WINDOW_MARGIN));
    ImGui::Begin("LineIntersection2D");
        //static bool mm = true;
        //ImGui::ShowDemoWindow(&mm);
        
        ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "Line");
        // every frame we invalidate the buffer, and submit new to OpenGL
        ImGui::SliderFloat("P0 x", &pVertices[0].x, -0.5f, 0.5f, "%.2f");
        ImGui::SliderFloat("P0 y", &pVertices[0].y, -0.5f, 0.5f, "%.2f");
        ImGui::SliderFloat("P0 z", &pVertices[0].z, -0.5f, 0.5f, "%.2f");

        ImGui::SliderFloat("P1 x", &pVertices[1].x, -0.5f, 0.5f, "%.2f");
        ImGui::SliderFloat("P1 y", &pVertices[1].y, -0.5f, 0.5f, "%.2f");
        ImGui::SliderFloat("P1 z", &pVertices[1].z, -0.5f, 0.5f, "%.2f");

        ImGui::Separator();

        ImGui::TextColored(ImVec4(0.0f,1.0f,1.0f,1.0f), "Plane");
        // plane normal
        bool isPlaneNormalModified = false;
        if (ImGui::SliderFloat("Norm x", &planeNotNecessaryNormalizedNormal.x, -1.0f, 1.0f, "%.2f"))
            isPlaneNormalModified = true;
        if (ImGui::SliderFloat("Norm y", &planeNotNecessaryNormalizedNormal.y, -1.0f, 1.0f, "%.2f"))
            isPlaneNormalModified = true;
        if (ImGui::SliderFloat("Norm z", &planeNotNecessaryNormalizedNormal.z, -1.0f, 1.0f, "%.2f"))
            isPlaneNormalModified = true;

        // TODO: probably need to check all zeros case which is not valid for plane's normal
        if (isPlaneNormalModified)
            plane.normal = glm::normalize(planeNotNecessaryNormalizedNormal);
            
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
    glDeleteBuffers(2, vao);
    glDeleteBuffers(2, vbo);
    shader.Destroy();
    dot.destroyGLObjects();
    planeDot.destroyGLObjects();
}

bool linePlaneIntersect(const Line& l, const Plane& p, glm::vec3& intersectedPos)
{
    glm::vec3 n = p.normal;
    glm::vec3 v = l.pos;
    glm::vec3 vdir = l.dir;

    float d = p.getD();
    float denom = (n.x*vdir.x + n.y*vdir.y + n.z*vdir.z);
    if (std::abs(denom) < kEpsilon)
        return false;
    float t = -(n.x*v.x + n.y*v.y + n.z*v.z + d) / denom;
    
    // (optional) check segment, only intersection can happen within the defined segment of the line
    // and within the plane itself (t = 0) (not extended unlimited)
    if (t < 0.0f || t > 1.0f)
        return false;

    intersectedPos = v + vdir*t;
    return true;
}

bool linePlaneIntersectWithinPlane(const Line& l, const Plane& p, const glm::vec3 planeCorners[4], glm::vec3& intersectedPos)
{
    glm::vec3 n = p.normal;
    glm::vec3 v = l.pos;
    glm::vec3 vdir = l.dir;

    float d = p.getD();
    float denom = (n.x*vdir.x + n.y*vdir.y + n.z*vdir.z);
    if (std::abs(denom) < kEpsilon)
        return false;
    float t = -(n.x*v.x + n.y*v.y + n.z*v.z + d) / denom;
    
    // (optional) check segment, only intersection can happen within the defined segment of the line
    // and within the plane itself (t = 0) (not extended unlimited)
    if (t < 0.0f || t > 1.0f)
        return false;

    glm::vec3 chkIntersectedPos = v + vdir*t;

    // check if intersected position is within plane using Heron's Formulae
    // so the edges forming from the points starting at top-right, then goes counter-clockwise
    float a1 = glm::distance(planeCorners[0], planeCorners[1]);
    float a2 = glm::distance(planeCorners[1], planeCorners[2]);
    float a3 = glm::distance(planeCorners[2], planeCorners[3]);
    float a4 = glm::distance(planeCorners[3], planeCorners[0]);

    float A = a1*a2;

    float b1 = glm::distance(chkIntersectedPos, planeCorners[0]);
    float b2 = glm::distance(chkIntersectedPos, planeCorners[1]);
    float b3 = glm::distance(chkIntersectedPos, planeCorners[2]);
    float b4 = glm::distance(chkIntersectedPos, planeCorners[3]);

    float u1 = (b1 + b2 + a1) / 2.0f;
    float u2 = (b2 + b3 + a2) / 2.0f;
    float u3 = (b3 + b4 + a3) / 2.0f;
    float u4 = (b1 + b4 + a4) / 2.0f;

    float A1 = std::sqrt(u1 * (u1 - a1) * (u1 - b1) * (u1 - b2));
    float A2 = std::sqrt(u2 * (u2 - a2) * (u2 - b2) * (u2 - b3));
    float A3 = std::sqrt(u3 * (u3 - a3) * (u3 - b3) * (u3 - b4));
    float A4 = std::sqrt(u4 * (u4 - a4) * (u4 - b1) * (u4 - b4));

    if (std::abs(A - (A1 + A2 + A3 + A4)) >= 0.001f)
        return false;

    intersectedPos = chkIntersectedPos;

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
