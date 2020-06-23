#include "Gizmo.h"

static const float boxVertices[] = {
    // positions          // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

static const float gizmoUpLinePoints[] = {
    0.0f, 0.0f, 0.0f,
    0.0f, 0.30f, 0.0f
};

#define DEFAULT_VIEWPORT_WIDTH 100
#define DEFAULT_VIEWPORT_HEIGHT 100
#define DEFAULT_CAMFOV 45.0f // in degrees

Gizmo::Gizmo(): Gizmo(0, 0, DEFAULT_VIEWPORT_WIDTH, DEFAULT_VIEWPORT_HEIGHT)
{ }

Gizmo::Gizmo(int x, int y, int width, int height):
    viewMatrix(glm::mat4(1.0f)),
    projectionMatrix(glm::perspective(glm::radians(DEFAULT_CAMFOV), 1.0f, 0.1f, 100.0f))
{
    gizmoViewport[0] = x;
    gizmoViewport[1] = y;
    gizmoViewport[2] = width;
    gizmoViewport[3] = height;
}

void Gizmo::build()
{
    setupVertexBuffers();
}

void Gizmo::destroyGLObjects()
{
    shader.Destroy();
    glDeleteBuffers(2, vao);
    glDeleteBuffers(2, vbo);
}

void Gizmo::setupVertexBuffers()
{
    // create gizmo's box shader
    int result = shader.Build("data2/gizmo.vert", "data2/gizmo.frag");
    LGL_ERROR_QUIT(result, "Error creating gizmo shader");

    // vertex buffers for gizmo
    glGenVertexArrays(2, vao);
    glGenBuffers(2, vbo);

    glBindVertexArray(vao[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // lines
    glBindVertexArray(vao[1]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(gizmoUpLinePoints), gizmoUpLinePoints, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    shader.Use();
    glUniform3f(shader.GetUniformLocation("color"), 0.7f, 0.7f, 0.7);

    // create a fake view matrix (identity matrix) as we don't want to require user to send in
    // view matrix at this time. Also it is probably be updated soon at the first frame, or at the
    // initialization sequence at user's code.
    glm::mat4 viewCopy = glm::mat4(viewMatrix);
    viewCopy[3][0] = 0.0f;
    viewCopy[3][1] = 0.0f;
    viewCopy[3][2] = -1.0f;     // move the camera back slightly to see all angle of object
    glUniformMatrix4fv(shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(viewCopy));
    glUniformMatrix4fv(shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
}

void Gizmo::draw()
{
    // get current viewport area to set it back later when done drawing
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    // set viewport area
    glViewport(gizmoViewport[0], gizmoViewport[1], gizmoViewport[2], gizmoViewport[3]);
    glClear(GL_DEPTH_BUFFER_BIT);

    shader.Use();

    glm::mat4 viewCopy = glm::mat4(viewMatrix);
    viewCopy[3][0] = 0.0f;
    viewCopy[3][1] = 0.0f;
    viewCopy[3][2] = -1.0f;
    glUniformMatrix4fv(shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(viewCopy));

    glBindVertexArray(vao[0]);
        // draw box
        {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.15f));
        glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(shader.GetUniformLocation("color"), 0.7f, 0.7f, 0.7f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        }

    // draw lines
    glBindVertexArray(vao[1]);
        // y-axis
        {
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(shader.GetUniformLocation("color"), 0.0f, 1.0f, 0.0f);
        glDrawArrays(GL_LINES, 0, 6);
        }

        // x-axis
        {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(shader.GetUniformLocation("color"), 1.0f, 0.0f, 0.0f);
        glDrawArrays(GL_LINES, 0, 6);
        }

        // z-axis
        {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(shader.GetUniformLocation("color"), 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_LINES, 0, 6);
        }

    // draw boxes at the tips of the lines
    glBindVertexArray(vao[0]);
        // draw box-dots y-axis
        {
        glm::vec3 dir = glm::vec3(0.0f, gizmoUpLinePoints[4], 0.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, dir);
        model = glm::scale(model, glm::vec3(0.03f));
        glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(shader.GetUniformLocation("color"), 0.0f, 0.7f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        }   

        // draw box-dots x-axis
        {
        glm::vec3 dir = glm::vec3(gizmoUpLinePoints[4], 0.0f, 0.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, dir);
        model = glm::scale(model, glm::vec3(0.03f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(shader.GetUniformLocation("color"), 0.7f, 0.0f, 0.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        }   

        // draw box-dots z-axis
        {
        glm::vec3 dir = glm::vec3(0.0f, 0.0f, gizmoUpLinePoints[4]);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, dir);
        model = glm::scale(model, glm::vec3(0.03f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(shader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(shader.GetUniformLocation("color"), 0.0f, 0.0f, 0.7f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        }    
    glBindVertexArray(0);

    // set opengl setting back to what's before this function call
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

void Gizmo::updateViewMatrix(const glm::mat4& view)
{
    viewMatrix = view;

    // gizmo won't be affected by virtual camera's translation
    glm::mat4 viewCopy = glm::mat4(viewMatrix);
    viewCopy[3][0] = 0.0f;
    viewCopy[3][1] = 0.0f;
    viewCopy[3][2] = -1.0f; // move camera back slightly to see the gizmo itself

    shader.Use();
    glUniformMatrix4fv(shader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(viewCopy));
}

void Gizmo::updateProjectionMatrix(const glm::mat4& projection)
{
    projectionMatrix = projection;
    shader.Use();
    glUniformMatrix4fv(shader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
}
