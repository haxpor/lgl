/*
====================
Basic rendering triangle
====================
*/
#include "lgl/Base.h"

// define vertex data in NDC coordinate space
// before it gets transformed into screen-space coordinate (clip space) as input for fragment shader
float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

class Demo : public lgl::App
{
public:
    void UserSetup() override {
        // compile vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &VS_CODE, NULL);
        glCompileShader(vertexShader);
        lgl::error::PrintGLShaderErrorIfAny(vertexShader);

        // compile fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &FS_CODE, NULL);
        glCompileShader(fragmentShader);
        lgl::error::PrintGLShaderErrorIfAny(fragmentShader);

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
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);  // or 3*sizeof(float) for its stride parameter
            glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    }

    void UserRender() override {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
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
}
)";

    GLuint VBO;
    GLuint VAO;
    GLuint shaderProgram;
};

int main()
{
    Demo app;
    app.Setup("Hello Triangle");
    app.Start();
    return 0;
}
