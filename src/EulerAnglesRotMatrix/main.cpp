/**
 *  EulerAnglesRotMatrix
 *  
 *  Based on top of ArbitraryRotationAxis then added implementation of Euler Angle's resultant
 *  rotational matrix for RxRyRz order (Rz happens first) to validate whether it will produce the
 *  same result as normal sequence of rotational matrix according to pre-defined sequence of rotation
 *  via normal matrix rotational operations. Provided that both approaches rotate around the cardinal
 *  axes.
 *
 *  As well we have another rotational sequence RzRyRx (Rx happens first) to validate the logic
 *  as it should be different from first case.
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
void renderPlane_geometry(const Plane& p, const glm::mat3& orientation, const glm::vec3& color, const glm::vec3& normColor);
void computePlaneCorners(const Plane& p, const glm::mat3& orientation, glm::vec3 outCorners[4]);
glm::mat3 rotateAroundAxis(const glm::vec3& axis, float angle);
glm::mat3 rotateEulerAnglesXYZ(float angleX, float angleY, float angleZ);
glm::mat3 rotateEulerAnglesZYX(float angleX, float angleY, float angleZ);

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
Plane plane1(glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
Plane plane2(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
Plane plane3(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

glm::vec3 planeRotXYZ = glm::vec3(0.0f);

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
// shared variables used to draw plane and its accessories
glm::vec3 planeCornersVertices[4];
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


void computePlaneCorners(const Plane& p, const glm::mat3& orientation, glm::vec3 outCorners[4])
{
    // convert from vec4 to vec3
    glm::vec3 left = orientation[0];
    glm::vec3 up = orientation[1];

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

/// create a rotational matrix to rotate around the input `axis` for `angle`.
/// it creates a new mat3.
glm::mat3 rotateAroundAxis(const glm::vec3& axis, float angle)
{
    float nx_squared = axis.x * axis.x;
    float ny_squared = axis.y * axis.y;
    float nz_squared = axis.z * axis.z;
    float nx_ny = axis.x * axis.y;
    float nx_nz = axis.x * axis.z;
    float ny_nz = axis.y * axis.z;
    float costheta = std::cos(angle);
    float sintheta = std::sin(angle);
    float one_costheta = 1.0f - costheta;
    
    return glm::mat3(
            nx_squared*one_costheta + costheta,     nx_ny*one_costheta + sintheta*axis.z,   nx_nz*one_costheta - sintheta*axis.y,
            nx_ny*one_costheta - sintheta*axis.z,   ny_squared*one_costheta + costheta,     ny_nz*one_costheta + sintheta*axis.x,
            nx_nz*one_costheta + sintheta*axis.y,   ny_nz*one_costheta - sintheta*axis.x,   nz_squared*one_costheta + costheta);
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

/// create a combined rotational matrix via euler angles method in order of ZYX
glm::mat3 rotateEulerAnglesZYX(float angleX, float angleY, float angleZ)
{
    float c1 = std::cos(angleX); 
    float s1 = std::sin(angleX);
    float c2 = std::cos(angleY);
    float s2 = std::sin(angleY);
    float c3 = std::cos(angleZ);
    float s3 = std::sin(angleZ);

    return glm::mat3(c3*c2,         -s3*c1 + c3*s2*s1,          s3*s1 + c3*s2*c1,
                     s3*c2,         c3*c1 + s3*s2*s1,           -c3*s1 + s3*s2*c1,
                     -s2,           c2*s1,                      c2*c1);
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

// required: dotShader needs to already began
void drawDotAtPlaneCorners(const glm::vec3 corners[4])
{
    // top-right plane-corner
    dotVertex = corners[0];
    glUniform3f(planeDot.shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
    glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
            glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
    planeDot.drawBatchDraw();

    // top-left plane-corner
    dotVertex = corners[1];
    glUniform3f(planeDot.shader.GetUniformLocation("color"), 0.0f, 1.0f, 0.0f);
    glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
            glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
    planeDot.drawBatchDraw();

    // bottom-left plane-corner
    dotVertex = corners[2];
    glUniform3f(planeDot.shader.GetUniformLocation("color"), 0.0f, 0.0f, 1.0f);
    glUniformMatrix4fv(planeDot.shader.GetUniformLocation("model"), 1, GL_FALSE, 
            glm::value_ptr(glm::translate(glm::mat4(1.0f), dotVertex)));
    planeDot.drawBatchDraw();

    // bottom-right plane-corner
    dotVertex = corners[3];
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
        // compute rotation transformation matrix for plane
        // rotate in sequence in order of XYZ (rotate around z-axis first)
        glm::mat3 orientation = rotateAroundAxis(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(planeRotXYZ.x)) *
            rotateAroundAxis(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(planeRotXYZ.y)) *
            rotateAroundAxis(glm::vec3(0.0f, 0.0f, 1.0f), glm::radians(planeRotXYZ.z));
        renderPlane_geometry(plane1, orientation, glm::vec3(0.0f, 0.6f, 0.7f), glm::vec3(0.0f, 0.8f, 1.0f));

        glm::mat3 orientation2 = rotateEulerAnglesXYZ(glm::radians(planeRotXYZ.x), glm::radians(planeRotXYZ.y), glm::radians(planeRotXYZ.z));
        renderPlane_geometry(plane2, orientation2, glm::vec3(0.6f, 0.3f, 0.3f), glm::vec3(0.0f, 0.8f, 1.0f));

        glm::mat3 orientation3 = rotateEulerAnglesZYX(glm::radians(planeRotXYZ.x), glm::radians(planeRotXYZ.y), glm::radians(planeRotXYZ.z));
        renderPlane_geometry(plane3, orientation3, glm::vec3(0.4f, 0.4f, 0.0f), glm::vec3(0.0f, 0.8f, 1.0f));

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

        planeDot.shader.Use();
        planeDot.drawBatchBegin();
            // compute plane's corners for both use in
            // 1. (within) plane intersection
            // 2. debugging draw for spheres on all plane's corners
            computePlaneCorners(plane1, orientation, planeCornersVertices);
            drawDotAtPlaneCorners(planeCornersVertices);

            computePlaneCorners(plane2, orientation2, planeCornersVertices);
            drawDotAtPlaneCorners(planeCornersVertices);

            computePlaneCorners(plane3, orientation3, planeCornersVertices);
            drawDotAtPlaneCorners(planeCornersVertices);
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
#define IMGUI_WINDOW_HEIGHT 180
#define IMGUI_WINDOW_MARGIN 5
    ImGui::SetNextWindowSize(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT));
    ImGui::SetNextWindowSizeConstraints(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT), ImVec2(IMGUI_WINDOW_WIDTH,IMGUI_WINDOW_HEIGHT));
    //ImGui::SetNextWindowPos(ImVec2(screenWidth - IMGUI_WINDOW_WIDTH - IMGUI_WINDOW_MARGIN, IMGUI_WINDOW_MARGIN));
    ImGui::Begin("Configurations");
        //static bool mm = true;
        //ImGui::ShowDemoWindow(&mm);

        static const float f32_zero = 0.f, f32_360 = 360.0f, f32_neg_180 = -180.0f, f32_pos_180 = 180.f, f32_neg_90 = -90.0f, f32_pos_90 = 90.0f;

        // plane transformation
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Plane Rot (in degrees)");
        if (ImGui::DragScalar("Z", ImGuiDataType_Float, &planeRotXYZ.z, 1.0f, &f32_zero, &f32_360, "%f", 1.0f))
        {
            // bound value entered from user
            if (planeRotXYZ.z < f32_zero)
                planeRotXYZ.z = f32_zero;
            else if (planeRotXYZ.z > f32_360)
                planeRotXYZ.z = f32_360;
        }
        if (ImGui::DragScalar("Y", ImGuiDataType_Float, &planeRotXYZ.y, 1.0f, &f32_neg_180, &f32_pos_180, "%f", 1.0f))
        {
            if (planeRotXYZ.y < f32_neg_180)
                planeRotXYZ.y = f32_neg_180;
            else if (planeRotXYZ.y > f32_pos_180)
                planeRotXYZ.y = f32_pos_180;
        }
        if (ImGui::DragScalar("X", ImGuiDataType_Float, &planeRotXYZ.x, 1.0f, &f32_neg_90, &f32_pos_90, "%f", 1.0f))
        {
            if (planeRotXYZ.z < f32_neg_90)
                planeRotXYZ.z = f32_neg_90;
            else if (planeRotXYZ.z > f32_pos_90)
                planeRotXYZ.z = f32_pos_90;
        }

        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("XYZ rotation used for left-most, and middle plane.\nZYX is used for right-most plane.");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
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
