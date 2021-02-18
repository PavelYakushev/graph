#version 330 core
out vec4 FragColor;
 
in vec3 Normal;
in vec3 Position;
 
uniform bool refractFlag;
uniform vec3 cameraPos;
uniform samplerCube skybox;
 
void main()
{    
    float ratio = 1.00 / 1.52;
    vec3 I = normalize(Position - cameraPos);
    vec3 R = vec3(1.0, 1.0, 1.0);
    if (refractFlag)
        R = refract(I, normalize(Normal), ratio);
    else
        R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}