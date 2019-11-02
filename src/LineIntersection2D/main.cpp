/**
 * Line intersection 2D
 *
 * Involves line equation
 *
 * Actually it's still 3D but we just work in xy plane.
 * This program determine and render intersection point between two lines.
 */
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

void initMem();
void destroyMem();
void reshapeCB(GLFWwindow* window, int w, int h);
void keyboardCB(double deltaTime);
void mouseCB(GLFWwindow* window, double dx, double dy);
void mouseScrollCB(GLFWwindow* window, double dx, double dy);
void update(double dt);
void render();

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
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 6, nullptr, GL_DYNAMIC_DRAW);
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

    std::cout << glfwGetVersionString() << std::endl;;
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
        {
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        }
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

    glBindVertexArray(vao[1]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        // intersected point
        Vector3 intersectedPos;
        if (lineIntersect(Line(pVertices[0], pVertices[1]), Line(qVertices[0], qVertices[1]), intersectedPos))
        {
            std::cout << intersectedPos.x << ", " << intersectedPos.y << ", " << intersectedPos.z << std::endl;

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(intersectedPos.x, intersectedPos.y, 0.0f));
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
            glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(shader.GetUniformLocation("color"), 0.0f, 0.0f, 1.0f);

            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 6, nullptr, GL_DYNAMIC_DRAW);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3) * 6, recti, GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    glBindVertexArray(0);
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
    // defined at z=-1.0f on xy plane
}

void destroyMem()
{
    glDeleteBuffers(2, vao);
    glDeleteBuffers(2, vbo);
    shader.Destroy();
}

bool lineIntersect(const Line& p, const Line& q, Vector3& intersectedPos)
{
    // convert into glm::vec3 to make use of its glm::cross
    glm::vec3 p0 = p.pos.toGLMvec3();
    glm::vec3 pDir = p.dir.toGLMvec3();
    glm::vec3 q0 = q.pos.toGLMvec3();
    glm::vec3 qDir = q.dir.toGLMvec3();

    std::cout << glm::to_string(p0) << std::endl;
    std::cout << glm::to_string(pDir) << std::endl;
    std::cout << glm::to_string(q0) << std::endl;
    std::cout << glm::to_string(qDir) << std::endl;

    glm::vec3 p_cross_q = glm::cross(pDir, qDir);
    if (std::isnan(p_cross_q.x) || std::isnan(p_cross_q.y) || std::isnan(p_cross_q.z))
        return false;       
    if (p_cross_q.x == 0 && p_cross_q.y == 0 && p_cross_q.z == 0)
        return false;

    std::cout << "p_cross_q: " << glm::to_string(p_cross_q) << std::endl;
    glm::vec3 b = glm::cross(q0 - p0, qDir);

    float t = 0.0f;
    if (p_cross_q.x != 0.0f)
        t = b.x / p_cross_q.x;
    else if (p_cross_q.y != 0.0f)
        t = b.y / p_cross_q.y;
    else if (p_cross_q.z != 0.0f)
        t = b.z / p_cross_q.z;

    std::cout << "t: " << t << std::endl;
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

        glfwSwapBuffers(window);
    }

    destroyMem();
    glfwTerminate();

    return 0;
}
