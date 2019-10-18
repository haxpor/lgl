#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 vsTexCoord;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vsTexCoord = aTexCoord;
}
