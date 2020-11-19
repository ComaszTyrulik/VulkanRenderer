#include "VulkanRenderer/Vulkan/Initializer.h"

#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>
#include <fmt/format.h>

namespace vr
{
    void vr::VulkanInitializer::ListAvailableVulkanExtensions()
    {
        spdlog::info("Available vulkan extensions:");

        const auto availableExtensions = vk::enumerateInstanceExtensionProperties();
        for (const auto& extension : availableExtensions)
        {
            spdlog::info("Extension name: {}", extension.extensionName);
        }
    }
    
    std::unique_ptr<Vulkan> VulkanInitializer::Init(const std::string& appName, GLFWwindow* window)
    {
        return std::make_unique<Vulkan>(appName, window);
    }
} // namespace vr