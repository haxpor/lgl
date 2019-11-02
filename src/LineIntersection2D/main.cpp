/**
 * Line intersection 2D
 *
 * Involves line equation
 *
 * Actually it's still 3D but we just work in xy plane.
 * This program determine and render intersection point between two lines.
 *
 * Note
 *  - There are 2 copies of data one at application, and another at OpenGL on GPU's video memory.
 *    We can modify a copy at application side at will, then make (sync or async) update such copy to OpenGL.
 *    It's good to read more about implicit synchronization, and how to re-invalidate the buffer object at
 *    Khronos official website detailing about OpenGL.
 *  - Rendering line in 2d plane (at same level of depth) with depth testing enable would mess up
 *    pixels output on screen. Disable it is better.
 *
 * Features
 *  - configure slider UI to define two lines
 *  - intersection point will be rendered if there is only
 *  - self-implemented Vector3 / Line working in conjunction with glm::vec3 (we can use solely glm)
 */
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "lgl/lgl.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

// this can use glm::vec3 instead, but anyway custom structs are part of testbed
struct Vector3
{
    float x;
    float y;
    float z;

    Vector3(): x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x_, float y_, float z_): x(x_), y(y_), z(z_) {}
    Vector3(const Vector3& rhs): x(rhs.x), y(rhs.y), z(rhs.z) {}

    Vector3 operator-(const Vector3& rhs) const
    {
        return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
    }
    Vector3 operator+(const Vector3& rhs) const
    {
        return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }
    Vector3 operator*(const Vector3& rhs) const
    {
        return Vector3(x * rhs.x, y * rhs.y, z * rhs.z);
    }
    Vector3 operator*(const float& c) const
    {
        return Vector3(x * c, y * c, z * z);
    }

    Vector3& operator=(const Vector3& rhs)
    {
        this->x = rhs.x;
        this->y = rhs.y;
        this->z = rhs.z;
        return *this;
    }

    glm::vec3 toGLMvec3() const
    {
        return glm::vec3(x, y, z);
    }

    void fromGLM(const glm::vec3& rhs)
    {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
    }
};

struct Line
{
    Vector3 dir;
    Vector3 pos;

    Line(Vector3 e0, Vector3 e1):
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

void initMem();
void destroyMem();
void reshapeCB(GLFWwindow* window, int w, int h);
void keyboardCB(double deltaTime);
void mouseCB(GLFWwindow* window, double dx, double dy);
void mouseScrollCB(GLFWwindow* window, double dx, double dy);
void update(double dt);
void render();
void renderGUI();

// find intersection between line p and q
// return true if two input lines intersected, `intersectedPos` is filled with intersection position.
// Otherwise return false, and `intersectedPos` is left intact. 
bool lineIntersect(const Line& p, const Line& q, Vector3& intersectedPos);

////////////////////////
/// global variables
////////////////////////
GLuint vao[2];
GLuint vbo[2];
lgl::Shader shader;
lgl::Shader dotShader;

const float kVDepth = -1.0f;
Vector3 pVertices[2] = {
    Vector3(-0.5f, -0.5f, kVDepth),
    Vector3(0.5f, 0.2f, kVDepth)
};
Vector3 vl_pVertices[2];
Vector3 qVertices[2] = {
    Vector3(-0.3f, 0.4f, kVDepth),
    Vector3(0.4f, -0.5f, kVDepth)
};
Vector3 vl_qVertices[2];
Vector3 xAxis[2] = {
    Vector3(0.0f, 1.0f, kVDepth),
    Vector3(0.0f, -1.0f, kVDepth)
};
Vector3 yAxis[6] = {
    Vector3(-1.0f, 0.0f, kVDepth),
    Vector3(1.0f, 0.0f, kVDepth)
};
Vector3 recti[6] = {
    Vector3(0.1f, 0.1f, kVDepth),
    Vector3(-0.1f, 0.1f, kVDepth),
    Vector3(-0.1f, -0.1f, kVDepth),
    Vector3(0.1f, 0.1f, kVDepth),
    Vector3(-0.1f, -0.1f, kVDepth),
    Vector3(0.1f, -0.1f, kVDepth)
};

Vector3 dotVertex;
Vector3 tmpIntersectedPos;

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
    
    // create shader
    int result = shader.Build("data2/trans.vert", "data2/color.frag");
    LGL_ERROR_QUIT(result, "Error creating shader");

