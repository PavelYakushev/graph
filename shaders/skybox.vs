#version 330 core
layout (location = 0) in vec3 position;
 
out vec3 texCoords;
 
uniform mat4 projectionMat;
uniform mat4 viewMat;
 
void main()
{
    texCoords = position;
    vec4 pos = projectionMat * viewMat * vec4(position, 1.0);
    gl_Position = pos.xyww;
}