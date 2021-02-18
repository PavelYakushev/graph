#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 coordinates;

out vec2 texCoords;

//uniform vec3 cameraPos;
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;

void main()
{
    /*
    mat3 M1 = inverse(mat3(viewMat));
    vec3 objToCam = normalize(cameraPos - position);
    vec3 objToCamProj = normalize(vec3(objToCam.x, 0.0, objToCam.z));
    vec3 right = normalize(vec3(M1[0][0], M1[1][0], M1 [2][0]));
    vec3 up = normalize(vec3(M1[0][1], M1[1][1], M1 [2][1]));
    vec3 lookAt = cross(up, right);
    */
    vec3 Position = inverse(mat3(viewMat)) * position;

    texCoords = coordinates;    
    gl_Position = projectionMat * viewMat * modelMat * vec4(Position, 1.0f);
}