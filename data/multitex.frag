#version 330 core
#extension GL_ARB_explicit_uniform_location : require

layout (location = 0) uniform sampler2D textureSampler;
layout (location = 1) uniform sampler2D textureSampler2;

in vec2 vsTexCoord;

out vec4 fsColor;

void main()
{
    vec4 t1 = texture(textureSampler, vsTexCoord);
    vec4 t2 = texture(textureSampler2, vsTexCoord);

    fsColor = mix(t1, t2, 0.2);
}
