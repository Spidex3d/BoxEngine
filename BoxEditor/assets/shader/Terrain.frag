#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;


// =====================================================
// TERRAIN TEXTURES
// =====================================================

uniform sampler2D texture_grass;
uniform sampler2D texture_rock;
uniform sampler2D texture_dirt;

// Rock normal map
uniform sampler2D normal_rock;


// =====================================================
// TERRAIN SETTINGS
// =====================================================

uniform float textureTiling;

uniform float rockSlopeStart;
uniform float rockSlopeEnd;

uniform float dirtHeightStart;
uniform float dirtHeightEnd;

uniform float dirtAmount;

uniform float rockNormalStrength;


// =====================================================
// MAIN
// =====================================================

void main()
{
    // -------------------------------------------------
    // Texture coordinates
    // -------------------------------------------------

    vec2 tiledUV =
        TexCoords *
        textureTiling;


    // -------------------------------------------------
    // Sample colour textures
    // -------------------------------------------------

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


    // =================================================
    // TERRAIN NORMAL
    // =================================================

    vec3 terrainNormal =
        normalize(Normal);


    // =================================================
    // SLOPE
    // =================================================

    float slope =
        1.0 -
        clamp(
            dot(
                terrainNormal,
                vec3(
                    0.0,
                    1.0,
                    0.0
                )
            ),
            0.0,
            1.0
        );


    // =================================================
    // ROCK BLEND
    // =================================================

    float rockBlend =
        smoothstep(
            rockSlopeStart,
            rockSlopeEnd,
            slope
        );


    // =================================================
    // DIRT BLEND
    // =================================================

    float height =
        FragPos.y;


    float dirtBlend =
        1.0 -
        smoothstep(
            dirtHeightStart,
            dirtHeightEnd,
            height
        );


    // Don't put dirt over rock.
    dirtBlend *=
        1.0 -
        rockBlend;


    dirtBlend *=
        dirtAmount;


    // =================================================
    // GRASS BLEND
    // =================================================

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


    // =================================================
    // ROCK NORMAL MAP
    // =================================================

    vec3 rockNormal =
        texture(
            normal_rock,
            tiledUV
        ).rgb;


    // Convert normal texture from 0..1 to -1..1.
    rockNormal =
        rockNormal *
        2.0 -
        1.0;


    // Amount of small rock detail.
    //float rockNormalStrength = 0.35;


    rockNormal.x *=
        rockNormalStrength;

    rockNormal.y *=
        rockNormalStrength;


    rockNormal =
        normalize(
            rockNormal
        );


    // =================================================
    // BUILD A BASIS AROUND THE TERRAIN NORMAL
    // =================================================

    vec3 helperAxis =
        abs(terrainNormal.y) < 0.999
        ? vec3(0.0, 1.0, 0.0)
        : vec3(1.0, 0.0, 0.0);


    vec3 tangent =
        normalize(
            cross(
                helperAxis,
                terrainNormal
            )
        );


    vec3 bitangent =
        normalize(
            cross(
                terrainNormal,
                tangent
            )
        );


    mat3 TBN =
        mat3(
            tangent,
            bitangent,
            terrainNormal
        );


    vec3 detailedRockNormal =
        normalize(
            TBN *
            rockNormal
        );


    // =================================================
    // ROCK DETAIL LIGHTING
    //
    // Only a subtle brightness variation.
    // We keep your original terrain colour brightness.
    // =================================================

    vec3 detailLightDirection =
        normalize(
            vec3(
                -0.6,
                 1.0,
                -0.4
            )
        );


    float rockLight =
        max(
            dot(
                detailedRockNormal,
                detailLightDirection
            ),
            0.0
        );


    // Keep this close to 1.0 so it does not make
    // the terrain dark.
    float rockDetailBrightness =
        mix(
            0.85,
            1.15,
            rockLight
        );


    // =================================================
    // ORIGINAL TERRAIN COLOUR BLENDING
    // =================================================

    vec3 terrainColor =
        grass * grassBlend +
        dirt  * dirtBlend +
        rock  * rockBlend;


    // -------------------------------------------------
    // Apply normal-map detail ONLY to rock
    // -------------------------------------------------

    terrainColor *=
        mix(
            1.0,
            rockDetailBrightness,
            rockBlend
        );


    // =================================================
    // FINAL OUTPUT
    // =================================================

    FragColor =
        vec4(
            terrainColor,
            1.0
        );
}

