#include <iostream>
#include <cmath>
#include <string>
#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "stb_image.h"
//#define DEBUG


//====================GLOBAL==========================
//window dimensions
const GLuint WIDTH = 1280, HEIGHT = 960;
//keyboard related
bool keys[1024];
//camera related
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = (GLfloat)WIDTH / 2.0;
GLfloat lastY = (GLfloat)HEIGHT / 2.0;
bool firstMouse = true;
//lighting
glm::vec3 directLightPos(-11.0f, -2.0f, -5.0f);
glm::vec3 mirrorCubePos(-2.5f, 1.5f, 2.0f);
const int numberOfPointLights = 2;
//deltatime-time between current frame and last frame
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
//================================================================================
//======================================FUNCTIONS=================================
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    std::cout<<key<<std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS) {
            keys[key] = true;
        }
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void moveCamera(){
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keys[GLFW_KEY_Z])
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (keys[GLFW_KEY_X])
        camera.ProcessKeyboard(UP, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadSkybox(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Skybox tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void drawFloor(const glm::mat4 projectionMat, const unsigned int planeVAO, Shader myShader, const unsigned int floorTexture)
{
    glm::mat4 modelMat = glm::mat4(1.0f);
    glm::mat4 viewMat = camera.GetViewMatrix();

    glStencilMask(0x00);

    myShader.setMat4("viewMat", viewMat);
    myShader.setMat4("projectionMat", projectionMat);
    myShader.Use();
    glBindVertexArray(planeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, -0.01f, 0.0f));
    myShader.setMat4("modelMat", modelMat);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);

    glStencilMask(0xFF);
}

