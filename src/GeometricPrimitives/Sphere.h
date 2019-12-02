#ifndef LGL_SPHERE_H
#define LGL_SPHERE_H

#include <vector>
#include "lgl/Shader.h"
#include "lgl/Wrapped_GL.h"
#include "glm/vec3.hpp"

/// Sphere
/// Provides vertices + indices specification aimed for rendering sphere quickly in code.
/// It self-manage VBO + EBO, and provide rendering function.
///
/// User can create Sphere with different level of detail (via numStacks, numSectors and radius) and its
/// model, view, projection matrix as well as color accordingly (look at trans.vert, and color.frag) can
/// be configured via its shader. User has to maintain these information externally, Sphere class
/// doesn't maintain them.
///
/// Provides with batch draw call
/// (technically this is to bind its own vertex array object at begin, then reset when done)
///     - drawBatchBegin()
///     - drawBatchDraw()
///     - drawBatchEnd()
class Sphere
{
public:
    lgl::Shader shader;

    Sphere();
    Sphere(unsigned int numStacks, unsigned int numSectors, float r);

    void build();
    void destroyGLObjects();
    void draw() const;
    void drawBatchBegin() const;
    void drawBatchDraw() const;
    void drawBatchEnd() const;

    inline const std::vector<glm::vec3>& getVertices() const { return vertices; }
    inline const std::vector<unsigned int>& getIndices() const { return indices; }
    inline float getRadius() const { return radius; }

    /// Required: attached shader needs to call Shader::Use() before setting any of the following
    /// functions.
    void updateProjectionMatrix(const glm::mat4& mat);
    void updateViewMatrix(const glm::mat4& mat);
    void updateModelMatrix(const glm::mat4& mat);

    /// Required: attached shader needs to call Shader::Use()
    void setColor(float r, float g, float b);
    const glm::vec3& getColor() const;

private:
    unsigned int numStacks;
    unsigned int numSectors;
    float radius;

    bool isShaderBuilt;
    GLuint spec_vao;
    GLuint spec_vbo;
    GLuint spec_ebo;

    glm::vec3 color;

    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    void buildVertexSpecifications();
    void setupVertexBuffers();
    void destroyVertexBuffersIfNeeded();
    void destroyShaderIfNeeded();
};

/// inline implementations
inline const glm::vec3& Sphere::getColor() const
{
    return color;
}

#endif
