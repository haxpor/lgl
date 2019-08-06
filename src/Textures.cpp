/*
====================
Based on Shader2_with_FileReader.cpp
with texture for primitive
====================
*/
#include "Base.h"
#include <cmath>
#include <cstdlib>

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
        // create shader program and build it immediately
        int result = basicShader.Build("../data/basic.vert", "../data/basic.frag");
        if (result != 0) { exit(1); }

        // wrap vertex attrib configurations via VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
            // prepare vertex data
            GLuint VBO;
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
            GLuint EBO;
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

        basicShader.Use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        if (!isFirstFrameWaitDone)
        {
            isFirstFrameWaitDone = true;
        }
    }

private:
    bool isFirstFrameWaitDone;
    lgl::Shader basicShader;
    GLuint VAO;
};

int main(int argc, char* argv[])
{
    Demo app;
    app.Setup("Indexed drawing");
    app.Start();
    return 0;
}
