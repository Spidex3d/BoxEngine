#version 460 core

layout(location = 0)
in vec3 aPosition;

layout(location = 1)
in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 WorldPos;
out vec2 TexCoords;

void main()
{
    vec4 worldPosition =
        model * vec4(
            aPosition,
            1.0
        );

    WorldPos =
        worldPosition.xyz;

    TexCoords =
        aTexCoords;

    gl_Position =
        projection *
        view *
        worldPosition;
}


//#version 460 core
//
//layout(location = 0)
//in vec3 aPosition;
//
//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;
//
//out vec3 WorldPos;
//
//void main()
//{
//    vec4 worldPosition =
//        model * vec4(aPosition, 1.0);
//
//    WorldPos =
//        worldPosition.xyz;
//
//    gl_Position =
//        projection *
//        view *
//        worldPosition;
//}
