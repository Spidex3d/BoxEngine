#version 460 core

in vec3 vNormal;
in vec3 vWorldPosition;

out vec4 FragColor;

uniform vec3 uObjectColor;
uniform vec3 uLightDirection;

void main()
{
    vec3 normal =
        normalize(vNormal);

    vec3 lightDirection =
        normalize(-uLightDirection);

    float diffuse =
        max(
            dot(normal, lightDirection),
            0.0
        );

    float ambient = 0.25;

    vec3 finalColor =
        uObjectColor *
        (ambient + diffuse * 0.75);

    FragColor = vec4(
        finalColor,
        1.0
    );
}