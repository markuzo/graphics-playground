#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <tuple>
#include <random>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "plyreader.h"
#include "shader.h"

int g_width = 1024;
int g_height = 768;
bool g_windowResized = false;
GLFWwindow* g_window = nullptr;

glm::vec3 g_position(0.f,0.1f,0.5f);
glm::vec3 g_light(0.f,0.1f,0.5f);
glm::vec3 g_lightColor(0.f,0.1f,0.5f);

float g_horizontalAngle = 3.14f;
float g_verticalAngle = 0.f;
float g_speed = 50.0f;
float g_mouseSpeed = 5.0f;

bool g_shouldBlur = true;
float g_radius = 0.5f;
float g_bias = 0.025f;

bool g_isFlying = false;

glm::mat4 g_viewMat(1.f);

void onWindowResize(GLFWwindow* window, int width, int height) {
    g_width = width;
    g_height = height;
    g_windowResized = true;
}

void processInput(float deltaTime, double xpos, double ypos) {

    if (g_isFlying) {
        g_horizontalAngle += g_mouseSpeed * deltaTime * float(g_width/2 - xpos);
        g_verticalAngle   += g_mouseSpeed * deltaTime * float(g_height/2 - ypos);
        glfwSetCursorPos(g_window,  g_width/2,  g_height/2);
    }

    auto direction = glm::vec3(
        cos(g_verticalAngle) * sin(g_horizontalAngle),
        sin(g_verticalAngle),
        cos(g_verticalAngle) * cos(g_horizontalAngle)
    );

    auto right = glm::vec3(
        sin(g_horizontalAngle - 3.14f/2.0f),
        0,
        cos(g_horizontalAngle - 3.14f/2.0f)
    );
    auto up = glm::cross( right, direction );

    if (glfwGetKey(g_window, GLFW_KEY_W) == GLFW_PRESS) {
        g_position += direction * deltaTime * g_speed;
    }
    if (glfwGetKey(g_window, GLFW_KEY_S) == GLFW_PRESS) {
        g_position -= direction * deltaTime * g_speed;
    }
    if (glfwGetKey(g_window, GLFW_KEY_D) == GLFW_PRESS) {
        g_position += right * deltaTime * g_speed;
    }
    if (glfwGetKey(g_window, GLFW_KEY_A) == GLFW_PRESS) {
        g_position -= right * deltaTime * g_speed;
    }
    if (glfwGetKey(g_window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!g_isFlying)
            glfwSetCursorPos(g_window,  g_width/2,  g_height/2);
        g_isFlying = !g_isFlying;    
    }

    g_viewMat = glm::lookAt(g_position, g_position+direction, up);
}

