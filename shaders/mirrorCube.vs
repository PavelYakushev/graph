#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 Position;
out vec3 Normal;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;

void main()
{
    Normal = mat3(transpose(inverse(modelMat))) * normal;
    Position = vec3(modelMat * vec4(position, 1.0));
    gl_Position = projectionMat * viewMat * vec4(Position, 1.0f);
}