#version 330 core

uniform sampler2D textureSampler;
uniform sampler2D textureSampler2;
uniform float mixFactor;

in vec2 vsTexCoord;

out vec4 fsColor;

void main()
{
    vec4 t1 = texture(textureSampler, vsTexCoord);
    vec4 t2 = texture(textureSampler2, vsTexCoord);

    fsColor = mix(t1, t2, mixFactor);
}
