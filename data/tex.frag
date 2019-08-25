#version 330 core

uniform sampler2D textureSampler;
uniform vec4 tintColor;

in vec3 vsColor;
in vec2 vsTexCoord;

out vec4 fsColor;

void main()
{
    fsColor = texture(textureSampler, vsTexCoord) * vec4(vsColor, 1.0) * tintColor;
}
