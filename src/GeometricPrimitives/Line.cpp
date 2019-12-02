#include "Line.h"

void Line::initialInitialize(const LineData& ldata)
{
    spec_vao = 0;
    spec_vbo = 0;
    isShaderBuilt = false;
    lineColor = glm::vec3(1.0f, 1.0f, 1.0f);
    t = 1.0f;
    lineData = ldata;
    lineDataDraw[0] = lineData.pos;
    lineDataDraw[1] = lineData.pos + t*lineData.dir;
}

void Line::initialInitialize(const LineData&& ldata)
{
    spec_vao = 0;
    spec_vbo = 0;
    isShaderBuilt = false;
    lineColor = glm::vec3(1.0f, 1.0f, 1.0f);
    t = 1.0f;
    lineData = ldata;
    lineDataDraw[0] = lineData.pos;
    lineDataDraw[1] = lineData.pos + t*lineData.dir;
}

void Line::destroyVertexBuffersIfNeeded()
{
    if (spec_vbo != 0)
    {
        glDeleteBuffers(1, &spec_vbo);
        spec_vbo = 0;
    }
    if (spec_vao != 0)
    {
        glDeleteBuffers(1, &spec_vao);
        spec_vao = 0;
    }
}

void Line::destroyShaderIfNeeded()
{
    if (isShaderBuilt)
    {
        shader.Destroy();
        isShaderBuilt = false;
    }
}

void Line::build()
{
    destroyVertexBuffersIfNeeded();
    destroyShaderIfNeeded();

    const char* vertexShaderStr = R"(#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
})";

    const char* fragmentShaderStr = R"(#version 330 core
uniform vec3 color;
out vec4 fsColor;

void main()
{
    fsColor = vec4(color, 1.0); 
})";

    int result = shader.BuildFromSrc(vertexShaderStr, fragmentShaderStr);
    LGL_ERROR_QUIT(result, "Error creating shader");
    isShaderBuilt = true;

    glGenVertexArrays(1, &spec_vao);
    glGenBuffers(1, &spec_vbo);

    computeLineDataDraw();
    std::cout << glm::to_string(lineDataDraw[0]) << std::endl;
    std::cout << glm::to_string(lineDataDraw[1]) << std::endl;

    shader.Use();
    
    glBindVertexArray(spec_vao);
        glBindBuffer(GL_ARRAY_BUFFER, spec_vbo);
        glUniform3f(shader.GetUniformLocation("color"), lineColor.x, lineColor.y, lineColor.z);

        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 2, lineDataDraw, GL_STREAM_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Line::destroyGLObjects()
{
    destroyVertexBuffersIfNeeded();
    destroyShaderIfNeeded();
}

void Line::draw() const
{
    shader.Use();
    glBindVertexArray(spec_vao);
        glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
}

void Line::drawBatchBegin() const
{
    shader.Use();
    glBindVertexArray(spec_vao);
}

void Line::drawBatchDraw() const
{
    glDrawArrays(GL_LINES, 0, 2);
}

void Line::drawBatchEnd() const
{
    glBindVertexArray(0); 
}

void Line::computeLineDataDraw()
{
    lineDataDraw[0] = lineData.pos;
    lineDataDraw[1] = lineData.pos + t*lineData.dir;
}

void Line::updateProjectionMatrix(const glm::mat4& mat)
{
    glUniformMatrix4fv(shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(mat));
}

void Line::updateViewMatrix(const glm::mat4& mat)
{
    glUniformMatrix4fv(shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(mat));
}

void Line::updateModelMatrix(const glm::mat4& mat)
{
    glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(mat));
}
