#version 460 core

in vec3 vNormal;

//in vec3 vFragPos;
in vec3 vWorldPosition;

in vec2 vTexCoord;

// Matirials
uniform vec4 uMaterialColors[8];
flat in int vMaterialIndex;


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
    
    vec3 normal = normalize(vNormal);

    // new material
    vec4 materialColor = uMaterialColors[
        min(vMaterialIndex, 7u)
    ];


    if (uUseBaseColorTexture)
    {
    materialColor *= texture(uBaseColorTexture, vTexCoord);
    }

    //vec3 lightDirection = normalize(uLightPosition - vFragPos);
    vec3 lightDirection = normalize(uLightPosition - vWorldPosition);
    
    //vec3 viewDirection =  normalize(uCameraPosition - vFragPos);
    vec3 viewDirection =  normalize(uCameraPosition - vWorldPosition);

    vec3 reflectDirection = reflect(-lightDirection, normal);

    float shininess = mix(128.0, 4.0, uRoughness);

    float specularAmount = pow(max(dot(viewDirection, reflectDirection), 0.0), shininess);

    vec3 dielectricSpecular = vec3(0.04);

   // vec3 specularColor = mix(dielectricSpecular, uBaseColor.rgb, uMetallic);
    vec3 specularColor = mix(dielectricSpecular, materialColor.rgb, uMetallic);

    //vec3 diffuse = uBaseColor.rgb * max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = materialColor.rgb * max(dot(normal, lightDirection), 0.0);

    diffuse *= 1.0 - uMetallic;

    vec3 specular = specularColor * specularAmount;
    
    float ambientStrength = 0.12;

    //vec3 ambient = uBaseColor.rgb * ambientStrength;
    vec3 ambient = materialColor.rgb * ambientStrength;


    vec3 finalColor = ambient + diffuse + specular;

    

    finalColor *= uLightColor;

    FragColor = vec4(finalColor, materialColor.a);
   
 
}
