#ifndef _BASE_H_
#define _BASE_H_

#include <iostream>
#include "lgl/Wrapped_GL.h"
#include "lgl/Util.h"
#include "lgl/Error.h"
#include "lgl/Shader.h"
#include <GLFW/glfw3.h>

// include the most frequently used at this level
#define LGL_EXTERNAL_GLM_INCLUDE
#include "lgl/External.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

namespace lgl
{
    // configurations used to setup and initialize the App 
    struct AppConfigs
    {
        bool MousePosEnabled;
        bool RelativeMouseCursor;
        bool MouseScrollEnabled;

        AppConfigs(): MousePosEnabled(false),
                      RelativeMouseCursor(false),
                      MouseScrollEnabled(false)
        { }
    };

    /*
    ====================
    App instance
    ====================
    */
    class App
    {
    public:
        int Setup(const char* title, const lgl::AppConfigs& configs=lgl::AppConfigs());
        void Start();
        GLFWwindow* GetGLFWWindow() const;

        // ------------ user-callback ---------------- //
        virtual void UserFramebufferSizeCallback(const int width, const int height);
        virtual void UserProcessKeyInput(double deltaTime);
        virtual void UserProcessMousePos(float xOffset, float yOffset);
        virtual void UserProcessMouseScrollInput(float xOffset, float yOffset);
        virtual void UserSetup();
        virtual void UserUpdate(double delta);    // delta in seconds
        virtual void UserRender();
        virtual void UserShutdown();
        // ------------ end of user-callback --------- //

    private:
        struct TmpHolder
        {
            GLFWwindow* window;
        };

        // defined within the context of class here to allow
        // static functions to access App's private member variables
        static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
        static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
        static void MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

        // mouse control
        float lastMouseX;
        float lastMouseY;

        void ProcessKeyInput(GLFWwindow* window, double deltaTime);

        TmpHolder holder;
        double prevTicks;
    };
}

inline GLFWwindow* lgl::App::GetGLFWWindow() const
{
    return holder.window;
}

inline int lgl::App::Setup(const char* title, const lgl::AppConfigs& configs)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title, nullptr, nullptr);
    if (window == nullptr)
    {
        lgl::error::ErrorWarn("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    if (configs.MousePosEnabled)
    {
        lastMouseX = SCREEN_WIDTH / 2.0f;
        lastMouseY = SCREEN_HEIGHT / 2.0f;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, lgl::App::FramebufferSizeCallback);

    if (configs.MousePosEnabled)
    {
        glfwSetCursorPosCallback(window, lgl::App::MouseCallback);
        if (configs.RelativeMouseCursor)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (configs.MouseScrollEnabled)
        glfwSetScrollCallback(window, lgl::App::MouseScrollCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        lgl::error::ErrorWarn("Failed to initalize GLAD");
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, 800, 600);

    glfwSetWindowUserPointer(window, this);

    // save to single instance
    holder.window = window;

    UserSetup();

    return 0;
}

inline void lgl::App::MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    lgl::App *app = static_cast<lgl::App*>(glfwGetWindowUserPointer(window));

    // prevent first frame of sudden jump of camera
    static bool firstMouse = true;
    if (firstMouse)
    {
        app->lastMouseX = xpos;
        app->lastMouseY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - app->lastMouseX;
    float yOffset = app->lastMouseY - ypos;
    app->lastMouseX = xpos;
    app->lastMouseY = ypos;

    static float kSensitivity = 0.05f;
    xOffset *= kSensitivity;
    yOffset *= kSensitivity;

    app->UserProcessMousePos(xOffset, yOffset);
}

inline void lgl::App::MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    lgl::App *app = static_cast<lgl::App*>(glfwGetWindowUserPointer(window));
    app->UserProcessMouseScrollInput(xOffset, yOffset);
}

inline void lgl::App::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    lgl::App *app = static_cast<lgl::App*>(glfwGetWindowUserPointer(window));
    app->UserFramebufferSizeCallback(width, height);
}

inline void lgl::App::ProcessKeyInput(GLFWwindow* window, double deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    UserProcessKeyInput(deltaTime);
}

inline void lgl::App::Start()
{
    prevTicks = 0.0;

    while (!glfwWindowShouldClose(holder.window))
    {
        // get window
        GLFWwindow* window = holder.window;

        glfwPollEvents();

        double delta = glfwGetTime() - prevTicks; 
        // update input
        ProcessKeyInput(window, delta);
        // update main loop
        UserUpdate(delta);
        prevTicks = glfwGetTime();

        // render
        UserRender();
        // not that useful, but should give visual feedback to user

        glfwSwapBuffers(window);
    }

    UserShutdown();
    glfwTerminate();
}

inline void lgl::App::UserSetup() { }
inline void lgl::App::UserUpdate(double) { }
inline void lgl::App::UserRender()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}
inline void lgl::App::UserShutdown() { }

inline void lgl::App::UserProcessKeyInput(double) { }
inline void lgl::App::UserProcessMousePos(float, float) { }
inline void lgl::App::UserProcessMouseScrollInput(float, float) { }
inline void lgl::App::UserFramebufferSizeCallback(int, int) { }

#endif
