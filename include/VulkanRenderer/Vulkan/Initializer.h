#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanRenderer/Vulkan/Instance.h"

namespace vr
{
    class VulkanInitializer
    {
    public:
        void ListAvailableVulkanExtensions();
        std::unique_ptr<Instance> CreateInstance(const std::string& appName);
    };
} // namespace vr