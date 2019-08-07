/*
====================
Based on Shader2_with_FileReader.cpp
with texture for primitive

Changes
- updated to use with proper shader sources (now tex.vert/frag)
- fully and properly clean up memory used by this program, see UserShutdown()
====================
*/
#include "Base.h"
#include <cmath>
#include <cstdlib>

float vertices[] = {
    // positions        // colors         // texture coords
     0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,               // top right
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,               // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,               // bottom left
    -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f                // top left
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
        int result = basicShader.Build("../data/tex.vert", "../data/tex.frag");
        LGL_ERROR_QUIT(result, "Error creating basic shader");

        // load texture
        int width, height, nrChannels;
        unsigned char *data = stbi_load("../data/container.jpg", &width, &height, &nrChannels, 0);
        if (data)
        {
            glGenTextures(1, &containerTexture);
            glBindTexture(GL_TEXTURE_2D, containerTexture);

            // set texture filtering on currently bound texture object
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);
        }
        else
        {
            lgl::error::ErrorWarn("Error loading ../data/container.jpg [%s]", stbi_failure_reason());
        }

        // wrap vertex attrib configurations via VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
            // prepare vertex data
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            // positions
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0));  // or 3*sizeof(float) for its stride parameter
            glEnableVertexAttribArray(0);
            // colors
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));  // start after 3 floats
            glEnableVertexAttribArray(1);
            // texture coords
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));  // start after 6 floats
            glEnableVertexAttribArray(2);

            // bind texture
            glBindTexture(GL_TEXTURE_2D, containerTexture);

            // prepare of EBO (Element Buffer Object) for indexed drawing
            EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindVertexArray(0);

        isFirstFrameWaitDone = false;
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
    GLuint containerTexture;
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