int main() {
    if (!glfwInit()) {
        std::cout << "Can't init glfw" << std::endl;
        return 1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    g_window = glfwCreateWindow(g_width, g_height, "SSAO test", nullptr, nullptr);
    if (g_window == nullptr) {
        std::cout << "Can't create glfw window" << std::endl;
        return 1;
    }

    // getting the window size in case the window manager 
    // changed size (common in tiling WM)
    glfwGetWindowSize(g_window, &g_width, &g_height);
    glfwSetWindowSizeCallback(g_window, onWindowResize);
    glfwSetCursorPos(g_window,  g_width/2,  g_height/2);

    glfwMakeContextCurrent(g_window);
    if (glewInit() != GLEW_OK) {
        std::cout << "Can't init glew" << std::endl;
        return 1;
    }

    // UI setup
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    ImGui_ImplOpenGL3_Init("#version 450 core");
    ImGui::StyleColorsDark();

    // data 
    //const auto filename = "/home/markuzo/work/data/dragon_recon/dragon_vrip.ply";
    //const auto filename = "/home/markuzo/work/data/triangle/triangle.ply";
    const auto filename = "/home/markuzo/work/data/bunny/bunny/reconstruction/bun_zipper.ply";
    //const auto filename = "/home/markuzo/work/data/lucy/lucy.ply";
    
    auto [ vertices, indices, normals ] = PlyReader().readObj(filename);
    auto vsizeof = [] (const auto& v) { return sizeof(v[0])*v.size(); };

    // opengl
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint bufferV;
    glGenBuffers(1, &bufferV);
    glBindBuffer(GL_ARRAY_BUFFER, bufferV);
    glBufferData(GL_ARRAY_BUFFER, vsizeof(vertices), &vertices[0], GL_STATIC_DRAW);
    GLuint bufferI;
    glGenBuffers(1, &bufferI);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferI);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vsizeof(indices), &indices[0], GL_STATIC_DRAW);
    GLuint bufferN;
    glGenBuffers(1, &bufferN);
    glBindBuffer(GL_ARRAY_BUFFER, bufferN);
    glBufferData(GL_ARRAY_BUFFER, vsizeof(normals), &normals[0], GL_STATIC_DRAW);

    // shader layout: v=0,n=1
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, bufferV);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferI);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, bufferN);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);

    // post processing setup
    GLuint gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // first the position texture
    GLuint gPositionTexture;
    glGenTextures(1, &gPositionTexture);
    glBindTexture(GL_TEXTURE_2D, gPositionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_width, g_height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gPositionTexture, 0);

    // now the normal texture
    GLuint gNormalTexture;
    glGenTextures(1, &gNormalTexture);
    glBindTexture(GL_TEXTURE_2D, gNormalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_width, g_height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, gNormalTexture, 0);

    // now the albedo texture
    GLuint gAlbedoTexture;
    glGenTextures(1, &gAlbedoTexture);
    glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_width, g_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, gAlbedoTexture, 0);

    // now the depth texture (using renderbuffer instead)
    GLuint depthBufferRb;
    glGenRenderbuffers(1, &depthBufferRb);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferRb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_width, g_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferRb);

    GLenum attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // now the SSAO stuff
    GLuint ssaoFBO, ssaoBlurFBO;
    glGenFramebuffers(1, &ssaoFBO);  
    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    GLuint ssaoColorBuffer, ssaoColorBufferBlur;
    // SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g_width, g_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    // and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g_width, g_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // generate sample kernel
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    int kernelSize = 4;
    const auto lerp = [] (float a, float b, float f) {
        return a + f * (b - a);
    };
    for (unsigned int i = 0; i < kernelSize; ++i) {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);

        // scale exponentially
        float scale = float(i) / (float)kernelSize;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // generate noise texture
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }

    GLuint noiseTexture; 
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // now the VAO of the framebuffer (including binding textures)
    GLuint framebufferVao;
    glGenVertexArrays(1, &framebufferVao);
    glBindVertexArray(framebufferVao);

    static const GLfloat framebufferVboData[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
    };

    GLuint framebufferVbo;
    glGenBuffers(1, &framebufferVbo);
    glBindBuffer(GL_ARRAY_BUFFER, framebufferVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(framebufferVboData), framebufferVboData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);

    auto modelMat      = glm::mat4(1.0f);
    auto projectionMat = glm::perspective(glm::radians(45.0f), (float)g_width/(float)g_height, 0.1f, 100.f);

    auto gshader        = Shader("shaders/gshader.vert",  "shaders/gshader.frag");
    auto ssaoshader     = Shader("shaders/ppshader.vert", "shaders/ssaoshader.frag");
    auto ssaoblurshader = Shader("shaders/ppshader.vert", "shaders/ssaoblurshader.frag");
    auto ppShader       = Shader("shaders/ppshader.vert", "shaders/ppshader.frag" );

    auto lastTime = (float) glfwGetTime();
    
    glViewport(0,0, g_width, g_height);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glClearColor(0.1f,0.2f,0.0f,1.0f);

    while ((glfwGetKey(g_window, GLFW_KEY_ESCAPE) != GLFW_PRESS) 
            && !glfwWindowShouldClose(g_window)) {

        if (g_windowResized) {
            glViewport(0,0, g_width, g_height);
            projectionMat = glm::perspective(glm::radians(45.0f), (float)g_width/(float)g_height, 0.01f, 100.f);
            g_windowResized = false; 

            // recreate all textures and fbos
            glBindTexture(GL_TEXTURE_2D, gPositionTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_width, g_height, 0, GL_RGB, GL_FLOAT, nullptr);
            glBindTexture(GL_TEXTURE_2D, gNormalTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_width, g_height, 0, GL_RGB, GL_FLOAT, nullptr);
            glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_width, g_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g_width, g_height, 0, GL_RGB, GL_FLOAT, nullptr);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g_width, g_height, 0, GL_RGB, GL_FLOAT, nullptr);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBufferRb);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_width, g_height);
        }

        double xpos, ypos;
        glfwGetCursorPos(g_window, &xpos, &ypos);

        float deltaTime = (float)glfwGetTime() - lastTime;

        processInput(deltaTime, xpos, ypos);

        auto normalMat = glm::mat3(glm::mat4(1.0f)); 

        // 1. first, gbuffer fbo
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        gshader.use();
        glBindVertexArray(vao);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUniformMatrix4fv(2, 1, GL_FALSE, &modelMat[0][0]);
        glUniformMatrix4fv(3, 1, GL_FALSE, &g_viewMat[0][0]);
        glUniformMatrix4fv(4, 1, GL_FALSE, &projectionMat[0][0]);
        glUniformMatrix3fv(5, 1, GL_FALSE, &normalMat[0][0]);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);

        // 2. second, ssao
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glBindVertexArray(framebufferVao);
        glClear(GL_COLOR_BUFFER_BIT);
        ssaoshader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPositionTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormalTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glUniform1i(0, 0); 
        glUniform1i(1, 1);
        glUniform1i(2, 2);
        glUniformMatrix4fv(3, 1, GL_FALSE, &projectionMat[0][0]);
        glUniform1f(4, g_width);
        glUniform1f(5, g_height);
        glUniform1i(6, kernelSize);
        glUniform1f(7, g_radius);
        glUniform1f(8, g_bias);
        glUniform3fv(glGetUniformLocation(ssaoshader.id(), "samples"), kernelSize, &ssaoKernel[0][0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. third, ssao blur
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glBindVertexArray(framebufferVao);
        glClear(GL_COLOR_BUFFER_BIT);
        ssaoblurshader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
        glUniform1i(0, 0); 
        glUniform1i(1, g_shouldBlur ? 1 : 0); 
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // 4. fourth, lighting step
        ppShader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(framebufferVao);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPositionTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormalTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
        glUniform1i(0, 0); 
        glUniform1i(1, 1);
        glUniform1i(2, 2);
        glUniform1i(3, 3);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Settings");
        ImGui::Checkbox("Blur", &g_shouldBlur);
        ImGui::SliderFloat("Radius", &g_radius, 0.0f, 5.0f);
        ImGui::SliderFloat("Bias", &g_bias, 0.0f, 1.0f);
        ImGui::End();
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(g_window);

        glfwPollEvents(); 
        lastTime = (float)glfwGetTime();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(g_window);
    glfwTerminate();

    return 0;
}


