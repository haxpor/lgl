/*
====================
Based on Shader2_with_FileReader.cpp
with texture for primitive

Changes
- updated to use with proper shader sources (now tex.vert/frag)
- fully and properly clean up memory used by this program, see UserShutdown()
- mix color between two textures in GLSL, set its mixFactor via uniform, texture filtering config
====================
*/
#include "lgl/Base.h"
#include <cmath>
#include <cstdlib>

float vertices[] = {
    // positions        // texture coords
     0.5f,  0.5f, 0.0f, 2.0f, 2.0f,               // top right
     0.5f, -0.5f, 0.0f, 2.0f, 0.0f,               // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,               // bottom left
    -0.5f,  0.5f, 0.0f, 0.0f, 2.0f                // top left
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
        int result = basicShader.Build("data/tex.vert", "data/multitex.frag");
        LGL_ERROR_QUIT(result, "Error creating basic shader");

        // load texture
        containerTexture = lgl::util::LoadTexture("data/container.jpg");
        if (containerTexture == LGL_FAIL) { lgl::error::ErrorExit("Error loading data/container.jpg"); }
        // modify its texture filtering
        glBindTexture(GL_TEXTURE_2D, containerTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        awesomefaceTexture = lgl::util::LoadTexture("data/awesomeface.png");
        if (awesomefaceTexture == LGL_FAIL) { lgl::error::ErrorExit("Error loading data/awesomeface.png"); }

        // wrap vertex attrib configurations via VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
            // prepare vertex data
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            // positions
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));  // or 3*sizeof(float) for its stride parameter
            glEnableVertexAttribArray(0);
            // texture coords
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));  // start after 3 floats
            glEnableVertexAttribArray(1);

            // bind texture
            glBindTexture(GL_TEXTURE_2D, containerTexture);

            // prepare of EBO (Element Buffer Object) for indexed drawing
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindVertexArray(0);

        // once preparation
        // tell opengl which texture sampler map to whichs texture object
        basicShader.Use();
        glActiveTexture(GL_TEXTURE0);
        basicShader.SetUniform(0, 0);
        glActiveTexture(GL_TEXTURE1);
        basicShader.SetUniform(1, 1);
        // set default uniform values
        basicShader.SetUniform(2, mixFactor);
    }

    void UserProcessKeyInput() override {
        // poll for key input
        // blend more color from awesomeface
        GLFWwindow* window = GetGLFWWindow();
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            mixFactor += 0.01f;
            if (mixFactor > 1.0f)
            {
                mixFactor = 1.0f;
            }
            basicShader.SetUniform(2, mixFactor);
        }
        // blend more color from container
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            mixFactor -= 0.01f;
            if (mixFactor < 0.0f)
            {
                mixFactor = 0.0f;
            }
            basicShader.SetUniform(2, mixFactor);
        }
    }

    void UserShutdown() override {
        // disable vertex attributes
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        // delete all VBOs
        glDeleteBuffers(1, &VBO);
        VBO = -1;
        // delete all EBO (index buffer)
        glDeleteBuffers(1, &EBO);
        EBO = -1;
        // reset texture binding to default texture
        glBindTexture(GL_TEXTURE_2D, 0);
        // delete all textures
        glDeleteTextures(1, &containerTexture);
        containerTexture = -1;
        // delete shader program
        basicShader.Destroy();
        // delete all VAOs
        glDeleteVertexArrays(1, &VAO);
        VAO = -1;
    }

    void UserRender() override {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        basicShader.Use();
        // bind texture 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, containerTexture);
        // bind texture 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, awesomefaceTexture);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    lgl::Shader basicShader;
    GLuint containerTexture, awesomefaceTexture;
    GLfloat mixFactor = 0.2f;
    GLuint EBO;
    GLuint VBO;
    GLuint VAO;
};

int main(int argc, char* argv[])
{
    Demo app;
    app.Setup("Textures");
    app.Start();
    return 0;
}
