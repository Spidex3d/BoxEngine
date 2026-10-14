#version 460 core

in vec3 WorldPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform float uTime;
uniform float uWaveSpeed;
uniform float uWaveStrength;

uniform vec3 uCameraPos;
uniform vec3 uLightDirection;

uniform float uSpecularPower;

uniform sampler2D uNormalMap;


void main()
{
    // -------------------------------------------------
    // Time
    // -------------------------------------------------

    float t =
        uTime * uWaveSpeed;


    // -------------------------------------------------
    // Tile the normal map
    // -------------------------------------------------

    vec2 tiledUV =
        TexCoords * 24.0;


    // -------------------------------------------------
    // First moving normal layer
    // -------------------------------------------------

    vec2 uv1 =
        tiledUV +
        vec2(
            t * 0.025,
            t * 0.015
        );


    // -------------------------------------------------
    // Second moving normal layer
    // -------------------------------------------------

    vec2 uv2 =
        tiledUV +
        vec2(
            -t * 0.018,
             t * 0.022
        );


    // -------------------------------------------------
    // Sample normal map twice
    // -------------------------------------------------

    vec3 normal1 =
        texture(
            uNormalMap,
            uv1
        ).rgb;

    vec3 normal2 =
        texture(
            uNormalMap,
            uv2
        ).rgb;


    // -------------------------------------------------
    // Convert 0..1 texture values to -1..1
    // -------------------------------------------------

    normal1 =
        normal1 * 2.0 - 1.0;

    normal2 =
        normal2 * 2.0 - 1.0;


    // -------------------------------------------------
    // Combine normal maps
    // -------------------------------------------------

    vec3 mapNormal =
        normalize(
            normal1 +
            normal2
        );


    // -------------------------------------------------
    // Convert normal map to our Y-up water surface
    // -------------------------------------------------

    vec3 normal =
        normalize(
            vec3(
                mapNormal.x *
                uWaveStrength,

                1.0,

                mapNormal.y *
                uWaveStrength
            )
        );


    // -------------------------------------------------
    // Light direction
    // -------------------------------------------------

    vec3 lightDir =
        normalize(
            -uLightDirection
        );


    // -------------------------------------------------
    // Camera direction
    // -------------------------------------------------

    vec3 viewDir =
        normalize(
            uCameraPos -
            WorldPos
        );


    // -------------------------------------------------
    // Diffuse lighting
    // -------------------------------------------------

    float diffuse =
        max(
            dot(
                normal,
                lightDir
            ),
            0.0
        );


    // -------------------------------------------------
    // Specular lighting
    // -------------------------------------------------

    vec3 halfwayDir =
        normalize(
            lightDir +
            viewDir
        );


    float specular =
        pow(
            max(
                dot(
                    normal,
                    halfwayDir
                ),
                0.0
            ),
            uSpecularPower
        );


    // -------------------------------------------------
    // Water colours
    // -------------------------------------------------

    vec3 deepWater =
        vec3(
            0.02,
            0.18,
            0.32
        );

    vec3 surfaceWater =
        vec3(
            0.04,
            0.34,
            0.52
        );


    vec3 waterColor =
        mix(
            deepWater,
            surfaceWater,
            diffuse
        );


    // -------------------------------------------------
    // Specular highlight colour
    // -------------------------------------------------

    vec3 specularColor =
        vec3(
            0.85,
            0.95,
            1.0
        );


    waterColor +=
        specularColor *
        specular *
        0.75;


    // -------------------------------------------------
    // Final colour
    // -------------------------------------------------

    FragColor =
        vec4(
            waterColor,
            0.72
        );
}

