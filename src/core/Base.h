#ifndef _BASE_H_
#define _BASE_H_

#include <iostream>
#include "Wrapped_GL.h"
#include "Util.h"
#include "Error.h"
#include "Shader.h"
#include "External.h"
#include <GLFW/glfw3.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

namespace lgl
{
    /*
    ====================
    App instance
    ====================
    */
    class App
    {
    public:
        int Setup(const char* title);
        void Start();

        // ------------ user-callback ---------------- //
        virtual void UserFramebufferSizeCallback(const int width, const int height);
        virtual void UserProcessKeyInput();
        virtual void UserSetup();
        virtual void UserUpdate(const double delta);    // delta in seconds
        virtual void UserRender();
        virtual void UserShutdown();
        // ------------ end of user-callback --------- //

    private:
        struct TmpHolder
        {
            GLFWwindow* window;
        };

        // need to be static to satisfy the requirement of GLFW's function to set this function pointer
        static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
        void ProcessKeyInput(GLFWwindow* window);

        TmpHolder holder;
        double prevTicks;
    };
};

inline int lgl::App::Setup(const char* title)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title, nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to crate GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
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

inline void lgl::App::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    lgl::App *app = static_cast<lgl::App*>(glfwGetWindowUserPointer(window));
    app->UserFramebufferSizeCallback(width, height);
}

inline void lgl::App::ProcessKeyInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    UserProcessKeyInput();
}

inline void lgl::App::Start()
{
    prevTicks = 0.0;

    while (!glfwWindowShouldClose(holder.window))
    {
        // get window
        GLFWwindow* window = holder.window;

        glfwPollEvents();

        // update
        ProcessKeyInput(window);

        double delta = glfwGetTime() - prevTicks; 
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
inline void lgl::App::UserUpdate(const double delta) { }
inline void lgl::App::UserRender()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}
inline void lgl::App::UserShutdown() { }

inline void lgl::App::UserProcessKeyInput() { }
inline void lgl::App::UserFramebufferSizeCallback(int width, int height) { }

#endif
