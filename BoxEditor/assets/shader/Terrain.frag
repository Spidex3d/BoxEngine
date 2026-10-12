#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D texture_grass;
uniform sampler2D texture_rock;
uniform sampler2D texture_dirt;

void main()
{
    vec2 tiledUV =
        TexCoords * 32.0;

    vec3 grass =
        texture(
            texture_grass,
            tiledUV
        ).rgb;

    vec3 rock =
        texture(
            texture_rock,
            tiledUV
        ).rgb;

    vec3 dirt =
        texture(
            texture_dirt,
            tiledUV
        ).rgb;


    // -------------------------------------------------
    // Slope
    // -------------------------------------------------

    vec3 normal =
        normalize(Normal);

    float slope =
        1.0 -
        clamp(
            dot(
                normal,
                vec3(0.0, 1.0, 0.0)
            ),
            0.0,
            1.0
        );


    // -------------------------------------------------
    // Rock on steep slopes
    // -------------------------------------------------

    float rockBlend =
        smoothstep(
            0.30,
            0.65,
            slope
        );


    // -------------------------------------------------
    // Dirt based partly on height
    // -------------------------------------------------

    float height =
        FragPos.y;

    float dirtBlend =
        smoothstep(
            -2.0,
            4.0,
            height
        );

    dirtBlend *=
        1.0 - rockBlend;

    dirtBlend *= 0.35;
    //dirtBlend *= 1.0;


    // -------------------------------------------------
    // Grass fills remaining area
    // -------------------------------------------------

    float grassBlend =
        1.0 -
        rockBlend -
        dirtBlend;

    grassBlend =
        clamp(
            grassBlend,
            0.0,
            1.0
        );


    // -------------------------------------------------
    // Final blend
    // -------------------------------------------------

    vec3 terrainColor =
        grass * grassBlend +
        dirt  * dirtBlend +
        rock  * rockBlend;

    FragColor =
        vec4(
            terrainColor,
            1.0
        );
}


//#version 460 core
//
//in vec3 FragPos;
//in vec3 Normal;
//in vec2 TexCoords;
//
//out vec4 FragColor;
//
//uniform sampler2D texture_grass;
//uniform sampler2D texture_rock;
//uniform sampler2D texture_dirt;
//
//void main()
//{
//    vec2 tiledUV =
//        TexCoords * 32.0;
//
//    vec3 grass =
//        texture(
//            texture_grass,
//            tiledUV
//        ).rgb;
//
//    FragColor =
//        vec4(
//            grass,
//            1.0
//        );
//}
//
