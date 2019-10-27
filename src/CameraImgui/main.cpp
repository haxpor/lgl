/**
 * Camera manipulation with self-implementation to form look-at matrix, with ImGUI for manipulate
 * the camera + Gizmo.
 *
 * This version modifies Camera/ sample to not capture mouse cursor, and force it to be in the central
 * of screen all the time. User can activate using mouse movement to manipulate the camera by hold
 * pressing left-mouse button then pan mouse normally. This is to allow mouse input to be able to
 * interact with GUI as well.
 *
 * Gizmo is rendered on screen by using glViewport to specify the lower-left area with always-pass
 * depth-testing (still allow depth-buffer writing). It has main box, 3 lines as axeses, and another
 * 3 boxes at the tip of each axis. Box vertex attribute data is shared with normal boxes rendered in
 * base scene. It uses just one vertex/fragment shader to render all of its components with
 * customization of color (along with model, view, and projection matrix).
 *
 * Control:
 * - hold left-mouse button then pan to look around
 * - while holding left-mouse button, w/s/a/d to walk around
 * - space key when configuration window is closed to show it again
 * - escape key to quit the program
 *
 * Integration:
 * Integrating with Dear ImGUI, notice 'imgui_impl_opengl3.cpp' file in which inclusion of glad/glad.h
 * uses "" double quote instead of <> as we bundle our generated glad.h header with the project.
 *
 */
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "lgl/lgl.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

int screenWidth = 800;
int screenHeight = 600;
GLFWwindow* window = nullptr;
double prevTicks;
double lastMouseX, lastMouseY;
glm::mat4 view, projection;

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
void initGizmo();

void initMem();
void destroyMem();
void reshapeCB(GLFWwindow* window, int w, int h);
void keyboardCB(double deltaTime);
void mouseCB(GLFWwindow* window, double dx, double dy);
void mouseScrollCB(GLFWwindow* window, double dx, double dy);
void mouseButtonCB(GLFWwindow* window, int button, int action, int mods);
void update(double dt);
void render();
void renderGUI();
void renderGizmo();

glm::mat4 selfImplemented_lookAt(glm::vec3 pos, glm::vec3 targetPos, glm::vec3 up);

////////////////////////
/// global variables
////////////////////////
GLuint vao, gizmoVAO[2];
GLuint vbo, gizmoVBO[2];
lgl::Shader shader;
lgl::Shader gizmoShader;
GLuint containerTexture;
GLuint awesomeTexture;

glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
float camYaw = -90.0f, camPitch = 0.0f;
float camFov = 45.0f;
float mixFactor = 0.5f;
bool isLeftMousePressed = false;
bool isGuiWindowShow = true;
float bgColor[3] = { 51/256.0f, 43/256.0f, 58.0f/256.0f };

