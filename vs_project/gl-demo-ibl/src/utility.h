#ifndef _UTILITY_H
#define _UTILITY_H

#include "shader.h"

#include <vector>
#include <string>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

unsigned int create_skybox_VAO();
unsigned int create_sphere_VAO();

unsigned int load_cubemap(std::vector<std::string> faces);
bool init_gui(GLFWwindow*& window, unsigned int width = 1920, unsigned int height = 1080);


std::string getPath(const std::string& path);

unsigned int hdr2cubemap(std::string);
unsigned int irrandiancemap(unsigned int cubemap);
unsigned int filtered_color(unsigned int cubemap, unsigned int roughness_range = 5);
unsigned int brdf_LUT(bool save_to_file = false);


#endif // !_UTILITY_H
