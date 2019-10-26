#include "lgl/Base.h"
#include <cmath>
#include <cstdlib>

float vertices[] = {
    // positions          // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

glm::vec3 cubePositions[] = {
  glm::vec3( 0.0f,  0.0f,  0.0f),
  glm::vec3( 2.0f,  5.0f, -15.0f),
  glm::vec3(-1.5f, -2.2f, -2.5f),
  glm::vec3(-3.8f, -2.0f, -12.3f),
  glm::vec3( 2.4f, -0.4f, -3.5f),
  glm::vec3(-1.7f,  3.0f, -7.5f),
  glm::vec3( 1.3f, -2.0f, -2.5f),
  glm::vec3( 1.5f,  2.0f, -2.5f),
  glm::vec3( 1.5f,  0.2f, -1.5f),
  glm::vec3(-1.3f,  1.0f, -1.5f)
};

unsigned int indices[] = {
    0, 1, 2,                // first triangle (right)
    0, 2, 3                 // second triangle (left)
};

class Demo : public lgl::App
{
public:
    Demo() : App()
    {
    }

    void UserSetup() override {
        // create shader program and build it immediately
        int result = basicShader.Build("data/tex2.vert", "data/multitex.frag");
        LGL_ERROR_QUIT(result, "Error creating basic shader");

        // load texture
        containerTexture = lgl::util::LoadTexture("data/container.jpg");
        if (lgl::error::AnyGLError() != 0) { lgl::error::ErrorExit("Error loading data/container.jpg"); }
        // modify its texture filtering
        glBindTexture(GL_TEXTURE_2D, containerTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        awesomefaceTexture = lgl::util::LoadTexture("data/awesomeface.png");
        if (lgl::error::AnyGLError() != 0) { lgl::error::ErrorExit("Error loading data/awesomeface.png"); }

        // wrap vertex attrib configurations via VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
            // prepare vertex data
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            // positions
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));  // or 3*sizeof(float) for its stride parameter
            glEnableVertexAttribArray(0);
            // texture coords
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));  // start after 3 floats
            glEnableVertexAttribArray(1);

            // bind texture
            glBindTexture(GL_TEXTURE_2D, containerTexture);
        glBindVertexArray(0);

        // once preparation
        // tell opengl which texture sampler map to whichs texture object
        basicShader.Use();
        glActiveTexture(GL_TEXTURE0);
        basicShader.SetUniform(basicShader.GetUniformLocation("textureSampler"), 0);
        glActiveTexture(GL_TEXTURE1);
        basicShader.SetUniform(basicShader.GetUniformLocation("textureSampler2"), 1);
        // set default uniform values
        basicShader.SetUniform(basicShader.GetUniformLocation("mixFactor"), mixFactor);

        // set view and projection matrix to shader
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        glUniformMatrix4fv(basicShader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 projection = glm::perspective(glm::radians(camFov), SCREEN_WIDTH * 1.0f / SCREEN_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(basicShader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // enable depth testing
        glEnable(GL_DEPTH_TEST); 
    }

    void UserProcessKeyInput(double deltaTime) override {
        // poll for key input
        // blend more color from awesomeface
        GLFWwindow* window = GetGLFWWindow();
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            mixFactor += 0.01f;
            if (mixFactor > 1.0f)
            {
                mixFactor = 1.0f;
            }
            basicShader.SetUniform("mixFactor", mixFactor);
        }
        // blend more color from container
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            mixFactor -= 0.01f;
            if (mixFactor < 0.0f)
            {
                mixFactor = 0.0f;
            }
            basicShader.SetUniform("mixFactor", mixFactor);
        }

        // camera movement
        const float kCamSpeed = 2.5f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camPos += kCamSpeed * camFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camPos -= kCamSpeed * camFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camPos += kCamSpeed * glm::normalize(glm::cross(camUp, camFront));
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camPos -= kCamSpeed * glm::normalize(glm::cross(camUp, camFront));
    }

    void UserProcessMousePos(float xOffset, float yOffset) override
    {
        camYaw = std::fmod(camYaw + xOffset, 360.0f);
        camPitch += yOffset;

        if (camPitch > 89.0f)
            camPitch = 89.0f;
        if (camPitch < -89.0f)
            camPitch = -89.0f;

        // compute front vector
        // suitable for FPS-style camera
        glm::vec3 front = glm::vec3(
            std::cos(glm::radians(camYaw)) * std::cos(glm::radians(camPitch)),
            std::sin(glm::radians(camPitch)),
            std::sin(glm::radians(camYaw)) * std::cos(glm::radians(camPitch))
        );
        camFront = glm::normalize(front);
    }

    void UserProcessMouseScrollInput(float xOffset, float yOffset) override
    {
        std::cout << yOffset << std::endl;
        if (camFov >= 1.0f && camFov <= 45.0f)
            camFov -= yOffset;
        if (camFov < 1.0f)
            camFov = 1.0f;
        if (camFov > 45.0f)
            camFov = 45.0f;

        glm::mat4 projection = glm::perspective(glm::radians(camFov), SCREEN_WIDTH * 1.0f / SCREEN_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(basicShader.GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }

    void UserShutdown() override {
        // disable vertex attributes
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        // delete all VBOs
        glDeleteBuffers(1, &VBO);
        VBO = -1;
        // reset texture binding to default texture
        glBindTexture(GL_TEXTURE_2D, 0);
        // delete all textures
        glDeleteTextures(1, &containerTexture);
        containerTexture = -1;
        // delete shader program
        basicShader.Destroy();
        // delete all VAOs
        glDeleteVertexArrays(1, &VAO);
        VAO = -1;
    }

    void UserRender() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        basicShader.Use();
        // bind texture 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, containerTexture);
        // bind texture 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, awesomefaceTexture);

        glBindVertexArray(VAO);

        glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        glUniformMatrix4fv(basicShader.GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));

        for (std::size_t i=0; i<10; ++i)
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePositions[i]);

            float angle = 35.0f * i + 20.0f;
            if (i % 3 == 0)
                model = glm::rotate(model, static_cast<float>(glfwGetTime()) * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            else
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(basicShader.GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindVertexArray(0);
    }

    void UserUpdate(const double delta) override
    {
    }

private:
    lgl::Shader basicShader;
    GLuint containerTexture, awesomefaceTexture;
    GLfloat mixFactor = 0.2f;
    GLuint VBO;
    GLuint VAO;
    
    glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // camera looks into -z axis, the following values make sure initial values and conditions are met
    float camYaw = -90.0f, camPitch = 0.0f;
    float camFov = 45.0f;
};

int main()
{
    Demo app;

    lgl::AppConfigs configs;
    configs.MousePosEnabled = true;
    configs.RelativeMouseCursor = true;
    configs.MouseScrollEnabled = true;

    app.Setup("Camera", configs);
    app.Start();
    return 0;
}
