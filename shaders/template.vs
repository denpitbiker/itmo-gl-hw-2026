#version 330 core

layout(location = 0) in vec2 pos;
uniform vec2 center;
uniform vec2 halfExtent;
out vec2 worldPosition;

void main()
{
    worldPosition = pos;
    gl_Position = vec4((pos - center) / halfExtent, 0.0, 1.0);
}
