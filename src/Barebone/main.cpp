/**
 * First initiative to implement the self-contained testbed using freeglut3.
 *
 * This is by design to mostly have things in flat level for ease of implementation, and more focus
 * on topic of testbed not about code structure and management.
 *
 * This is a barebone version of application to just show window and read some data from inputs e.g.
 * mouse and keyboard.
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
}

void displayCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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
    std::cout << "mouse is at " << x << ", " << y << std::endl;
}

void mouseCB(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
            std::cout << "Left mouse clicked" << std::endl;
    }
    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
            std::cout << "Right mouse clicked" << std::endl;
    }
    else if (button == GLUT_MIDDLE_BUTTON)
    {
        if (state == GLUT_DOWN)
            std::cout << "Middle mouse clicked" << std::endl;
    }
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
    std::cout << "Init mem" << std::endl;
}

void destroyMem()
{
    std::cout << "Destroy mem" << std::endl;
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
