#pragma once
#include <vector>
#include <string>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

unsigned int create_skybox_VAO();
unsigned int create_sphere_VAO();

unsigned int load_cubemap(std::vector<std::string> faces);
bool init_gui(GLFWwindow*& window, unsigned int width = 1920, unsigned int height = 1080);

std::string getPath(const std::string& path);

