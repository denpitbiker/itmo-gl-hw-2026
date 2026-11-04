#version 330 core

in vec2 worldPosition;
out vec4 out_col;

void main()
{
    float radius = clamp(length(worldPosition), 0.0, 1.0);
    vec3 color = mix(vec3(0.85, 0.97, 1.0), vec3(0.1, 0.5, 0.9), radius);
    out_col = vec4(color, 1.0);
}
