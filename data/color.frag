#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) uniform vec4 tintColor;

out vec4 FragColor;
in vec3 ourColor;

void main()
{
    FragColor = vec4(ourColor, 1.0) * tintColor;
}
