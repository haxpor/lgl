/*
====================
Adapted from Textures.cpp demo but with sample texture.
====================
*/
#include "lgl/Base.h"
#include <cmath>
#include <cstdlib>

using namespace glm;

float vertices[] = {
    // positions        // texture coords
     0.5f,  0.5f, 0.0f, 1.0f, 1.0f,               // top right
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,               // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,               // bottom left
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f                // top left
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
        basicShader.SetUniform(basicShader.GetUniformLocation("texturSampler"), 0);
        glActiveTexture(GL_TEXTURE1);
        basicShader.SetUniform(basicShader.GetUniformLocation("textureSampler2"), 1);
        // set default uniform values
        basicShader.SetUniform(basicShader.GetUniformLocation("mixFactor"), mixFactor);

        mat4 trans(1.0f);
        // this will scale first, then rotate
        trans = rotate(trans, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
        trans = scale(trans, vec3(0.5f, 0.5f, 0.5f));

        glUniformMatrix4fv(basicShader.GetUniformLocation("transform"), 1, GL_FALSE, value_ptr(trans));
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
            basicShader.SetUniform(basicShader.GetUniformLocation("mixFactor"), mixFactor);
        }
        // blend more color from container
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            mixFactor -= 0.01f;
            if (mixFactor < 0.0f)
            {
                mixFactor = 0.0f;
            }
            basicShader.SetUniform(basicShader.GetUniformLocation("mixFactor"), mixFactor);
        }
    }

    void UserShutdown() override {
        // disable vertex attributes
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
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
    app.Setup("Transformations");
    app.Start();
    return 0;
}
