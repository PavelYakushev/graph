#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 coordinates;
layout (location = 2) in vec3 normal;

out vec2 texCoords;
out vec3 Normal;
out vec3 FragmentPos;
out vec4 FragPosLightSpace;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;
uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = projectionMat * viewMat * modelMat * vec4(position, 1.0f);
    texCoords = coordinates;
    Normal = mat3(transpose(inverse(modelMat))) * normal;
    FragmentPos = vec3(modelMat * vec4(position, 1.0f));
    FragPosLightSpace = lightSpaceMatrix * vec4(FragmentPos, 1.0);
}