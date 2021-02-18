#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

uniform mat4 projectionMat;
uniform mat4 viewMat;
uniform mat4 modelMat;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    FragPos = vec3(modelMat * vec4(aPos, 1.0));   
    TexCoords = aTexCoords;   
    
    vec3 T = normalize(mat3(modelMat) * aTangent);
    vec3 B = normalize(mat3(modelMat) * aBitangent);
    vec3 N = normalize(mat3(modelMat) * aNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    TangentLightPos = TBN * lightPos;
    TangentViewPos  = TBN * viewPos;
    TangentFragPos  = TBN * FragPos;
    
    gl_Position = projectionMat * viewMat * modelMat * vec4(aPos, 1.0);
}