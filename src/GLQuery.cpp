/*
====================
Demonstrate a simple query information from OpenGL
====================
*/
#include "Base.h"

int main(int argc, char* argv[])
{
    lgl::App app;
    app.Setup("OpenGL Query");

    // make a query
    // note: if make any query, do so during the time of Setup() or right after but before
    // it goes into loop as whenever it goes out of the loop, things get terminated, thus you will
    // get segmentfault.
    GLint numResult;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numResult);
    std::cout << "Number of attributes: " << numResult << "\n";
    
    app.Start();

    return 0;
}
