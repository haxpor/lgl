/**
 * Render a triangle using index buffer.
 *
 * Compile with
 *  
 *  Linux:
 *   g++ -g -Wall -Wextra -pedantic -fno-exceptions -Wno-stringop-overflow -Wno-unused-parameter -std=c++11 \
 *       -Iexternals/glad/include \
 *       -Iexternals/stb_image \
 *       -Iincludes \
 *       src/lgl/*.cpp \
 *       externals/glad/src/glad.c \
 *       <this-source-file> \
 *       -lglut -lGL -lX11 -lpthread -lm -ldl
 */
#include "lgl/lgl.h"
#include <GL/freeglut.h>
#include <iostream>
#include <cstdlib>

int screenWidth = 800;
int screenHeight = 600;

////////////////////////
/// glut callback
////////////////////////
int initGLUT(int argc, char** argv);
void initGL();

void displayCB();
void timerCB(int ms);
void reshapeCB(int w, int h);
void keyboardCB(unsigned char key, int x, int y);
void mouseCB(int button, int state, int x, int y);
void motionCB(int x, int y);
void initMem();
void destroyMem();

////////////////////////
/// global variables
////////////////////////
GLuint shaderProgram;
GLuint vao;
GLuint vbo;
GLuint ebo;

float vertices[] =
{
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

unsigned int indices[] =
{
    0, 1, 2,
    0, 2, 3
};

////////////////////////
// implementations
////////////////////////
int initGLUT(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(screenWidth, screenHeight);

    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    int handle = glutCreateWindow(argv[0]);

    if (!gladLoadGL())
        lgl::error::ErrorExit("Failed to initalize GLAD");

    glutDisplayFunc(displayCB);
    glutTimerFunc(17, timerCB, 17);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(motionCB);
    glutReshapeFunc(reshapeCB);

    return handle;
}

void initGL()
{
    GLint majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    std::cout << "OpenGL version in use: " << majorVersion << "." << minorVersion << std::endl;

    glEnable(GL_DEPTH_TEST);

    const char* VS_CODE = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

    const char* FS_CODE = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

    // compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VS_CODE, nullptr);
    glCompileShader(vertexShader);
    if (lgl::error::AnyGLShaderError(vertexShader) != LGL_SUCCESS)
        glutLeaveMainLoop();

    // compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &FS_CODE, nullptr);
    glCompileShader(fragmentShader);
    if (lgl::error::AnyGLShaderError(fragmentShader) != LGL_SUCCESS)
        glutLeaveMainLoop();

    // link all shaders together
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    if (lgl::error::AnyGLShaderProgramError(shaderProgram) != LGL_SUCCESS)
        glutLeaveMainLoop();

    // delete shader objects after we're done with them
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void displayCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glUseProgram(shaderProgram);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glutSwapBuffers();
}

void timerCB(int ms)
{
    glutTimerFunc(ms, timerCB, ms);
    glutPostRedisplay();
}

void reshapeCB(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    
    glViewport(0, 0, static_cast<GLsizei>(screenWidth), static_cast<GLsizei>(screenHeight));
}

void motionCB(int x, int y)
{
}

void mouseCB(int button, int state, int x, int y)
{
}

void keyboardCB(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 27:    // escape key
            glutLeaveMainLoop();
            break;
    }
}

void initMem()
{
}

void destroyMem()
{
    glDeleteBuffers(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shaderProgram);
}

int main(int argc, char** argv)
{
    initMem();

    std::atexit(destroyMem);

    initGLUT(argc, argv);
    initGL();

    glutMainLoop();

    return 0;
}
