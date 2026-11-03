#version 460 core

// ============================================
// Grass Blade Vertex Shader
// ============================================

// Vertex attributes
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 aInstanceModel;

// Transformation matrices
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
// false = existing single-blade rendering
// true  = GPU instancing
uniform bool uUseInstancing = false;

// Output to fragment shader
out vec3 vNormal;
out vec3 vWorldPosition;
out vec2 vTexCoord;

void main()
{
    // ========================================
    // CHOOSE MODEL MATRIX
    // ========================================

    mat4 finalModel = uModel;

    if (uUseInstancing)
    {
        finalModel =
            uModel * aInstanceModel;
    }

    // ========================================
    // WORLD POSITION
    // ========================================

    vec4 worldPosition =
        finalModel *
        vec4(aPosition, 1.0);

    vWorldPosition =
        worldPosition.xyz;

    // ========================================
    // NORMAL
    // ========================================

    mat3 normalMatrix =
        transpose(
            inverse(
                mat3(finalModel)
            )
        );

    vNormal =
        normalize(
            normalMatrix * aNormal
        );

    // ========================================
    // UV
    // ========================================

    vTexCoord = aTexCoord;

    // ========================================
    // FINAL POSITION
    // ========================================

    gl_Position =
        uProjection *
        uView *
        worldPosition;
}


//void main()
//{
//    // ----------------------------------------
//    // Transform vertex into world space
//    // ----------------------------------------
//
//    vec4 worldPosition =
//        uModel * vec4(aPosition, 1.0);
//
//    vWorldPosition = worldPosition.xyz;
//
//    // ----------------------------------------
//    // Transform normal
//    // ----------------------------------------
//
//    mat3 normalMatrix =
//        transpose(inverse(mat3(uModel)));
//
//    vNormal =
//        normalize(normalMatrix * aNormal);
//
//    // ----------------------------------------
//    // Pass UV coordinates
//    // ----------------------------------------
//
//    vTexCoord = aTexCoord;
//
//    // ----------------------------------------
//    // Final clip-space position
//    // ----------------------------------------
//
//    gl_Position = uProjection * uView * worldPosition;
//}