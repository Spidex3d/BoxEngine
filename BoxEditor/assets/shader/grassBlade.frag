#version 460 core

// ============================================
// Grass Blade Fragment Shader
// ============================================

in vec3 vNormal;
in vec3 vWorldPosition;
in vec2 vTexCoord;

out vec4 FragColor;

// ============================================
// Grass Colour Ramp
// ============================================

// Dark green near the roots
uniform vec3 uBaseColor =
    vec3(0.07, 0.22, 0.06);

// Medium green
uniform vec3 uMiddleColor =
    vec3(0.22, 0.50, 0.14);

// Light green near the tip
uniform vec3 uTipColor =
    vec3(0.55, 0.75, 0.27);

// ============================================
// Lighting
// ============================================

// Direction towards the light
uniform vec3 uLightDirection =
    vec3(0.4, 1.0, 0.3);

uniform vec3 uLightColor =
    vec3(1.0);

// Ambient lighting
uniform float uAmbientStrength = 0.35;

// ============================================
// Main
// ============================================

void main()
{
    // ----------------------------------------
    // Height along the blade
    //
    // 0.0 = bottom
    // 1.0 = tip
    // ----------------------------------------

    float height = clamp(
        vTexCoord.y,
        0.0,
        1.0
    );

    // ========================================
    // GRASS COLOUR RAMP
    // ========================================

    vec3 grassColor;

    if (height < 0.5)
    {
        // Dark base to medium green
        float t = smoothstep(
            0.0,
            0.5,
            height
        );

        grassColor = mix(
            uBaseColor,
            uMiddleColor,
            t
        );
    }
    else
    {
        // Medium green to light tip
        float t = smoothstep(
            0.5,
            1.0,
            height
        );

        grassColor = mix(
            uMiddleColor,
            uTipColor,
            t
        );
    }

    // ========================================
    // LIGHTING
    // ========================================

    vec3 normal = normalize(vNormal);

    // Flip the normal for the back of the blade.
    // This allows both sides to receive lighting.
    if (!gl_FrontFacing)
    {
        normal = -normal;
    }

    vec3 lightDirection =
        normalize(uLightDirection);

    float diffuseStrength =
        max(
            dot(normal, lightDirection),
            0.0
        );

    // ----------------------------------------
    // Ambient
    // ----------------------------------------

    vec3 ambient =
        grassColor *
        uAmbientStrength;

    // ----------------------------------------
    // Diffuse
    // ----------------------------------------

    vec3 diffuse =
        grassColor *
        diffuseStrength *
        uLightColor;

    // ========================================
    // FINAL COLOUR
    // ========================================

    vec3 finalColor = ambient + diffuse;

    FragColor = vec4(finalColor, 1.0);
}