float vertices[] = {
    // positions          // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

glm::vec3 cubePositions[] = {
  glm::vec3( 0.0f,  0.0f,  0.0f),
  glm::vec3( 2.0f,  5.0f, -15.0f),
  glm::vec3(-1.5f, -2.2f, -2.5f),
  glm::vec3(-3.8f, -2.0f, -12.3f),
  glm::vec3( 2.4f, -0.4f, -3.5f),
  glm::vec3(-1.7f,  3.0f, -7.5f),
  glm::vec3( 1.3f, -2.0f, -2.5f),
  glm::vec3( 1.5f,  2.0f, -2.5f),
  glm::vec3( 1.5f,  0.2f, -1.5f),
  glm::vec3(-1.3f,  1.0f, -1.5f)
};

unsigned int indices[] =
{
    0, 1, 2,
    0, 2, 3
};

float gizmoUpLinePoints[] = {
    0.0f, 0.0f, 0.0f,
    0.0f, 0.30f, 0.0f
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
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

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
    glfwSetMouseButtonCallback(window, mouseButtonCB);
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
    glEnable(GL_DEPTH_TEST);

    GLint majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    std::cout << "OpenGL version in use: " << majorVersion << "." << minorVersion << std::endl;
    
    // create shader
    int result = shader.Build("data/tex2.vert", "data/multitex.frag");
    LGL_ERROR_QUIT(result, "Error creating shader");

    // load textures
    containerTexture = lgl::util::LoadTexture("data/container.jpg");
    if (lgl::error::AnyGLError() != 0)
        lgl::error::ErrorExit("Error loading data/container.jpg");
    // modify its texture filtering
    glBindTexture(GL_TEXTURE_2D, containerTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    awesomeTexture = lgl::util::LoadTexture("data/awesomeface.png");
    if (lgl::error::AnyGLError() != 0)
        lgl::error::ErrorExit("Error loading data/awesomeface.png");

    // prepare vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (static_cast<char*>(0) + 3 * sizeof(float)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindTexture(GL_TEXTURE_2D, containerTexture);
    glBindVertexArray(0);

    // set uniform values
    shader.Use();
    glActiveTexture(GL_TEXTURE0);
    shader.SetUniform("textureSampler", 0);
    glActiveTexture(GL_TEXTURE1);
    shader.SetUniform("textureSampler2", 1);
    shader.SetUniform("mixFactor", 0.5f);

    // compute view matrix
    // two version of implementations provided: 1. via GLM 2. Self-implemented
    //glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
    view = selfImplemented_lookAt(camPos, camPos + camFront, camUp);
    glUniformMatrix4fv(shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

    // compute projection matrix
    projection = glm::perspective(glm::radians(camFov), screenWidth * 1.0f / screenHeight, 0.1f, 100.0f);
    glUniformMatrix4fv(shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

    std::cout << glfwGetVersionString() << std::endl;;

    initGizmo();
    initImGUI();
}

void initGizmo()
{
    // create gizmo's box shader
    int result = gizmoShader.Build("data2/gizmo.vert", "data2/gizmo.frag");
    LGL_ERROR_QUIT(result, "Error creating gizmo shader");

    // box
    glGenVertexArrays(2, gizmoVAO);
    glGenBuffers(2, gizmoVBO);

    glBindVertexArray(gizmoVAO[0]);
        glBindBuffer(GL_ARRAY_BUFFER, gizmoVBO[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // lines
    glBindVertexArray(gizmoVAO[1]);
        glBindBuffer(GL_ARRAY_BUFFER, gizmoVBO[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(gizmoUpLinePoints), gizmoUpLinePoints, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    gizmoShader.Use();
    glUniform3f(gizmoShader.GetUniformLocation("color"), 0.7f, 0.7f, 0.7);

    glm::mat4 viewCopy = glm::mat4(view);
    viewCopy[3][0] = 0.0f;
    viewCopy[3][1] = 0.0f;
    viewCopy[3][2] = -1.0f;     // move the camera back slightly to see all angle of object
    glUniformMatrix4fv(gizmoShader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(viewCopy));

    glm::mat4 projection = glm::perspective(glm::radians(camFov), 1.0f, 0.1f, 100.0f);
    glUniformMatrix4fv(gizmoShader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
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
    glClearColor(bgColor[0], bgColor[1], bgColor[2], 1.0f);
    glDepthFunc(GL_LESS);

    glViewport(0, 0, static_cast<GLsizei>(screenWidth), static_cast<GLsizei>(screenHeight));

    shader.Use();
    // bind 1st texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, containerTexture);
    // bind 2nd texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, awesomeTexture);

    glBindVertexArray(vao);
        // two version of implementations provided: 1. via GLM 2. Self-implemented
        //glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        view = selfImplemented_lookAt(camPos, camPos + camFront, camUp);
        glUniformMatrix4fv(shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

        for (std::size_t i=0; i<10; ++i)
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePositions[i]);
            const float angle = 35.0f * i + 20.0f;
            model = glm::rotate(model, static_cast<float>(glfwGetTime()) * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    glBindVertexArray(0);
}

void renderGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    if (isGuiWindowShow)
    {
        ImGui::SetNextWindowSize(ImVec2(400, 200));
        ImGui::SetNextWindowSizeConstraints(ImVec2(400, 200), ImVec2(400,200));
        ImGui::SetNextWindowPos(ImVec2(screenWidth / 2 - 5, screenHeight-200-5));
        ImGui::Begin("CameraImgui (fixed pos & size, SPACE to reopen)", &isGuiWindowShow);

        //static bool mm = true;
        //ImGui::ShowDemoWindow(&mm);
        
        ImGui::LabelText("Configuration", "Value");
        if (ImGui::SliderFloat("mixFactor", &mixFactor, 0.0f, 1.0f, "%.2f"))
            shader.SetUniform("mixFactor", mixFactor);
        ImGui::ColorEdit3("BG color", bgColor);
    
        static float values[90] = {};
        static int values_offset = 0;
        values[values_offset] = static_cast<float>((glfwGetTime() - prevTicks) * 1000.0f);
        values_offset = (values_offset+1) % IM_ARRAYSIZE(values);
        {
            float maxValue = values[0];
            for (std::size_t i=1; i<IM_ARRAYSIZE(values); ++i)
            {
                if (values[i] > maxValue)
                   maxValue = values[i]; 
            }
            char overlay[10];
            std::snprintf(overlay, sizeof(overlay), "%.02f ms", values[values_offset-1]);
            ImGui::PlotLines("Elapsed Time", values, IM_ARRAYSIZE(values), values_offset, overlay, 0.0f, maxValue+0.5f, ImVec2(0,40));
        }

        ImGui::Text("Cam Pos: %.02f, %.02f, %.02f", camPos.x, camPos.y, camPos.z);
        ImGui::Text("Cam Front: %.02f, %.02f, %.02f", camFront.x, camFront.y, camFront.y); 

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void renderGizmo()
{
    glViewport(0, 0, 100, 100);
    glDepthFunc(GL_ALWAYS);

    gizmoShader.Use();

    glm::mat4 viewCopy = glm::mat4(view);
    viewCopy[3][0] = 0.0f;
    viewCopy[3][1] = -0.08f;
    viewCopy[3][2] = -1.0f;
    glUniformMatrix4fv(gizmoShader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(viewCopy));

    glBindVertexArray(gizmoVAO[0]);
        // draw box-dots y-axis
        {
        glm::vec3 dir = glm::vec3(0.0f, gizmoUpLinePoints[4], 0.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, dir);
        model = glm::scale(model, glm::vec3(0.03f));
        glUniformMatrix4fv(gizmoShader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(gizmoShader.GetUniformLocation("color"), 0.0f, 0.7f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        }   

        // draw box-dots x-axis
        {
        glm::vec3 dir = glm::vec3(gizmoUpLinePoints[4], 0.0f, 0.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, dir);
        model = glm::scale(model, glm::vec3(0.03f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(gizmoShader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(gizmoShader.GetUniformLocation("color"), 0.7f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        }   

        // draw box-dots z-axis
        {
        glm::vec3 dir = glm::vec3(0.0f, 0.0f, gizmoUpLinePoints[4]);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, dir);
        model = glm::scale(model, glm::vec3(0.03f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(gizmoShader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(gizmoShader.GetUniformLocation("color"), 0.0f, 0.0f, 0.7f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        }   
    
        // draw box
        {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.15f));
        glUniformMatrix4fv(gizmoShader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(gizmoShader.GetUniformLocation("color"), 0.7f, 0.7f, 0.7f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    glBindVertexArray(0);

    // draw lines
    glBindVertexArray(gizmoVAO[1]);
        // y-axis
        {
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(gizmoShader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(gizmoShader.GetUniformLocation("color"), 0.0f, 1.0f, 0.0f);
        glDrawArrays(GL_LINES, 0, 6);
        }

        // x-axis
        {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(gizmoShader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(gizmoShader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
        glDrawArrays(GL_LINES, 0, 6);
        }

        // z-axis
        {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(gizmoShader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(gizmoShader.GetUniformLocation("color"), 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_LINES, 0, 6);
        }
    glBindVertexArray(0);
}

glm::mat4 selfImplemented_lookAt(glm::vec3 pos, glm::vec3 targetPos, glm::vec3 up)
{
    // inverse because the scene will be affected, not actually camera
    glm::vec3 front = glm::normalize(pos - targetPos);
    glm::vec3 right = glm::normalize(glm::cross(up, front));
    glm::vec3 up_ = glm::cross(front, right);

    glm::mat4 translation = glm::mat4(1.0f);
    translation[3][0] = -pos.x;
    translation[3][1] = -pos.y;
    translation[3][2] = -pos.z;

    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0][0] = right.x;
    rotation[0][1] = up_.x;
    rotation[0][2] = front.x;
    
    rotation[1][0] = right.y;
    rotation[1][1] = up_.y;
    rotation[1][2] = front.y;

    rotation[2][0] = right.z;
    rotation[2][1] = up_.z;
    rotation[2][2] = front.z;

    return rotation * translation;
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

        glm::vec3 front = glm::vec3(
                std::cos(glm::radians(camYaw)) * std::cos(glm::radians(camPitch)),
                std::sin(glm::radians(camPitch)),
                std::sin(glm::radians(camYaw)) * std::cos(glm::radians(camPitch))
        );
        camFront = glm::normalize(front);
    }
}

void mouseScrollCB(GLFWwindow* window, double dx, double dy)
{
    if (!ImGui::IsAnyWindowHovered() && !ImGui::IsAnyItemHovered())
    {
        if (camFov >= 1.0f && camFov <= 45.0f)
            camFov -= dy;
        if (camFov < 1.0f)
            camFov = 1.0f;
        if (camFov > 45.0f)
            camFov = 45.0f;

        shader.Use();
        projection = glm::perspective(glm::radians(camFov), screenWidth * 1.0f / screenHeight, 0.1f, 100.0f);
        glUniformMatrix4fv(shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

        gizmoShader.Use();
        glm::mat4 projection = glm::perspective(glm::radians(camFov), 1.0f, 0.1f, 100.0f);
        glUniformMatrix4fv(gizmoShader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
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
    else if (!isGuiWindowShow && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        isGuiWindowShow = !isGuiWindowShow;
    }

    if (isLeftMousePressed && !ImGui::IsAnyWindowHovered() && !ImGui::IsAnyItemHovered())
    {
        const float kCamSpeed = 2.5f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camPos += kCamSpeed * camFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camPos -= kCamSpeed * camFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camPos += kCamSpeed * glm::normalize(glm::cross(camUp, camFront));
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)\
            camPos += kCamSpeed * glm::normalize(glm::cross(camFront, camUp));
    }
}

void initMem()
{
}

void destroyMem()
{
    glDeleteBuffers(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(2, gizmoVAO);
    glDeleteBuffers(2, gizmoVBO);
    shader.Destroy();
    gizmoShader.Destroy();
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
        renderGizmo();

        glfwSwapBuffers(window);
    }

    destroyMem();
    glfwTerminate();

    return 0;
}
