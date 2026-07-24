#version 460 core
in vec3 FragNormal;

out vec4 FragColor;

uniform vec4 uBaseColor;
uniform vec3 uLightDirection;

void main()
{
    vec3 normal = normalize(FragNormal);

    float diffuse = max(dot(normal, normalize(-uLightDirection)), 0.0);

    float ambient = 0.25;

    float lighting = ambient + diffuse * 0.75;

    vec3 finalColor = uBaseColor.rgb * lighting;

    FragColor = vec4(finalColor, uBaseColor.a);
}


//in vec3 vNormal;
//in vec3 vWorldPosition;
//
//out vec4 FragColor;
//
////uniform vec3 uObjectColor;
//uniform vec3 uBaseColor;
//uniform vec3 uLightDirection;
//
//void main()
//{
//    vec3 normal =
//        normalize(vNormal);
//
//    vec3 lightDirection =
//        normalize(-uLightDirection);
//
//    float diffuse =
//        max(
//            dot(normal, lightDirection),
//            0.0
//        );
//
//    float ambient = 0.25;
//
//    //vec3 finalColor = uObjectColor * (ambient + diffuse * 0.75);
//    vec3 finalColor = uBaseColor * (ambient + diffuse * 0.75);
//
//    FragColor = vec4(finalColor, 1.0);
//}