//#version 460 core
//
//in vec3 WorldPos;
//
//in vec2 TexCoords;
//out vec4 FragColor;
//
//uniform float uTime;
//uniform float uWaveSpeed;
//uniform float uWaveStrength;
//
//uniform vec3 uCameraPos;
//uniform vec3 uLightDirection;
//uniform float uSpecularPower;
//
//uniform sampler2D uNormalMap;
//
//void main()
//{
//    // -------------------------------------------------
//    // Animated waves
//    // -------------------------------------------------
//
//    // -------------------------------------------------
//    // Moving normal maps
//    // -------------------------------------------------
//
//float t =
//    uTime * uWaveSpeed;
//
//
//// Repeat the normal map many times across
//// the large water surface.
//vec2 tiledUV =
//    TexCoords * 24.0;
//
//
//// First layer moves one direction.
//vec2 uv1 =
//    tiledUV +
//    vec2(
//        t * 0.025,
//        t * 0.015
//    );
//
//
//// Second layer moves another direction.
//vec2 uv2 =
//    tiledUV +
//    vec2(
//        -t * 0.018,
//         t * 0.022
//    );
//
//
//// Sample normal map twice.
//vec3 normal1 =
//    texture(
//        uNormalMap,
//        uv1
//    ).rgb;
//
//vec3 normal2 =
//    texture(
//        uNormalMap,
//        uv2
//    ).rgb;
//
//
//// Convert normal map:
////
//// 0..1
////
//// into
////
//// -1..1
////
//normal1 =
//    normal1 * 2.0 - 1.0;
//
//normal2 =
//    normal2 * 2.0 - 1.0;
//
//
//// Combine both layers.
//vec3 mapNormal =
//    normalize(
//        normal1 +
//        normal2
//    );
//
//    // -------------------------------------------------
//    // Fake moving water normal
//    // -------------------------------------------------
//
//    float irregularX =
//    waveX +
//    waveDiag * 0.35 +
//    wave4 * 0.25 +
//    wave5 * 0.15;
//
//float irregularZ =
//    waveZ -
//    waveDiag * 0.35 +
//    wave4 * 0.20 -
//    wave5 * 0.20;
//
//
//vec3 normal =
//    normalize(
//        vec3(
//            irregularX * uWaveStrength,
//            1.0,
//            irregularZ * uWaveStrength
//        )
//    );
//
//
//
//    // -------------------------------------------------
//    // Light direction
//    //
//    // uLightDirection represents the direction
//    // the light is travelling, so reverse it here.
//    // -------------------------------------------------
//
//    vec3 lightDir =
//        normalize(
//            -uLightDirection
//        );
//
//
//    // -------------------------------------------------
//    // View direction
//    // -------------------------------------------------
//
//    vec3 viewDir =
//        normalize(
//            uCameraPos -
//            WorldPos
//        );
//
//
//    // -------------------------------------------------
//    // Basic water lighting
//    // -------------------------------------------------
//
//    float diffuse =
//        max(
//            dot(
//                normal,
//                lightDir
//            ),
//            0.0
//        );
//
//
//    // -------------------------------------------------
//    // Specular highlight
//    //
//    // Blinn-Phong style highlight
//    // -------------------------------------------------
//
//    vec3 halfwayDir =
//        normalize(
//            lightDir +
//            viewDir
//        );
//
//    float specular = pow(max(dot(normal, halfwayDir), 0.0), uSpecularPower);
//
//
//    // -------------------------------------------------
//    // Water colours
//    // -------------------------------------------------
//
//    vec3 deepWater =
//        vec3(
//            0.02,
//            0.18,
//            0.32
//        );
//
//    vec3 surfaceWater =
//        vec3(
//            0.04,
//            0.34,
//            0.52
//        );
//
//
//    vec3 waterColor =
//        mix(
//            deepWater,
//            surfaceWater,
//            diffuse
//        );
//
//
//    // -------------------------------------------------
//    // Bright reflection from the light
//    // -------------------------------------------------
//
//    vec3 specularColor =
//        vec3(
//            0.85,
//            0.95,
//            1.0
//        );
//
//    waterColor +=
//        specularColor *
//        specular *
//        0.75;
//
//
//    FragColor =
//        vec4(
//            waterColor,
//            0.72
//        );
//}
//
