#pragma once

#define VK_SOURCE_DIRECTORY "C:/Users/Tomek/Documents/Projects/C++/VulkanRenderer"
#define VK_RESOURCES_DIRECTORY "C:/Users/Tomek/Documents/Projects/C++/VulkanRenderer/res"
#define VK_SHADERS_DIRECTORY std::string(VK_RESOURCES_DIRECTORY) + std::string("/Shaders/")

#define VK_GET_SHADER_PATH(shaderFilename) VK_SHADERS_DIRECTORY + "/" + shaderFilename
