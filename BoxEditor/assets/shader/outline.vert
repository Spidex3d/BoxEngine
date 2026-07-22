#version 460 core

layout(location = 0) in vec3 aPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float outlineScale;

void main()
{
    vec3 expandedPosition =
        aPosition * outlineScale;

    gl_Position =
        projection *
        view *
        model *
        vec4(expandedPosition, 1.0);
}