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

    // -------------------------------------------------
    // Final colour
    // -------------------------------------------------

    FragColor = vec4(waterColor, alpha);
}



//#version 460 core
//
//in vec3 WorldPos;
//in vec2 TexCoords;
//
//out vec4 FragColor;
//
//uniform float uTime;
//uniform float uWaveSpeed;
//uniform float uWaveStrength;
//
//uniform vec3 uCameraPos;
//uniform vec3 uLightDirection;
//
//uniform float uSpecularPower;
//
//uniform sampler2D uNormalMap;
//
//
//void main()
//{
//    // -------------------------------------------------
//    // Time
//    // -------------------------------------------------
//
//    float t =
//        uTime * uWaveSpeed;
//
//
//    // -------------------------------------------------
//    // Tile the normal map
//    // -------------------------------------------------
//
//    vec2 tiledUV =
//        TexCoords * 24.0;
//
//
//    // -------------------------------------------------
//    // First moving normal layer
//    // -------------------------------------------------
//
//    vec2 uv1 =
//        tiledUV +
//        vec2(
//            t * 0.025,
//            t * 0.015
//        );
//
//
//    // -------------------------------------------------
//    // Second moving normal layer
//    // -------------------------------------------------
//
//    vec2 uv2 =
//        tiledUV +
//        vec2(
//            -t * 0.018,
//             t * 0.022
//        );
//
//
//    // -------------------------------------------------
//    // Sample normal map twice
//    // -------------------------------------------------
//
//    vec3 normal1 =
//        texture(
//            uNormalMap,
//            uv1
//        ).rgb;
//
//    vec3 normal2 =
//        texture(
//            uNormalMap,
//            uv2
//        ).rgb;
//
//
//    // -------------------------------------------------
//    // Convert 0..1 texture values to -1..1
//    // -------------------------------------------------
//
//    normal1 =
//        normal1 * 2.0 - 1.0;
//
//    normal2 =
//        normal2 * 2.0 - 1.0;
//
//
//    // -------------------------------------------------
//    // Combine normal maps
//    // -------------------------------------------------
//
//    vec3 mapNormal =
//        normalize(
//            normal1 +
//            normal2
//        );
//
//
//    // -------------------------------------------------
//    // Convert normal map to Y-up water surface
//    // -------------------------------------------------
//
//    vec3 normal =
//        normalize(
//            vec3(
//                mapNormal.x *
//                uWaveStrength,
//
//                1.0,
//
//                mapNormal.y *
//                uWaveStrength
//            )
//        );
//
//
//    // -------------------------------------------------
//    // Light direction
//    // -------------------------------------------------
//
//    vec3 lightDir =
//        normalize(
//            -uLightDirection
//        );
//
//
//    // -------------------------------------------------
//    // Camera direction
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
//    // Diffuse lighting
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
//    // Specular lighting
//    // -------------------------------------------------
//
//    vec3 halfwayDir =
//        normalize(
//            lightDir +
//            viewDir
//        );
//
//
//    float specular =
//        pow(
//            max(
//                dot(
//                    normal,
//                    halfwayDir
//                ),
//                0.0
//            ),
//            uSpecularPower
//        );
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
//    // Specular highlight
//    // -------------------------------------------------
//
//    vec3 specularColor =
//        vec3(
//            0.85,
//            0.95,
//            1.0
//        );
//
//
//    waterColor +=
//        specularColor *
//        specular *
//        0.75;
//
//
//    // =================================================
//    // FRESNEL TRANSPARENCY
//    // =================================================
//
//    float fresnel =
//        1.0 -
//        max(
//            dot(
//                normal,
//                viewDir
//            ),
//            0.0
//        );
//
//
//    // Shape the effect.
//    // Higher value = more transparent when looking down.
//    fresnel =
//        pow(
//            fresnel,
//            3.0
//        );
//
//
//    // -------------------------------------------------
//    // Alpha range
//    //
//    // Looking down       -> more transparent
//    // Looking across     -> more opaque
//    // -------------------------------------------------
//
//    float alpha =
//        mix(
//            0.35,
//            0.80,
//            fresnel
//        );
//
//
//    // -------------------------------------------------
//    // Final colour
//    // -------------------------------------------------
//
//    FragColor =
//        vec4(
//            waterColor,
//            alpha
//        );
//}
//