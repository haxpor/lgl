#ifndef GIZMO_H_
#define GIZMO_H_

#include "lgl/Wrapped_GL.h"
#include "lgl/Shader.h"
#include "glm/mat4x4.hpp"

/// Gizmo
/// Depends on GLSL shader files of gizmo.vert and gizmo.frag. Make sure they are exist in data2/
/// directory.
class Gizmo
{
public:

    /// Create gizmo and draw it at the default viewport area at the lower left of the screen.
    /// Default viewport area is 0,0 for x,y and 100,100 for width,height.
    Gizmo();

    /// Specified viewport area to draw gizmo on screen
    Gizmo(int x, int y, int width, int height);

    /// Build vertex buffers and internal data structures.
    /// Call this after OpenGL context has been created only.
    void build();

    /// destroy opengl related objects
    void destroyGLObjects();

    /// update view matrix to internally and to OpenGL's GLSL uniform variables
    /// Input view matrix is main virtual camera of the program, not for gizmo itself.
    /// This function will process input matrix to make it suitable for gizmo automatically.
    void updateViewMatrix(const glm::mat4& view);

    void draw();

    inline const GLint* getViewport() const { return &gizmoViewport[0]; }

private:
    GLuint vao[2];
    GLuint vbo[2];
    lgl::Shader shader;
    GLint prevViewport[4];
    GLint gizmoViewport[4];

    /// a copy of view matrix sent in from externally.
    /// to be usable for Gizmo, it still need to modify translation components to not move when
    /// external main camera moves.
    ///
    /// having a cache copy of view matrix will help reducing the time users need to call update
    /// from externally unnecessary
    glm::mat4 viewMatrix;

    glm::mat4 projectionMatrix;

    void setupVertexBuffers();

    /// update projection matrix to internally and to OpenGL's GLSL uniform variables
    void updateProjectionMatrix(const glm::mat4& projection);
};

#endif
