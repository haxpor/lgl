/*
====================
Indexed drawing
====================
*/
#include "lgl/Base.h"

float vertices[] = {
     0.5f,  0.5f, 0.0f,      // top right
     0.5f, -0.5f, 0.0f,      // bottom right
    -0.5f, -0.5f, 0.0f,      // bottom left
    -0.5f,  0.5f, 0.0f       // top left
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
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &VS_CODE, NULL);
        glCompileShader(vertexShader);
        lgl::error::AnyGLShaderError(vertexShader);

        // compile fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &FS_CODE, NULL);
        glCompileShader(fragmentShader);
        lgl::error::AnyGLShaderError(fragmentShader);

        // link all shaders together
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        lgl::error::AnyGLShaderProgramError(shaderProgram);

        // delete un-needed shader objects
        // note: in fact, it will mark them for deletion after our usage of shader program is done
        // they will be deleted after that
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // wrap vertex attrib configurations via VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
            // prepare vertex data
            GLuint VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);  // or 3*sizeof(float) for its stride parameter
            glEnableVertexAttribArray(0);

            // prepare of EBO (Element Buffer Object) for indexed drawing
            GLuint EBO;
            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindVertexArray(0);
    }

    void UserRender() override {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    const char* VS_CODE = R"(#version 330 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
})";

    const char* FS_CODE = R"(#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
})";

    GLuint VAO;
    GLuint shaderProgram;
};

int main()
{
    Demo app;
    app.Setup("EBO");
    app.Start();
    return 0;
}
