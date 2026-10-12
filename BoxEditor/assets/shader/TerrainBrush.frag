#version 460 core

out vec4 FragColor;

uniform vec3 brushColor;

void main()
{
    FragColor = vec4(brushColor, 1.0);
}