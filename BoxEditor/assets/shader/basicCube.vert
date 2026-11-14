
#version 460 core

// =============================================
// EXISTING MESH ATTRIBUTES
// =============================================

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in int  aMaterialIndex;
layout(location = 4) in vec3 aTangent;

// =============================================
// NEW: INSTANCE MATRIX ATTRIBUTES
// =============================================

// Each mat4 occupies four attribute locations.
layout(location = 5) in vec4 aInstanceCol0;
layout(location = 6) in vec4 aInstanceCol1;
layout(location = 7) in vec4 aInstanceCol2;
layout(location = 8) in vec4 aInstanceCol3;

// =============================================
// UNIFORMS
// =============================================

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// 0 = ordinary scene entity
// 1 = GPU-instanced mesh
uniform int uUseInstancing;

// =============================================
// OUTPUTS TO FRAGMENT SHADER
// =============================================

out vec3 vWorldPosition;
out vec3 vNormal;
out vec2 vTexCoord;

out vec3 vTangent;
out vec3 vBitangent;

flat out int vMaterialIndex;

// =============================================
// MAIN
// =============================================

void main()
{
    // -----------------------------------------
    // Choose our model matrix.
    // -----------------------------------------

    mat4 modelMatrix = uModel;

    if (uUseInstancing != 0)
    {
        modelMatrix = mat4(
            aInstanceCol0,
            aInstanceCol1,
            aInstanceCol2,
            aInstanceCol3
        );
    }

    // -----------------------------------------
    // World position
    // -----------------------------------------

    vec4 worldPosition =
        modelMatrix * vec4(aPosition, 1.0);

    vWorldPosition = worldPosition.xyz;

    // -----------------------------------------
    // Normal matrix
    // -----------------------------------------

    mat3 normalMatrix =
        transpose(inverse(mat3(modelMatrix)));

    vNormal = normalize(
        normalMatrix * aNormal
    );

    // -----------------------------------------
    // Tangent space
    // -----------------------------------------

    vTangent = normalize(
        normalMatrix * aTangent
    );

    vTangent = normalize(
        vTangent -
        dot(vTangent, vNormal) * vNormal
    );

    vBitangent = normalize(
        cross(vNormal, vTangent)
    );

    // -----------------------------------------
    // UV and material slot
    // -----------------------------------------

    vTexCoord = aTexCoord;

    vMaterialIndex = aMaterialIndex;

    // -----------------------------------------
    // Final clip-space position
    // -----------------------------------------

    gl_Position =
        uProjection *
        uView *
        worldPosition;
}



//#version 460 core
//
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
//
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
//    mat3 normalMatrix =
//        mat3(
//            transpose(
//                inverse(uModel)
//            )
//        );
//
//    vNormal =
//        normalize(
//            normalMatrix * aNormal
//        );
//
//    vTangent =
//        normalize(
//            normalMatrix * aTangent
//        );
//
//    vTangent =
//        normalize(
//            vTangent -
//            dot(vTangent, vNormal) *
//            vNormal
//        );
//
//    vBitangent =
//        normalize(
//            cross(
//                vNormal,
//                vTangent
//            )
//        );
//
//    vTexCoord =
//        aTexCoord;
//
//    vMaterialIndex =
//        aMaterialIndex;
//
//    gl_Position =
//        uProjection *
//        uView *
//        worldPosition;
//}
//