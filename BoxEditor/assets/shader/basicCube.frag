#version 460 core

in vec3 vNormal;

//in vec3 vFragPos;
in vec3 vWorldPosition;

in vec2 vTexCoord;

in vec3 vTangent;
in vec3 vBitangent;

// Matirials
uniform vec4 uMaterialColors[8];
flat in int vMaterialIndex;
// Matirials - Textures 
uniform sampler2D uMaterialTextures[8];
uniform int uMaterialUsesTexture[8];
// Metallic & Roughness
uniform float uMaterialMetallic[8];
uniform float uMaterialRoughness[8];
// Emission
uniform vec3  uMaterialEmissionColor[8];
uniform float uMaterialEmissionStrength[8];
// Normal Maps
uniform sampler2D uMaterialNormalTextures[8];
uniform int uMaterialUsesNormalTexture[8];
uniform float uMaterialNormalStrength[8];

out vec4 FragColor;


uniform vec4 uBaseColor;
uniform vec3 uLightDirection;
// new
uniform sampler2D uBaseColorTexture;
uniform bool uUseBaseColorTexture;


uniform float uMetallic;
uniform float uRoughness;
uniform vec3 uCameraPosition;
uniform vec3 uLightPosition;
uniform vec3 uLightColor;


void main()
{
    
  
    // Normal Mapping
    vec3 normal = normalize(vNormal);

int materialIndex =
    clamp(
        vMaterialIndex,
        0,
        7
    );

if (uMaterialUsesNormalTexture[
        materialIndex] != 0)
{
    vec3 normalSample =
        texture(
            uMaterialNormalTextures[
                materialIndex
            ],
            vTexCoord
        ).rgb;

    normalSample =
        normalSample *
        2.0 -
        1.0;

    normalSample.xy *=
        uMaterialNormalStrength[
            materialIndex
        ];

    normalSample =
        normalize(normalSample);

    mat3 TBN =
        mat3(
            normalize(vTangent),
            normalize(vBitangent),
            normalize(vNormal)
        );

    normal =
        normalize(
            TBN *
            normalSample
        );
}

    // #########

   // int materialIndex = clamp(vMaterialIndex, 0, 7);

    vec4 materialColor = uMaterialColors[materialIndex];

    if (uMaterialUsesTexture[materialIndex] != 0)
     {
        materialColor *=
            texture(
                uMaterialTextures[
                    materialIndex
                ],
                vTexCoord
            );
        }
        // Metallic & Roughness
   float metallic = uMaterialMetallic[materialIndex];

   float roughness = uMaterialRoughness[materialIndex];

                // Emission
    vec3 emissionColor = uMaterialEmissionColor[materialIndex];
    float emissionStrength = uMaterialEmissionStrength[materialIndex];


    //vec3 lightDirection = normalize(uLightPosition - vFragPos);
    vec3 lightDirection = normalize(uLightPosition - vWorldPosition);
    
    //vec3 viewDirection =  normalize(uCameraPosition - vFragPos);
    vec3 viewDirection =  normalize(uCameraPosition - vWorldPosition);

    vec3 reflectDirection = reflect(-lightDirection, normal);

    //float shininess = mix(128.0, 4.0, uRoughness);
    float shininess = mix(128.0, 4.0, roughness);

    float specularAmount = pow(max(dot(viewDirection, reflectDirection), 0.0), shininess);

    vec3 dielectricSpecular = vec3(0.04);

    //vec3 specularColor = mix(dielectricSpecular, materialColor.rgb, uMetallic);
    vec3 specularColor = mix(dielectricSpecular, materialColor.rgb, metallic);

    //vec3 diffuse = uBaseColor.rgb * max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = materialColor.rgb * max(dot(normal, lightDirection), 0.0);

   // diffuse *= 1.0 - uMetallic;
    diffuse *= 1.0 - metallic;

    vec3 specular = specularColor * specularAmount;
    
    float ambientStrength = 0.12;

    //vec3 ambient = uBaseColor.rgb * ambientStrength;
    vec3 ambient = materialColor.rgb * ambientStrength;

    vec3 finalColor =
    ambient +
    diffuse +
    specular;

finalColor *=
    uLightColor;


// --------------------------------
// Emission
// --------------------------------

vec3 emission =  emissionColor * emissionStrength;

finalColor += emission;


FragColor = vec4(finalColor, materialColor.a);
 
}
