#version 330 core

uniform vec3 color;

out vec4 fragColor;

void main()
{
    vec2 cxy = 2.0 * gl_PointCoord - 1.0;
    float r = dot(cxy, cxy);
    float delta = fwidth(r);
    float alpha = 1.0 - smoothstep(1.0 - delta, 1.0 + delta, r);
    fragColor = vec4(color, alpha);
}
