#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 Tangent;
//layout (location = 4) in vec3 Bitangent;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;
//out vec3 FragmentPos;
//out vec4 FragPosLightSpace;

uniform mat4 projectionMat;
uniform mat4 viewMat;
uniform mat4 modelMat;
//uniform mat4 lightSpaceMatrix;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    FragPos = vec3(modelMat * vec4(position, 1.0));   
    TexCoords = texCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(modelMat)));
    vec3 T = normalize(normalMatrix * Tangent);
    vec3 N = normalize(normalMatrix * normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));    
    TangentLightPos = TBN * lightPos;
    TangentViewPos  = TBN * viewPos;
    TangentFragPos  = TBN * FragPos;
    
    //FragPosLightSpace = lightSpaceMatrix * vec4(FragmentPos, 1.0);

    gl_Position = projectionMat * viewMat * modelMat * vec4(position, 1.0);
}