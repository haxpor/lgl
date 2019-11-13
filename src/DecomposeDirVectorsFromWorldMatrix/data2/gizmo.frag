#version 330 core

uniform vec3 color;

out vec4 fsColor;

void main()
{
    fsColor = vec4(color, 1.0); 
}

