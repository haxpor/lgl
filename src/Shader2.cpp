/*
====================
More vertex attribute
====================
*/
#include "Base.h"
#include <cmath>

float vertices[] = {
    // positions        // colors
     0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,   // top right
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom left
    -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f    // top left
};

unsigned int indices[] = {
    0, 1, 2,                // first triangle (right)
    0, 2, 3                 // second triangle (left)
};

class Demo : public lgl::App
{
public:
    void UserSetup() override {
        // compile vertex shader
        GL_uint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &VS_CODE, NULL);
        glCompileShader(vertexShader);
        lgl::util::PrintGLShaderErrorIfAny(vertexShader);

        // compile fragment shader
        GL_uint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &FS_CODE, NULL);
        glCompileShader(fragmentShader);
        lgl::util::PrintGLShaderErrorIfAny(fragmentShader);

        // link all shaders together
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        lgl::util::PrintGLShaderProgramErrorIfAny(shaderProgram);

        // delete un-needed shader objects
        // note: in fact, it will mark them for deletion after our usage of shader program is done
        // they will be deleted after that
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // wrap vertex attrib configurations via VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
            // prepare vertex data
            GL_uint VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            // positions
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));  // or 3*sizeof(float) for its stride parameter
            glEnableVertexAttribArray(0);
            // colors
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));  // start after 3 floats
            glEnableVertexAttribArray(1);

            // prepare of EBO (Element Buffer Object) for indexed drawing
            GL_uint EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindVertexArray(0);

        isFirstFrameWaitDone = false;
    }

    void UserUpdate(const double delta) override {
        if (isFirstFrameWaitDone)
        {
            double currentTicks = glfwGetTime();
            double greenValue = (std::sin(currentTicks) / 2.0f) + 0.5f;

            // skip calling to glUseProgram(shaderProgram) as we wait for 1 frame
            // shaderProgram by now is set to be active, so we save subsequent call from now on
            glUniform4f(0, 0.0f, greenValue, 0.0f, 1.0f);
        }
    }

    void UserRender() override {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        if (!isFirstFrameWaitDone)
        {
            isFirstFrameWaitDone = true;
        }
    }

private:
    const char* VS_CODE = R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    ourColor = aColor;
})";

    const char* FS_CODE = R"(#version 330 core
in vec3 ourColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)";

    bool isFirstFrameWaitDone;
    GL_uint VAO;
    GL_uint shaderProgram;
};

int main(int argc, char* argv[])
{
    Demo app;
    app.Setup("Indexed drawing");
    app.Start();
    return 0;
}
