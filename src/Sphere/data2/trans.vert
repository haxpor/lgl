#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) in vec3 aPos;

// projection * view
uniform mat4 pv;
uniform mat4 model;

void main()
{
    gl_Position = pv * model * vec4(aPos, 1.0);
}
