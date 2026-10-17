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
uniform sampler2D normal_grass;
uniform sampler2D normal_dirt;


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
uniform float grassNormalStrength;
uniform float dirtNormalStrength;


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
    vec3 rockNormal = texture(normal_rock,  tiledUV).rgb;
    // Convert normal texture from 0..1 to -1..1.
    rockNormal = rockNormal * 2.0 - 1.0;
    rockNormal.x *= rockNormalStrength;
    rockNormal.y *= rockNormalStrength;
    rockNormal = normalize(rockNormal);

    // =================================================
    // GRASS NORMAL MAP
    // =================================================
    vec3 grassNormal = texture(normal_grass,  tiledUV).rgb;
    // Convert normal texture from 0..1 to -1..1.
    grassNormal = grassNormal * 2.0 - 1.0;
    grassNormal.x *= grassNormalStrength;
    grassNormal.y *= grassNormalStrength;
    grassNormal = normalize(grassNormal);

    // =================================================
    // DIRT NORMAL MAP
    // =================================================
    vec3 dirtNormal = texture(normal_dirt,  tiledUV).rgb;
    // Convert normal texture from 0..1 to -1..1.
    dirtNormal = dirtNormal * 2.0 - 1.0;
    dirtNormal.x *= dirtNormalStrength;
    dirtNormal.y *= dirtNormalStrength;
    dirtNormal = normalize(dirtNormal);

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


    mat3 TBN = mat3(tangent, bitangent, terrainNormal);

    vec3 detailedRockNormal = normalize(TBN * rockNormal);
    vec3 detailedGrassNormal = normalize(TBN * grassNormal);
    vec3 detailedDirtNormal = normalize(TBN * dirtNormal);


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


    float rockLight = max(dot(detailedRockNormal, detailLightDirection), 0.0);
    float grassLight = max(dot(detailedGrassNormal, detailLightDirection), 0.0);
    float dirtLight = max(dot(detailedDirtNormal, detailLightDirection), 0.0);


    // Keep this close to 1.0 so it does not make
    // the terrain dark.
    float rockDetailBrightness = mix(0.85, 1.15, rockLight);
    float grassDetailBrightness = mix(0.85, 1.15, grassLight);
    float dirtDetailBrightness = mix(0.85, 1.15, dirtLight);


    // =================================================
    // ORIGINAL TERRAIN COLOUR BLENDING
    // =================================================

    vec3 terrainColor = grass * grassBlend + dirt  * dirtBlend + rock  * rockBlend;


    // -------------------------------------------------
    // Apply normal-map detail ONLY to rock
    // -------------------------------------------------

    //terrainColor *= mix(1.0, rockDetailBrightness, rockBlend);

    float finalDetailBrightness =
    grassDetailBrightness * grassBlend +
    dirtDetailBrightness  * dirtBlend +
    rockDetailBrightness  * rockBlend;

    terrainColor *= finalDetailBrightness;


    // =================================================
    // FINAL OUTPUT
    // =================================================

    FragColor = vec4(terrainColor, 1.0);
}

