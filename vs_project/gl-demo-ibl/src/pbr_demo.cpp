#include "model.h"
#include "utility.h"
#include "shader.h"
#include "camera.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <algorithm>
#include <functional>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(-6.0f, -6.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
static bool capture_cursor = false;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f); 
static unsigned int cubemap_texture;
static unsigned int irradiancemap_texture;
static unsigned int prefiltered_color_texture;
static unsigned int brdf_LUT_texture;

inline static void draw_skybox(Shader& skybox_shader)
{
    static unsigned int skybox_VAO = create_skybox_VAO();
    static vector<std::string> faces
    {
        getPath("resources/skybox/right.jpg"),
        getPath("resources/skybox/left.jpg"),
        getPath("resources/skybox/top.jpg"),
        getPath("resources/skybox/bottom.jpg"),
        getPath("resources/skybox/front.jpg"),
        getPath("resources/skybox/back.jpg")
    };
    //cubemap_texture = load_cubemap(faces);
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(skybox_VAO);
    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, irrandiancemap_texture); //·øÕÕ¶È ÌùÍ¼
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

inline static void draw_pbr_model(Shader& pbr_shader, glm::mat4 model)
{
    static unsigned int sphere_VAO = create_sphere_VAO();
    
    //pbr_shader.use();
    glBindVertexArray(sphere_VAO);
    glDrawElements(GL_TRIANGLE_STRIP, 8320, GL_UNSIGNED_INT, 0);
}

glm::vec3 light_positions[] = {
    glm::vec3(-10.0f,  10.0f, 10.0f),
    glm::vec3(10.0f,  10.0f, 10.0f),
    glm::vec3(-10.0f, -10.0f, 10.0f),
    glm::vec3(10.0f, -10.0f, 10.0f),
};
glm::vec3 light_colors[] = {
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f)
};

int main()
{
    GLFWwindow* window;
    if (!init_gui(window)) return -1;
    const char* glsl_version = "#version 130";

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //imgui for gui input
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    //model or vao
    //Model nanosuit_model(getPath("resources/nanosuit/nanosuit.obj"));
    //Model mit_sphere_model(getPath("resources/sphere/sphere.obj"));

    // build and compile shaders
    //Shader pbr_shader(getPath("shader/model_loading.vs"), getPath("shader/model_loading.fs"));
    Shader pbr_shader(getPath("shader/pbr_demo.vs"), getPath("shader/pbr_demo.fs"));
    Shader skybox_shader(getPath("shader/skybox.vs"), getPath("shader/skybox.fs"));

    cubemap_texture = hdr2cubemap(getPath("resources/newport_loft.hdr"));
    prefiltered_color_texture = filtered_color(cubemap_texture);
    irradiancemap_texture = irrandiancemap(cubemap_texture);
    brdf_LUT_texture = brdf_LUT();
    glViewport(0, 0, 1920, 1080);
    // render loop
    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static glm::vec3 albedo = glm::vec3(0.5, 0.0, 0.0);
        static float metallic = 0.0f;
        static float roughness = 0.05f;
        static float ao = 1.0f;

        {
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            //ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("metallic", &metallic, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::SliderFloat("roughness", &roughness, 0.05f, 1.0f);
            ImGui::SliderFloat("ao", &ao, 0.0f, 1.0f);
            ImGui::InputFloat3("albedo", (float*)&albedo); // Edit 3 floats representing a color

            //if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            //    counter++;
            //ImGui::SameLine();
            //ImGui::Text("counter = %d", counter);
            ImGui::Text("push \'k\' to move camera");

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        ImGui::Render();

        // render
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        pbr_shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        pbr_shader.set_mat4("projection", projection);
        pbr_shader.set_mat4("view", view);
        pbr_shader.set_vec3("camera_pos", camera.Position);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        pbr_shader.set_mat4("model", model);

        pbr_shader.set_int("irradianceMap", 0);
        pbr_shader.set_int("prefilterMap", 1);
        pbr_shader.set_int("brdfLUT", 2);

        pbr_shader.set_float("metallic", metallic);
        pbr_shader.set_float("roughness", roughness);
        pbr_shader.set_float("ao", ao);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradiancemap_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefiltered_color_texture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdf_LUT_texture);
        //albedo = glm::vec3(im_albedo.x, im_albedo.y, im_albedo.z);

        //light source
        pbr_shader.set_vec3("albedo", albedo);
        for (unsigned int i = 0; i < sizeof(light_positions) / sizeof(light_positions[0]); ++i)
        {
            glm::vec3 newPos = light_positions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            newPos = light_positions[i];
            pbr_shader.set_vec3("light_positions[" + std::to_string(i) + "]", newPos);
            pbr_shader.set_vec3("light_colors[" + std::to_string(i) + "]", light_colors[i]);
        }
        draw_pbr_model(pbr_shader, model);

        //model = glm::translate(model, glm::vec3(-3.0f, 0.0f, 0.0f));
        //pbr_shader.set_mat4("model", model);
        //draw_pbr_model(pbr_shader, model);

        for (int i = 0; i <= 5; i++)
        {
            float metallic_tmp = i * (1.0 / 5.0);
            pbr_shader.set_float("metallic", metallic_tmp);
            glm::mat4 model_x = glm::translate(model, glm::vec3(-3.0f * i, 0.0f, 0.0f));
            for (int j = 0; j <= 5; j++)
            {
                glm::mat4 model_y = glm::translate(model_x, glm::vec3(0.0f, -3.0f * j, 0.0f));
                float roughness_tmp = std::min(j * (1.0f / 5.0f) + 0.05f, 1.0f);
                pbr_shader.set_float("roughness", roughness_tmp);
                pbr_shader.set_mat4("model", model_y);
                draw_pbr_model(pbr_shader, model_y);
            }
        }

        //draw skybox
        skybox_shader.use();
        skybox_shader.set_int("skybox", 0);
        skybox_shader.set_mat4("projection", projection);
        view = glm::mat4(glm::mat3(view));
        skybox_shader.set_mat4("view", view);
        skybox_shader.set_mat4("model", model);
        skybox_shader.set_mat4("projection", projection);
        draw_skybox(skybox_shader);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        // tell GLFW to capture our mouse
        if (capture_cursor)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        capture_cursor = !capture_cursor;
    }

    if (!capture_cursor)
        return;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!capture_cursor)
        return;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

