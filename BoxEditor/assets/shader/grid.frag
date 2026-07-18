#version 460 core

out vec4 FragColor;

uniform vec3 uGridColor;

void main()
{
    FragColor = vec4(uGridColor, 1.0);
}