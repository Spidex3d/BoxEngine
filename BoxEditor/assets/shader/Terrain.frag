#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D texture_grass;
uniform sampler2D texture_rock;
uniform sampler2D texture_dirt;

uniform float textureTiling;
uniform float rockSlopeStart;
uniform float rockSlopeEnd;
uniform float dirtHeightStart;
uniform float dirtHeightEnd;
uniform float dirtAmount;

void main()
{

    vec2 tiledUV = TexCoords * textureTiling;

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
            rockSlopeStart,
            rockSlopeEnd,
            slope);


    // -------------------------------------------------
    // Dirt based partly on height
    // -------------------------------------------------
    float height = FragPos.y;

    float dirtBlend =
    1.0 - smoothstep(
        dirtHeightStart,
        dirtHeightEnd,
        height);

    dirtBlend *= 1.0 - rockBlend;

    dirtBlend *= dirtAmount;

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
