#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>

namespace vr
{
    struct QueueFamilies
    {
        std::optional<uint32_t> graphicsFamilly;
        std::optional<uint32_t> presentationFamilly;
    };

	class Device
    {
    public:
        Device(const std::pair<vk::PhysicalDevice, QueueFamilies>& physicalDevice);

    private:
        vk::UniqueDevice m_vkDevice;
        vk::Queue m_graphicsQueue;
        vk::Queue m_presentationQueue;
    };
} // namespace vr
