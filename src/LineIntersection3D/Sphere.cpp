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
    isShaderBuilt(false)
{
    assert(numStacks > 3);
    assert(numSectors > 3);
}

void Sphere::build()
{
    destroyVertexBuffersIfNeeded();
    destroyShaderIfNeeded();

    int result = shader.Build("data2/trans.vert", "data2/color.frag");
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
    assert(spec_vao != 0);

    glBindVertexArray(spec_vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Sphere::drawBatchBegin() const
{
    assert(spec_vao != 0);
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

    std::cout << "kPhiStepAngle: " << kPhiStepAngle << " rad | " << glm::degrees(kPhiStepAngle) << " degrees" << std::endl;
    std::cout << "kThetaStepAngle: " << kThetaStepAngle << " rad | " << glm::degrees(kThetaStepAngle) << " degrees" << std::endl;

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
    assert(vertices.size() > 0);
    assert(indices.size() > 0);

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