void drawNMap(const glm::mat4 projectionMat, const unsigned int nMapVAO, Shader shader, const unsigned int diffuseMap, const unsigned int normalMap)
{
    glm::mat4 viewMat = camera.GetViewMatrix();
    shader.Use();
    shader.setMat4("projectionMat", projectionMat);
    shader.setMat4("viewMat", viewMat);
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(5.0f, 0.5f, 2.0f));
    modelMat = glm::rotate(modelMat, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    modelMat = glm::scale(modelMat, glm::vec3(0.7f));
    shader.setMat4("modelMat", modelMat);
    shader.setVec3("viewPos", camera.Position);
    shader.setVec3("lightPos", -directLightPos);
    //shader.setVec3("lightAmbient", glm::vec3(0.05f));
    //shader.setVec3("lightDiffuse", glm::vec3(0.7f));
    //shader.setVec3("lightSpecular", glm::vec3(1.0f));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glBindVertexArray(nMapVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void drawParallax(const glm::mat4 projectionMat, const unsigned int parallaxVAO, Shader shader, const unsigned int diffuseMap,
    const unsigned int normalMap, const unsigned int heightMap)
{
    glm::mat4 viewMat = camera.GetViewMatrix();
    shader.Use();
    shader.setMat4("projectionMat", projectionMat);
    shader.setMat4("viewMat", viewMat);
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(5.0f, 0.5f, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(sin((float)glfwGetTime()) * 10.0f + 90.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
    modelMat = glm::scale(modelMat, glm::vec3(0.7f));
    shader.setMat4("modelMat", modelMat);
    shader.setVec3("viewPos", camera.Position);
    shader.setVec3("lightPos", -directLightPos);
    shader.setFloat("heightScale", 0.1f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, heightMap);
    glBindVertexArray(parallaxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void drawCubesAndOutline(const glm::mat4 projectionMat, const unsigned int containerVAO, Shader myShader, Shader outlineShader, glm::vec3* cubePositions,
    const unsigned int diffuseMap, const unsigned int specularMap, const unsigned int emissionMap)
{
    glm::mat4 viewMat = camera.GetViewMatrix();

    myShader.Use();
    myShader.setMat4("viewMat", viewMat);
    myShader.setMat4("projectionMat", projectionMat);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, emissionMap);

    //Draw figures
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    glBindVertexArray(containerVAO);
    for (unsigned int i = 0; i < 5; i++)
    {
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, cubePositions[i]);
        myShader.setMat4("modelMat", modelMat);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);

    //draw outline
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    //glDisable(GL_DEPTH_TEST);
    outlineShader.Use();
    float scale = 1.005f;

    outlineShader.setVec3("outlineColor", glm::vec3(0.5f, 0.5f, 0.0f));
    outlineShader.setMat4("viewMat", viewMat);
    outlineShader.setMat4("projectionMat", projectionMat);

    glBindVertexArray(containerVAO);
    for (unsigned int i = 0; i < 5; i++)
    {
        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, cubePositions[i]);
        modelMat = glm::scale(modelMat, glm::vec3(scale));
        outlineShader.setMat4("modelMat", modelMat);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
}

void drawSkyboxAndCubes(const glm::mat4 projectionMat, const unsigned int skyboxVAO, const unsigned int mirrorVAO, Shader skyboxShader, Shader mirrorShader,
    const unsigned int skyboxTexture)
{
    glm::mat4 viewMat = glm::mat4(1.0f);
    glm::mat4 modelMat = glm::mat4(1.0f);

    //draw skybox
    glDepthFunc(GL_LEQUAL);
    skyboxShader.Use();
    viewMat = glm::mat4(glm::mat3(camera.GetViewMatrix()));     //we will F' up view matrix to get rid of translation, but we will only do it for skybox
    skyboxShader.setMat4("viewMat", viewMat);
    skyboxShader.setMat4("projectionMat", projectionMat);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    viewMat = camera.GetViewMatrix();               //here we are "restoring" the "right" view matrix

    //draw mirror cube
    mirrorShader.Use();
    glm::mat4 mirrorModelMat = glm::mat4(1.0f);
    mirrorModelMat = glm::translate(mirrorModelMat, mirrorCubePos);
    mirrorModelMat = glm::rotate(mirrorModelMat, glm::radians((float)glfwGetTime() * 20.0f), glm::normalize(glm::vec3(-1.0, 1.0, -1.0)));
    mirrorModelMat = glm::scale(mirrorModelMat, glm::vec3(0.7f));
    mirrorShader.setMat4("modelMat", mirrorModelMat);
    mirrorShader.setMat4("viewMat", viewMat);
    mirrorShader.setMat4("projectionMat", projectionMat);
    mirrorShader.setVec3("cameraPos", camera.Position);
    mirrorShader.setBool("refractFlag", false);
    glBindVertexArray(mirrorVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    mirrorShader.Use();
    mirrorModelMat = glm::mat4(1.0f);
    mirrorModelMat = glm::translate(mirrorModelMat, mirrorCubePos + glm::vec3(0.0f, 1.0f, 1.0f));
    mirrorModelMat = glm::rotate(mirrorModelMat, glm::radians((float)glfwGetTime() * 20.0f), glm::normalize(glm::vec3(-1.0, 1.0, -1.0)));
    mirrorModelMat = glm::scale(mirrorModelMat, glm::vec3(0.7f));
    mirrorShader.setMat4("modelMat", mirrorModelMat);
    mirrorShader.setMat4("viewMat", viewMat);
    mirrorShader.setMat4("projectionMat", projectionMat);
    mirrorShader.setVec3("cameraPos", camera.Position);
    mirrorShader.setBool("refractFlag", true);
    glBindVertexArray(mirrorVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void drawBillboards(const glm::mat4 projectionMat, const unsigned int transparentVAO, Shader billboardShader, std::vector<glm::vec3> billboards,
    const unsigned int billboardTexture)
{
    glm::mat4 viewMat = camera.GetViewMatrix();
    glm::mat4 modelMat = glm::mat4(1.0f);

    //sorting billboards by distance
    std::multimap<float, glm::vec3> sortedBillboards;
    for (unsigned int i = 0; i < billboards.size(); i++)
    {
        float distance = glm::length(camera.Position - billboards[i]);
        sortedBillboards.insert(std::make_pair(distance, billboards[i]));
    }


    //drawing billboards
    billboardShader.Use();
    glBindVertexArray(transparentVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, billboardTexture);
    //billboardShader.setVec3("cameraPos", camera.Position);
    billboardShader.setMat4("viewMat", viewMat);
    billboardShader.setMat4("projectionMat", projectionMat);
    for (std::map<float, glm::vec3>::reverse_iterator it = sortedBillboards.rbegin(); it != sortedBillboards.rend(); ++it)
    {
        modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, it->second);
        billboardShader.setMat4("modelMat", modelMat);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glBindVertexArray(0);
}

void drawSceneForShadows(Shader shader, const unsigned int planeVAO, const unsigned int containerVAO, const unsigned int mirrorVAO,
    const unsigned int nMapVAO, glm::vec3 *cubePositions)
{
    //floor
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(0.0f, -0.01f, 0.0f));
    shader.setMat4("modelMat", modelMat);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    //cubes
    glBindVertexArray(containerVAO);
    for (unsigned int i = 0; i < 3; i++)
    {
        modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, cubePositions[i]);
        shader.setMat4("modelMat", modelMat);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
    
    //mirror cube
    glm::mat4 mirrorModelMat = glm::mat4(1.0f);
    mirrorModelMat = glm::translate(mirrorModelMat, mirrorCubePos);
    mirrorModelMat = glm::rotate(mirrorModelMat, glm::radians((float)glfwGetTime() * 20.0f), glm::normalize(glm::vec3(-1.0, 1.0, -1.0)));
    mirrorModelMat = glm::scale(mirrorModelMat, glm::vec3(0.7f));
    shader.setMat4("modelMat", mirrorModelMat);
    glBindVertexArray(mirrorVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    //refracting cube
    mirrorModelMat = glm::mat4(1.0f);
    mirrorModelMat = glm::translate(mirrorModelMat, mirrorCubePos + glm::vec3(0.0f, 1.0f, 1.0f));
    mirrorModelMat = glm::rotate(mirrorModelMat, glm::radians((float)glfwGetTime() * 20.0f), glm::normalize(glm::vec3(-1.0, 1.0, -1.0)));
    mirrorModelMat = glm::scale(mirrorModelMat, glm::vec3(0.7f));
    shader.setMat4("modelMat", mirrorModelMat);
    glBindVertexArray(mirrorVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    //normal mapping plane
    modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(5.0f, 0.5f, 2.0f));
    modelMat = glm::rotate(modelMat, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    modelMat = glm::scale(modelMat, glm::vec3(0.7f));
    shader.setMat4("modelMat", modelMat);
    glBindVertexArray(nMapVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    //parallax mapping plane
    modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, glm::vec3(5.0f, 0.5f, 0.0f));
    modelMat = glm::rotate(modelMat, glm::radians(sin((float)glfwGetTime()) * 10.0f + 90.0f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
    modelMat = glm::scale(modelMat, glm::vec3(0.7f));
    shader.setMat4("modelMat", modelMat);
    glBindVertexArray(nMapVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

#ifdef DEBUG
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
        // координаты        // текстурные координаты
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
#endif
//=====================================================================================================================================================================================================

int main()
{
    //Init GLFW
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Graphics", NULL, NULL);
    if (window == NULL)
    {
        std::cout<<"Failed to create GLFW window"<<std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Build and compile our shader programs
    Shader myShader("../shaders/default.vs", "../shaders/default.fs");
    Shader outlineShader("../shaders/outline.vs", "../shaders/outline.fs");
    Shader billboardShader("../shaders/billboard.vs", "../shaders/billboard.fs");
    Shader skyboxShader("../shaders/skybox.vs", "../shaders/skybox.fs");
    Shader mirrorShader("../shaders/mirrorCube.vs", "../shaders/mirrorCube.fs");
    Shader simpleDepthShader("../shaders/shadow_mapping.vs", "../shaders/shadow_mapping.fs");
    Shader nMapShader("../shaders/normal_mapping.vs", "../shaders/normal_mapping.fs");
    Shader parallaxShader("../shaders/parallax.vs", "../shaders/parallax.fs");
#ifdef DEBUG
    Shader debugDepthQuad("../shaders/3.1.3.debug_quad.vs", "../shaders/3.1.3.debug_quad.fs");    //DEBUG
#endif

    float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
    };

    float vertices[] = {
         //Position            //TextureCoord    //Normals
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,    0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 0.0f,    0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,    0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,    0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,     0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 1.0f,     0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 1.0f,     0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,    -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    1.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,    -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    1.0f, 0.0f,    -1.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,     1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    0.0f, 1.0f,     1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, 0.0f,     1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,    0.0f, -1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 1.0f,    0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,    1.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,    0.0f, -1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,     0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f,     0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,    1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f,     0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,    0.0f, 0.0f,     0.0f, 1.0f, 0.0f
    };

    float planeVertices[] = {
        //Coordinates         //TextureCoord    //Normals
         10.0f, -0.5f,  10.0f,   10.0f, 0.0f,     0.0f, 1.0f, 0.0f,
        -10.0f, -0.5f, -10.0f,   0.0f, 10.0f,     0.0f, 1.0f, 0.0f,
        -10.0f, -0.5f,  10.0f,   0.0f,  0.0f,     0.0f, 1.0f, 0.0f,

         10.0f, -0.5f,  10.0f,   10.0f, 0.0f,     0.0f, 1.0f, 0.0f,
         10.0f, -0.5f, -10.0f,   10.0f,10.0f,     0.0f, 1.0f, 0.0f,
        -10.0f, -0.5f, -10.0f,   0.0f, 10.0f,     0.0f, 1.0f, 0.0f
    };

    float transparentVertices[] = {
        //Coordinates        //TextureCoords
         0.0f,  0.5f,  0.0f,   0.0f, 0.0f,
         0.0f, -0.5f,  0.0f,   0.0f, 1.0f,
         1.0f, -0.5f,  0.0f,   1.0f, 1.0f,

         0.0f,  0.5f,  0.0f,   0.0f, 0.0f,
         1.0f, -0.5f,  0.0f,   1.0f, 1.0f,
         1.0f,  0.5f,  0.0f,   1.0f, 0.0f
    };
    //different WORLD posistions for cubes
    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        //glm::vec3(2.0f,  5.0f, -15.0f),
        //glm::vec3(-1.5f, -2.2f, -2.5f),
        //glm::vec3(-3.8f, -2.0f, -12.3f),
        //glm::vec3(2.4f, -0.4f, -3.5f),
        //glm::vec3(-1.7f,  3.0f, -7.5f),
        //glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(0.2f,  0.0f, -1.1f),
        glm::vec3(1.5f,  1.2f, 0.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    //and billboards
    std::vector<glm::vec3> billboards
    {
        glm::vec3(-1.9f, 1.3f, 0.48f),
        glm::vec3(1.0f, 2.5f, 0.51f),
        glm::vec3(-0.7f, 1.5f, 1.0f)
    };
    //skybox locatoins and load
    std::vector<std::string> skyboxFaces
    {
        "../textures/skybox/right.jpg",
        "../textures/skybox/left.jpg",
        "../textures/skybox/top.jpg",
        "../textures/skybox/bottom.jpg",
        "../textures/skybox/front.jpg",
        "../textures/skybox/back.jpg"
    };
    unsigned int skyboxTexture = loadSkybox(skyboxFaces);

    stbi_set_flip_vertically_on_load(true);

    //for cubes
    unsigned int VBO, containerVAO;
    glGenVertexArrays(1, &containerVAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(containerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //for floor
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    //for billboards
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    //for skybox
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    
    //for mirror and refraction cubes
    unsigned int mirrorVAO;
    glGenVertexArrays(1, &mirrorVAO);
    glBindVertexArray(mirrorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(5 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    //for normal mapping
    unsigned int nMapVAO, nMapVBO;
    //coords
    glm::vec3 nMapPos1(-1.0f, 1.0f, 0.0f);
    glm::vec3 nMapPos2(-1.0f, -1.0f, 0.0f);
    glm::vec3 nMapPos3(1.0f, -1.0f, 0.0f);
    glm::vec3 nMapPos4(1.0f, 1.0f, 0.0f);
    //texture
    glm::vec2 nMapuv1(0.0f, 1.0f);
    glm::vec2 nMapuv2(0.0f, 0.0f);
    glm::vec2 nMapuv3(1.0f, 0.0f);
    glm::vec2 nMapuv4(1.0f, 1.0f);
    //normal
    glm::vec3 nMapNorm(0.0f, 0.0f, 1.0f);

    //tangent and bitangent vectors of both triangles
    glm::vec3 tangent1, bitangent1;
    glm::vec3 tangent2, bitangent2;

    //1st triangle
    glm::vec3 edge1 = nMapPos2 - nMapPos1;
    glm::vec3 edge2 = nMapPos3 - nMapPos1;
    glm::vec2 deltaUV1 = nMapuv2 - nMapuv1;
    glm::vec2 deltaUV2 = nMapuv3 - nMapuv1;

    float nMapf = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent1.x = nMapf * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent1.y = nMapf * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent1.z = nMapf * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent1 = glm::normalize(tangent1);

    bitangent1.x = nMapf * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent1.y = nMapf * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent1.z = nMapf * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent1 = glm::normalize(bitangent1);

    //2nd triangle
    edge1 = nMapPos3 - nMapPos1;
    edge2 = nMapPos4 - nMapPos1;
    deltaUV1 = nMapuv3 - nMapuv1;
    deltaUV2 = nMapuv4 - nMapuv1;

    nMapf = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent2.x = nMapf * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent2.y = nMapf * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent2.z = nMapf * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent2 = glm::normalize(tangent2);

    bitangent2.x = nMapf * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent2.y = nMapf * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent2.z = nMapf * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent2 = glm::normalize(bitangent2);

    float quadVertices[] = {
        //coords                            //normals                           //texture coords      //tangent                           //bitangent
        nMapPos1.x, nMapPos1.y, nMapPos1.z, nMapNorm.x, nMapNorm.y, nMapNorm.z, nMapuv1.x, nMapuv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        nMapPos2.x, nMapPos2.y, nMapPos2.z, nMapNorm.x, nMapNorm.y, nMapNorm.z, nMapuv2.x, nMapuv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        nMapPos3.x, nMapPos3.y, nMapPos3.z, nMapNorm.x, nMapNorm.y, nMapNorm.z, nMapuv3.x, nMapuv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

        nMapPos1.x, nMapPos1.y, nMapPos1.z, nMapNorm.x, nMapNorm.y, nMapNorm.z, nMapuv1.x, nMapuv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
        nMapPos3.x, nMapPos3.y, nMapPos3.z, nMapNorm.x, nMapNorm.y, nMapNorm.z, nMapuv3.x, nMapuv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
        nMapPos4.x, nMapPos4.y, nMapPos4.z, nMapNorm.x, nMapNorm.y, nMapNorm.z, nMapuv4.x, nMapuv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
    };
    glGenVertexArrays(1, &nMapVAO);
    glGenBuffers(1, &nMapVBO);
    glBindVertexArray(nMapVAO);
    glBindBuffer(GL_ARRAY_BUFFER, nMapVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //framebuffer for shadows
    const unsigned int SHADOW_WIDTH = 1280, SHADOW_HEIGHT = 1280;
    unsigned int shadowMapFBO;
    glGenFramebuffers(1, &shadowMapFBO);
    unsigned int shadowMap;
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int diffuseMap = loadTexture("../textures/container2.png");
    unsigned int specularMap = loadTexture("../textures/container2_specular.png");
    unsigned int emissionMap = loadTexture("../textures/matrix.jpg");
    unsigned int floorTexture = loadTexture("../textures/metal_floor.jpg");
    unsigned int billboardTexture = loadTexture("../textures/window.png");
    unsigned int nMapDiffuseMap = loadTexture("../textures/brickwall.jpg");
    unsigned int nMapNormalMap = loadTexture("../textures/brickwall_normal.jpg");
    unsigned int parallaxDiffuse = loadTexture("../textures/toy_box_diffuse.png");
    unsigned int parallaxNormal = loadTexture("../textures/toy_box_normal.png");
    unsigned int parallaxHeight = loadTexture("../textures/toy_box_disp.png");

    //we need to set up proper texture unit
    myShader.Use();
    myShader.setInt("material.diffuse", 0);
    myShader.setInt("material.specular", 1);
    myShader.setInt("material.emission", 2);
    myShader.setInt("shadowMap", 3);
    billboardShader.Use();
    billboardShader.setInt("billboardTexture", 0);
    skyboxShader.Use();
    skyboxShader.setInt("skybox", 0);
    nMapShader.Use();
    nMapShader.setInt("diffuseMap", 0);
    nMapShader.setInt("normalMap", 1);
    parallaxShader.Use();
    parallaxShader.setInt("diffuseMap", 0);
    parallaxShader.setInt("normalMap", 1);
    parallaxShader.setInt("depthMap", 2);

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done

    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        moveCamera();

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //transformations
        glm::mat4 projectionMat = glm::mat4(1.0f);
        glm::mat4 viewMat = glm::mat4(1.0f);
        glm::mat4 modelMat = glm::mat4(1.0f);
        projectionMat = glm::perspective(glm::radians(camera.Zoom), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        viewMat = camera.GetViewMatrix();

        myShader.Use();
        //passing all sorts of values to the shader
        myShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
        myShader.setFloat("time", 5.0 * currentFrame);
        //material
        myShader.setFloat("material.shininess", 64.0f);
        //lights
        glm::vec3 lightColor = glm::vec3(1.0f);
        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
        glm::vec3 ambientColor = lightColor * glm::vec3(0.2f); // low influence
        //direction light
        myShader.setVec3("directLight.direction", directLightPos);
        myShader.setVec3("directLight.ambient", glm::vec3(0.05f));
        myShader.setVec3("directLight.diffuse", glm::vec3(0.7f));
        myShader.setVec3("directLight.specular", glm::vec3(1.0f));

        //spotlight
        myShader.setVec3("spotlight.position", camera.Position);
        myShader.setVec3("spotlight.direction", camera.Front);
        myShader.setFloat("spotlight.cutOff", glm::cos(glm::radians(12.5f)));
        myShader.setFloat("spotlight.outerCutOff", glm::cos(glm::radians(15.5f)));
        myShader.setFloat("spotlight.constant", 1.0f);          //chose constants for 50 units
        myShader.setFloat("spotlight.linear", 0.09f);
        myShader.setFloat("spotlight.quadratic", 0.032f);
        myShader.setVec3("spotlight.ambient", glm::vec3(0.0f));
        myShader.setVec3("spotlight.diffuse", glm::vec3(1.0f));
        myShader.setVec3("spotlight.specular", glm::vec3(1.0f));

        //first we draw the scene to make shadow map
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 20.0f;
        //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // обратите внимание, что если вы используете матрицу перспективной проекции, вам придется изменить положение света, так как текущего положения света недостаточно для отображения всей сцены
        lightProjection = glm::ortho(-15.0f, 20.0f, -15.0f, 20.0f, near_plane, far_plane);
        lightView = glm::lookAt(-directLightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        simpleDepthShader.Use();
        simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        drawSceneForShadows(simpleDepthShader, planeVAO, containerVAO, mirrorVAO, nMapVAO, cubePositions);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //then we draw the scene normally
        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        myShader.Use();
        myShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, shadowMap);

        drawFloor(projectionMat, planeVAO, myShader, floorTexture);
        drawNMap(projectionMat, nMapVAO, nMapShader, nMapDiffuseMap, nMapNormalMap);
        drawParallax(projectionMat, nMapVAO, parallaxShader, parallaxDiffuse, parallaxNormal, parallaxHeight);
        drawCubesAndOutline(projectionMat, containerVAO, myShader, outlineShader, cubePositions, diffuseMap, specularMap, emissionMap);
        drawSkyboxAndCubes(projectionMat, skyboxVAO, mirrorVAO, skyboxShader, mirrorShader, skyboxTexture);
        drawBillboards(projectionMat, transparentVAO, billboardShader, billboards, billboardTexture);

#ifdef DEBUG
        //DEBUG
        // рендеринг на плоскости карты глубины для наглядной отладки
        // ---------------------------------------------
        debugDepthQuad.Use();
        debugDepthQuad.setFloat("near_plane", near_plane);
        debugDepthQuad.setFloat("far_plane", far_plane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        renderQuad();
#endif
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &containerVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &transparentVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &mirrorVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &transparentVBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}