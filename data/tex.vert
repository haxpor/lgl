#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 vsTexCoord;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vsTexCoord = aTexCoord;
}
