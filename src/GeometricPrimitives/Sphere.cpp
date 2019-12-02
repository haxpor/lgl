#include "Sphere.h"
#include "lgl/Error.h"

#define DEFAULT_NUM_STACKS 20
#define DEFAULT_NUM_SECTORS 20
#define DEFAULT_RADIUS 1.0f

Sphere::Sphere(): Sphere(DEFAULT_NUM_STACKS, DEFAULT_NUM_SECTORS, DEFAULT_RADIUS)
{ }

Sphere::Sphere(unsigned int numStacks, unsigned int numSectors, float r):
    numStacks(numStacks),
    numSectors(numSectors),
    radius(r),
    color(glm::vec3(1.0f, 1.0f, 1.0f)),
    isShaderBuilt(false)
{
    assert(numStacks > 3 && "numStacks must be more than 3 to programmatically generate Sphere vertices");
    assert(numSectors > 3 && "numSectors must be more than 3 to programmatically generate Sphere vertices");
}

void Sphere::build()
{
    destroyVertexBuffersIfNeeded();
    destroyShaderIfNeeded();

    const char* vertexShaderStr = R"(#version 330 core
#extension GL_ARB_explicit_uniform_location : require
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

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
}
)";
    int result = shader.BuildFromSrc(vertexShaderStr, fragmentShaderStr);
    LGL_ERROR_QUIT(result, "Error creating shader");
    isShaderBuilt = true;

    buildVertexSpecifications();
    setupVertexBuffers();
}

void Sphere::destroyVertexBuffersIfNeeded()
{
    if (spec_vbo != 0)
    {
        glDeleteBuffers(1, &spec_vbo);
        spec_vbo = 0;
    }
    if (spec_ebo != 0)
    {
        glDeleteBuffers(1, &spec_ebo);
        spec_ebo = 0;
    }
    if (spec_vao != 0)
    {
        glDeleteBuffers(1, &spec_vao);
        spec_vao = 0;
    }
}

void Sphere::destroyShaderIfNeeded()
{
    if (isShaderBuilt)
    {
        shader.Destroy();
        isShaderBuilt = false;
    }
}

void Sphere::destroyGLObjects()
{
    destroyVertexBuffersIfNeeded();
    destroyShaderIfNeeded();
}

void Sphere::draw() const
{
    glBindVertexArray(spec_vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Sphere::drawBatchBegin() const
{
    glBindVertexArray(spec_vao);
}

void Sphere::drawBatchDraw() const
{
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void Sphere::drawBatchEnd() const
{
    glBindVertexArray(0);
}

void Sphere::buildVertexSpecifications()
{
    const float kPhiStepAngle = glm::radians(std::ceil(180.0f / numStacks));
    const float kThetaStepAngle = glm::radians(std::ceil(360.0f / numSectors));

    vertices.clear();
    indices.clear();

    vertices.reserve(numStacks * numSectors);
    indices.reserve(numStacks * numSectors * 6);

    // make vertices
    for (unsigned int sti=0; sti<=numStacks; ++sti)
    {
        for (unsigned int seci=0; seci<=numSectors; ++seci)
        {
            float x = radius * std::cos(glm::half_pi<float>() - (sti)*kPhiStepAngle) * std::sin(seci*kThetaStepAngle);
            float y = radius * std::sin(glm::half_pi<float>() - (sti)*kPhiStepAngle);
            float z = radius * std::cos(glm::half_pi<float>() - (sti)*kPhiStepAngle) * std::cos(seci*kThetaStepAngle);
            vertices.emplace_back(glm::vec3(x, y, z));
        }
    }

    // make indices
    for (unsigned int sti=0; sti<numStacks; ++sti)
    {
        unsigned int currLevelStacki = sti*(numSectors+1);
        unsigned int nextLevelStacki = (sti+1)*(numSectors+1);

        for (unsigned int seci=0; seci<numSectors; ++seci)
        {
            if (sti != 0)
            {
                indices.emplace_back(currLevelStacki + seci);
                indices.emplace_back(nextLevelStacki + seci);
                indices.emplace_back(currLevelStacki + seci + 1);
            }

            if (sti != numStacks - 1)
            {
                indices.emplace_back(currLevelStacki + seci + 1);
                indices.emplace_back(nextLevelStacki + seci);
                indices.emplace_back(nextLevelStacki + seci + 1);
            }
        }
    }
}

void Sphere::setupVertexBuffers()
{
    assert(vertices.size() > 0 && "vertices.size() must greater than 0. Call buildVertexSpecifications function first.");
    assert(indices.size() > 0 && "indices.size() must be greater than 0. Make sure buildVertexSpecifications function is called and indices was properly generated");

    glGenVertexArrays(1, &spec_vao);
    glBindVertexArray(spec_vao);
        glGenBuffers(1, &spec_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, spec_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(*vertices.data()) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &spec_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spec_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indices.data()) * indices.size(), indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void Sphere::updateProjectionMatrix(const glm::mat4& mat)
{
    glUniformMatrix4fv(shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(mat));
}

void Sphere::updateViewMatrix(const glm::mat4& mat)
{
    glUniformMatrix4fv(shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(mat));
}

void Sphere::updateModelMatrix(const glm::mat4& mat)
{
    glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(mat));
}

void Sphere::setColor(float r, float g, float b)
{
    glUniform3f(shader.GetUniformLocation("color"), r, g, b);
}
