#version 460 core

in vec3 WorldPos;

out vec4 FragColor;

uniform float uTime;
uniform float uWaveSpeed;
uniform float uWaveStrength;

void main()
{
    // -------------------------------------------------
    // Moving wave pattern
    // -------------------------------------------------

    float t =
        uTime * uWaveSpeed;

    float waveX =
        sin(WorldPos.x * 0.45 + t);

    float waveZ =
        sin(WorldPos.z * 0.55 - t * 0.8);

    float waveDiag =
        sin(
            (WorldPos.x + WorldPos.z) * 0.30 +
            t * 0.6
        );


    // -------------------------------------------------
    // Fake surface normal
    // -------------------------------------------------

    vec3 normal =
        normalize(
            vec3(
                waveX * uWaveStrength,
                1.0,
                waveZ * uWaveStrength
            )
        );


    // Add a little diagonal disturbance.
    normal.x +=
        waveDiag *
        uWaveStrength *
        0.35;

    normal.z -=
        waveDiag *
        uWaveStrength *
        0.35;

    normal =
        normalize(normal);


    // -------------------------------------------------
    // Simple overhead light
    // -------------------------------------------------

    vec3 lightDirection =
        normalize(
            vec3(
                -0.3,
                1.0,
                0.2
            )
        );

    float lightAmount =
        max(
            dot(
                normal,
                lightDirection
            ),
            0.0
        );


    // -------------------------------------------------
    // Water colour
    // -------------------------------------------------

    vec3 deepWater =
        vec3(
            0.02,
            0.20,
            0.36
        );

    vec3 lightWater =
        vec3(
            0.05,
            0.40,
            0.58
        );


    vec3 waterColor =
        mix(
            deepWater,
            lightWater,
            lightAmount
        );


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
//out vec4 FragColor;
//
//uniform float uTime;
//uniform float uWaveSpeed;
//uniform float uWaveStrength;
//
//void main()
//{
//    // -------------------------------------------------
//    // Two waves travelling in different directions
//    // -------------------------------------------------
//
//    float wave1 =
//    sin(
//        WorldPos.x * 0.55 +
//        uTime * uWaveSpeed
//    );
//
//float wave2 =
//    sin(
//        WorldPos.z * 0.70 -
//        uTime * uWaveSpeed * 0.75
//    );
//
//float wave3 =
//    sin(
//        (WorldPos.x + WorldPos.z) * 0.40 +
//        uTime * uWaveSpeed * 0.45
//    );
//
//
//
//
//    float waves =
//        (wave1 + wave2 + wave3) /
//        3.0;
//
//
//    // -------------------------------------------------
//    // Water colours
//    // -------------------------------------------------
//
//    vec3 darkWater =
//    vec3(
//        0.03,
//        0.28,
//        0.48
//    );
//
//vec3 lightWater =
//    vec3(
//        0.08,
//        0.40,
//        0.58
//    );
//
//
//
//    // Convert -1..1 into 0..1.
//    float waveAmount =
//        waves * 0.5 + 0.5;
//
//
//    // Wave strength controls how noticeable
//    // the colour variation is.
//    waveAmount =
//        mix(
//            0.5,
//            waveAmount,
//            uWaveStrength
//        );
//
//
//    vec3 waterColor =
//        mix(
//            darkWater,
//            lightWater,
//            waveAmount
//        );
//
//
//    FragColor =
//        vec4(
//            waterColor,
//            0.72
//        );
//}
//
