#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) uniform sampler2D textureSampler;

in vec3 vsColor;
in vec2 vsTexCoord;

out vec4 fsColor;

void main()
{
    fsColor = texture(textureSampler, vsTexCoord);
}
