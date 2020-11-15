//#include "shader.h"
//
//#include <glad/glad.h> 
//#include <GLFW/glfw3.h>
//
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//
//#include <iostream>
//
//#define DEBUG
//
//
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//#ifdef DEBUG
//    std::cout << "view resize to " << width << " " << height << std::endl;
//#endif // DEBUG
//
//    glViewport(0, 0, width, height);
//}
//
//void process_input(GLFWwindow* window)
//{
//    //process keyboard input
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, true);
//}
//
//int main()
//{
//    glfwInit();
//    //configure some parameters
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//not compatibility
//
//#ifdef MAC_OS
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif // MAC_OS
//
//
//    GLFWwindow* window = glfwCreateWindow(800, 600, "gl-demo-ui", NULL, NULL);
//    if (window == NULL)
//    {
//        std::cout << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//    glfwMakeContextCurrent(window);
//
//    //code to find gl function pointer
//    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//    {
//        std::cout << "Failed to initialize GLAD" << std::endl;
//        return -1;
//    }
//
//    glViewport(0, 0, 800, 600);
//    //register callback fun for view resize event
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//    while (!glfwWindowShouldClose(window))
//    {
//        process_input(window);
//
//        //render code 
//        glClearColor(0.0f, 0.8f, 0.8f, 1.0f); // set color for clear function
//        glClear(GL_COLOR_BUFFER_BIT); //just clear color buffer
//        //end render code
//
//        glfwPollEvents();  //check out events 
//        glfwSwapBuffers(window);
//    }
//
//    return 0;
//}