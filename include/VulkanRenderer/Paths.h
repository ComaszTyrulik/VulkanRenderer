#pragma once

#ifndef VK_SOURCE_DIRECTORY
    #define VK_SOURCE_DIRECTORY "C:/Users/Tomek/Documents/Projects/C++/VulkanRenderer"
#endif

#ifndef VK_RESOURCES_DIRECTORY
    #define VK_RESOURCES_DIRECTORY "C:/Users/Tomek/Documents/Projects/C++/VulkanRenderer/res"
#endif

#ifndef VK_SHADERS_DIRECTORY
    #define VK_SHADERS_DIRECTORY std::string(VK_RESOURCES_DIRECTORY) + std::string("/Shaders/")
#endif

#ifndef VK_TEXTURES_DIRECTORY
    #define VK_TEXTURES_DIRECTORY std::string(VK_RESOURCES_DIRECTORY) + std::string("/Textures/")
#endif

#ifndef VK_MODELS_DIRECTORY
    #define VK_MODELS_DIRECTORY std::string(VK_RESOURCES_DIRECTORY) + std::string("/Models/")
#endif

#ifndef VK_GET_SHADER_PATH
    #define VK_GET_SHADER_PATH(shaderFilename) VK_SHADERS_DIRECTORY + shaderFilename
#endif

#ifndef VK_GET_TEXTURE_PATH
    #define VK_GET_TEXTURE_PATH(textureFilename) VK_TEXTURES_DIRECTORY + textureFilename
#endif

#ifndef VK_GET_MODEL_PATH
    #define VK_GET_MODEL_PATH(modelFilename) VK_MODELS_DIRECTORY + modelFilename
#endif
