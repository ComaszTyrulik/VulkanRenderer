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
    
    std::unique_ptr<Instance> VulkanInitializer::CreateInstance(const std::string& appName)
    {
        return std::make_unique<Instance>(appName);
    }
} // namespace vr