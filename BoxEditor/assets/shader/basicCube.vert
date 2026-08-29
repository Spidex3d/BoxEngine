#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in int aMaterialIndex;
layout(location = 4) in vec3 aTangent;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vWorldPosition;
out vec3 vNormal;
out vec2 vTexCoord;

out vec3 vTangent;
out vec3 vBitangent;

flat out int vMaterialIndex;

void main()
{
    vec4 worldPosition =
        uModel *
        vec4(aPosition, 1.0);

    vWorldPosition =
        worldPosition.xyz;

    mat3 normalMatrix =
        mat3(
            transpose(
                inverse(uModel)
            )
        );

    vNormal =
        normalize(
            normalMatrix * aNormal
        );

    vTangent =
        normalize(
            normalMatrix * aTangent
        );

    vTangent =
        normalize(
            vTangent -
            dot(vTangent, vNormal) *
            vNormal
        );

    vBitangent =
        normalize(
            cross(
                vNormal,
                vTangent
            )
        );

    vTexCoord =
        aTexCoord;

    vMaterialIndex =
        aMaterialIndex;

    gl_Position =
        uProjection *
        uView *
        worldPosition;
}

//layout(location = 0) in vec3 aPosition;
//layout(location = 1) in vec3 aNormal;
//layout(location = 2) in vec2 aTexCoord;
//layout(location = 3) in int aMaterialIndex;
//layout(location = 4) in vec3 aTangent;
//
//uniform mat4 uModel;
//uniform mat4 uView;
//uniform mat4 uProjection;
//
//out vec3 vWorldPosition;
//out vec3 vNormal;
//out vec2 vTexCoord;
//out vec3 vTangent;
//out vec3 vBitangent;
//
//flat out int vMaterialIndex;
//
//void main()
//{
//    vec4 worldPosition =
//        uModel *
//        vec4(aPosition, 1.0);
//
//    vWorldPosition =
//        worldPosition.xyz;
//
//    vNormal =
//        mat3(
//            transpose(
//                inverse(uModel)
//            )
//        ) * aNormal;
//
//    vTexCoord =  aTexCoord;
//
//    vMaterialIndex = aMaterialIndex;
//
//    gl_Position =
//        uProjection *
//        uView *
//        worldPosition;
//}
