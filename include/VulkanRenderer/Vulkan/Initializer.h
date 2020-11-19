#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanRenderer/Vulkan/Vulkan.h"

namespace vr
{
    class VulkanInitializer
    {
    public:
        void ListAvailableVulkanExtensions();
        std::unique_ptr<Vulkan> Init(const std::string& appName, GLFWwindow* window);
    };
} // namespace vr