#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vWorldPosition;
out vec3 vNormal;
out vec2 vTexCoord;

void main()
{
    vec4 worldPosition =
        uModel *
        vec4(aPosition, 1.0);

    vWorldPosition =
        worldPosition.xyz;

    vNormal =
        mat3(
            transpose(
                inverse(uModel)
            )
        ) * aNormal;

    // This line is essential.
    vTexCoord =
        aTexCoord;

    gl_Position =
        uProjection *
        uView *
        worldPosition;
}


//layout(location = 0) in vec3 aPosition;
//layout(location = 1) in vec3 aNormal;
//layout(location = 2) in vec2 aTexCoord;
//
//uniform mat4 uModel;
//uniform mat4 uView;
//uniform mat4 uProjection;
//
//out vec2 vTexCoord;
//out vec3 vNormal;
//out vec3 vWorldPosition;
//
//void main()
//{
//
//    //vTexCoord = aTexCoord;
//   
//
//    vec4 worldPosition = uModel * vec4(aPosition, 1.0);
//
//    vWorldPosition = worldPosition.xyz;
//
//    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
//
//    gl_Position = uProjection * uView * worldPosition;
//}