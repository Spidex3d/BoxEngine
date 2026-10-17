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

uniform sampler2D uSceneDepth;

uniform float uNearPlane;
uniform float uFarPlane;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;

    return (2.0 * uNearPlane * uFarPlane) / (
            uFarPlane + uNearPlane - z * (uFarPlane - uNearPlane));
}

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
    // Convert texture values from 0..1 to -1..1
    // -------------------------------------------------

    normal1 =
        normal1 * 2.0 - 1.0;

    normal2 =
        normal2 * 2.0 - 1.0;


    // -------------------------------------------------
    // Combine the two moving normal maps
    // -------------------------------------------------

    vec3 mapNormal =
        normalize(
            normal1 +
            normal2
        );


    // -------------------------------------------------
    // Convert normal map into Y-up water normal
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


    // =================================================
    // WATER DEPTH
    // =================================================

    vec2 screenUV =
        gl_FragCoord.xy /
        vec2(
            textureSize(
                uSceneDepth,
                0
            )
        );


    float sceneDepthRaw =
        texture(
            uSceneDepth,
            screenUV
        ).r;


    float rawDepthDifference =
        max(
            sceneDepthRaw -
            gl_FragCoord.z,
            0.0
        );


    // -------------------------------------------------
    // Convert tiny OpenGL depth difference into
    // a useful 0..1 blend value.
    // -------------------------------------------------

//    float depthBlend =
//        clamp(
//            rawDepthDifference *
//            1000.0,
//            0.0,
//            1.0
//        );

        float depthBlend =
        clamp(
            rawDepthDifference *
            500.0,
            0.0,
            1.0
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


    // =================================================
// DEPTH BASED WATER COLOUR
// =================================================

vec3 shallowWater =
    vec3(
        0.05,
        0.38,
        0.42
    );

vec3 deepWater =
    vec3(
        0.01,
        0.055,
        0.16
    );

vec3 waterColor =
    mix(
        shallowWater,
        deepWater,
        depthBlend
    );

//// =================================================
//// SHORELINE FOAM
//// =================================================
//
//float foam =
//    1.0 -
//    smoothstep(
//        0.0,
//        0.18,
//        depthBlend
//    );
//    // 18
//
//// -------------------------------------------------
//// Irregular moving foam pattern
//// -------------------------------------------------
//
//vec2 foamUV =
//    WorldPos.xz * 0.08 +
//    vec2(
//        uTime * 0.015,
//        uTime * 0.010
//    );
//
//float foamNoise =
//    texture(
//        uNormalMap,
//        foamUV
//    ).r;
//
//foam *=
//    smoothstep(
//        0.25,
//        0.60,
//        foamNoise
//    );
//
//foam *= 1.5;
//
//
//// -------------------------------------------------
//// Foam colour
//// -------------------------------------------------
//
//vec3 foamColor =
//    vec3(
//        1.0,
//        1.0,
//        1.0
//    );
//
//waterColor =
//    mix(
//        waterColor,
//        foamColor,
//        foam
//    );
//

// =================================================
// SHORELINE FOAM
// =================================================

// Convert terrain depth and water depth
// into proper distance values.

float sceneLinearDepth =
    LinearizeDepth(
        sceneDepthRaw
    );

float waterLinearDepth =
    LinearizeDepth(
        gl_FragCoord.z
    );


// Distance between water surface
// and terrain behind it.

float foamDepth =
    max(
        sceneLinearDepth -
        waterLinearDepth,
        0.0
    );


// -------------------------------------------------
// Foam only very close to shoreline.
// -------------------------------------------------

float foam =
    1.0 -
    smoothstep(
        0.0,
        1.2,
        foamDepth
    );


// -------------------------------------------------
// Break the foam up using water normal texture.
// -------------------------------------------------

vec2 foamUV =
    WorldPos.xz * 0.08 +
    vec2(
        uTime * 0.015,
        uTime * 0.010
    );

float foamNoise =
    texture(
        uNormalMap,
        foamUV
    ).r;


foam *=
    smoothstep(
        0.25,
        0.60,
        foamNoise
    );


// -------------------------------------------------
// Foam strength
// -------------------------------------------------

foam *= 1.25;

foam =
    clamp(
        foam,
        0.0,
        1.0
    );


// -------------------------------------------------
// Foam colour
// -------------------------------------------------

vec3 foamColor =
    vec3(
        1.0,
        1.0,
        1.0
    );


waterColor =
    mix(
        waterColor,
        foamColor,
        foam
    );


    // -------------------------------------------------
    // Add a little surface lighting
    // -------------------------------------------------

    waterColor *=
        mix(
            0.80,
            1.15,
            diffuse
        );


    // -------------------------------------------------
    // Specular highlight
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

        // =================================================
        // FRESNEL
        // =================================================

        float fresnel =
            1.0 -
            max(
                dot(
                    normal,
                    viewDir
                ),
                0.0
            );
        
        fresnel =
            pow(
                fresnel,
                3.0
            );
        

        // =================================================
        // DEPTH TRANSPARENCY
        // =================================================
        
        // Shallow water is clear.
        // Deep water becomes much more opaque.
        
        float depthAlpha =
            mix(
                0.25,
                0.92,
                depthBlend
            );


        // Looking across the surface adds a little
        // more opacity from the Fresnel effect.
        
        float fresnelAlpha =
            fresnel * 0.18;
        
        
        // Final alpha
        
        float alpha =
            clamp(
                depthAlpha +
                fresnelAlpha,
                0.0,
                0.95
            );
        

    // =================================================
    // FINAL OUTPUT
    // =================================================

    FragColor =
    vec4(
        waterColor,
        alpha
    );
}