    result = dotShader.Build("data2/dot.vert", "data2/dot.frag");
    LGL_ERROR_QUIT(result, "Error creating dotShader");

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
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, nullptr, GL_STREAM_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), nullptr);
        glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // prepare for rect (intersection) vao
    glBindVertexArray(vao[1]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 4, nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), nullptr);
        glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // compute once the PROJECTION * VIEW matrix, then submit to GPU
    // notice we use orthographic projection matrix in cube shape to make our line definition easily to define (see initMem())
    shader.Use();
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    glm::mat4 transform = projection * view;
    glUniformMatrix4fv(shader.GetUniformLocation("pv"), 1, GL_FALSE, glm::value_ptr(transform));
    lgl::error::AnyGLError();

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
    lgl::error::AnyGLError();

    dotShader.Use();
    glUniformMatrix4fv(dotShader.GetUniformLocation("pv"), 1, GL_FALSE, glm::value_ptr(transform));
    lgl::error::AnyGLError();
    glUniformMatrix4fv(dotShader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
    lgl::error::AnyGLError();

    std::cout << glfwGetVersionString() << std::endl;;

    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    
    shader.Use();

    glBindVertexArray(vao[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        // x-axis
        glUniform3f(shader.GetUniformLocation("color"), 1.0f, 1.0f, 1.0f);
        // invalidate entire buffer (orphan)
        // invalidate will put old storage on the free list once there is no other rendering commands
        // using it. Same goes for other lines.
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, nullptr, GL_STREAM_DRAW);
        // update buffer
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, xAxis, GL_STREAM_DRAW);
        glDrawArrays(GL_LINES, 0, 6);

        // y-axis
        glUniform3f(shader.GetUniformLocation("color"), 1.0f, 1.0f, 1.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, nullptr, GL_STREAM_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, yAxis, GL_STREAM_DRAW);
        glDrawArrays(GL_LINES, 0, 6);

        // virtual line p
        glUniform3f(shader.GetUniformLocation("color"), 0.5f, 0.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, nullptr, GL_STREAM_DRAW);
        {
            Vector3 dir = pVertices[1] - pVertices[0];
            // find coordinate at highest position in y-axis line can reach
            // - find t
            float t;
            t = (-1.0f - pVertices[0].y) / dir.y;
            // - substitute to find position
            vl_pVertices[0] = pVertices[0] + dir * t;

            // find coordinate at lowest position in y-axis line can reach
            t = (1.0f - pVertices[0].y) / dir.y;
            vl_pVertices[1] = pVertices[0] + dir * t;
        }
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, vl_pVertices, GL_STREAM_DRAW);
        glDrawArrays(GL_LINES, 0, 6);

        // line p
        glUniform3f(shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, nullptr, GL_STREAM_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, pVertices, GL_STREAM_DRAW);
        glDrawArrays(GL_LINES, 0, 6);

        // virtual line q
        glUniform3f(shader.GetUniformLocation("color"), 0.0f, 0.5f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, nullptr, GL_STREAM_DRAW);
        {
            Vector3 dir = qVertices[1] - qVertices[0];
            float t;
            t = (-1.0f - qVertices[0].y) / dir.y;
            vl_qVertices[0] = qVertices[0] + dir * t;

            t = (1.0f - qVertices[0].y) / dir.y;
            vl_qVertices[1] = qVertices[0] + dir * t;
        }
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, vl_qVertices, GL_STREAM_DRAW);
        glDrawArrays(GL_LINES, 0, 6);

        // line q
        glUniform3f(shader.GetUniformLocation("color"), 0.0f, 1.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, nullptr, GL_STREAM_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 2, qVertices, GL_STREAM_DRAW);
        glDrawArrays(GL_LINES, 0, 6);

    dotShader.Use();
    glBindVertexArray(vao[1]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

        // line p - tipping point 0
        dotVertex = pVertices[0];
        glUniform3f(shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3), &dotVertex, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, 1);
        
        // line p - tipping point 1
        dotVertex = pVertices[1];
        glUniform3f(shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3), &dotVertex, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, 1);

        // line q - tipping point 0
        dotVertex = qVertices[0];
        glUniform3f(shader.GetUniformLocation("color"), 0.0f, 1.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3), &dotVertex, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, 1);
        
        // line q - tipping point 1
        dotVertex = qVertices[1];
        glUniform3f(shader.GetUniformLocation("color"), 0.0f, 1.0f, 0.0f);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3), &dotVertex, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, 1);

        // intersected point
        if (lineIntersect(Line(pVertices[0], pVertices[1]), Line(qVertices[0], qVertices[1]), tmpIntersectedPos))
        {
            dotVertex = tmpIntersectedPos;
            glUniform3f(shader.GetUniformLocation("color"), 0.0f, 0.0f, 0.0f);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3), nullptr, GL_DYNAMIC_DRAW);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3), &dotVertex, GL_DYNAMIC_DRAW);
            glDrawArrays(GL_POINTS, 0, 1);
        }
    glBindVertexArray(0);
}

void renderGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

#define IMGUI_WINDOW_WIDTH 200
#define IMGUI_WINDOW_HEIGHT 300
#define IMGUI_WINDOW_MARGIN 5
    ImGui::SetNextWindowSize(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT));
    ImGui::SetNextWindowSizeConstraints(ImVec2(IMGUI_WINDOW_WIDTH, IMGUI_WINDOW_HEIGHT), ImVec2(IMGUI_WINDOW_WIDTH,IMGUI_WINDOW_HEIGHT));
    //ImGui::SetNextWindowPos(ImVec2(screenWidth - IMGUI_WINDOW_WIDTH - IMGUI_WINDOW_MARGIN, IMGUI_WINDOW_MARGIN));
    ImGui::Begin("LineIntersection2D");
        //static bool mm = true;
        //ImGui::ShowDemoWindow(&mm);
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::TextColored(ImVec4(1.0f,1.0f,1.0f,1.0f), "Line 1");
        ImGui::PopStyleColor();
        // every frame we invalidate the buffer, and submit new to OpenGL
        ImGui::SliderFloat("P0 x", &pVertices[0].x, -0.5f, 0.5f, "%.2f");
        ImGui::SliderFloat("P0 y", &pVertices[0].y, -0.5f, 0.5f, "%.2f");

        // every frame we invalidate the buffer, and submit new to OpenGL
        ImGui::SliderFloat("P1 x", &pVertices[1].x, -0.5f, 0.5f, "%.2f");
        ImGui::SliderFloat("P1 y", &pVertices[1].y, -0.5f, 0.5f, "%.2f");

        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.4f, 1.0f));
        ImGui::TextColored(ImVec4(1.0f,1.0f,1.0f,1.0f), "Line 2");
        ImGui::PopStyleColor();
        // every frame we invalidate the buffer, and submit new to OpenGL
        ImGui::SliderFloat("Q0 x", &qVertices[0].x, -0.5f, 0.5f, "%.2f");
        ImGui::SliderFloat("Q0 y", &qVertices[0].y, -0.5f, 0.5f, "%.2f");

        // every frame we invalidate the buffer, and submit new to OpenGL
        ImGui::SliderFloat("Q1 x", &qVertices[1].x, -0.5f, 0.5f, "%.2f");
        ImGui::SliderFloat("Q1 y", &qVertices[1].y, -0.5f, 0.5f, "%.2f");
            
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
    // do nothing
}

void mouseScrollCB(GLFWwindow* window, double dx, double dy)
{
    // do nothing
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
    dotShader.Destroy();
}

bool lineIntersect(const Line& p, const Line& q, Vector3& intersectedPos)
{
    // convert into glm::vec3 to make use of its glm::cross
    glm::vec3 p0 = p.pos.toGLMvec3();
    glm::vec3 pDir = p.dir.toGLMvec3();
    glm::vec3 q0 = q.pos.toGLMvec3();
    glm::vec3 qDir = q.dir.toGLMvec3();

    glm::vec3 p_cross_q = glm::cross(pDir, qDir);
    if (std::isnan(p_cross_q.x) || std::isnan(p_cross_q.y) || std::isnan(p_cross_q.z))
        return false;       
    if (p_cross_q.x == 0 && p_cross_q.y == 0 && p_cross_q.z == 0)
        return false;

    glm::vec3 b = glm::cross(q0 - p0, qDir);

    float t = 0.0f;
    if (p_cross_q.x != 0.0f)
        t = b.x / p_cross_q.x;
    else if (p_cross_q.y != 0.0f)
        t = b.y / p_cross_q.y;
    else if (p_cross_q.z != 0.0f)
        t = b.z / p_cross_q.z;

    glm::vec3 pout = p0 + pDir * t;
    intersectedPos.fromGLM(pout);
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
        renderGUI();

        glfwSwapBuffers(window);
    }

    destroyMem();
    glfwTerminate();

    return 0;
}
