#include "VulkanRenderer/Vulkan/Device.h"
#include <spdlog/spdlog.h>
#include <set>

namespace vr
{
    Device::Device(const std::pair<vk::PhysicalDevice, QueueFamilies>& physicalDevice)
    {
        spdlog::info("DEVICE CREATION STARTED");
        {
            const std::vector<float> queuePriorities = {1.0f};

            const std::set<uint32_t> queueFamiliesIndices = {physicalDevice.second.graphicsFamilly.value(), physicalDevice.second.presentationFamilly.value()};
            std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
            for (const auto& famillyIndex : queueFamiliesIndices)
            {
                deviceQueueCreateInfos.push_back(vk::DeviceQueueCreateInfo({}, famillyIndex, queuePriorities));
            }

            const vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfos, {}, {});
            m_vkDevice = physicalDevice.first.createDeviceUnique(deviceCreateInfo);

            m_graphicsQueue = m_vkDevice->getQueue(physicalDevice.second.graphicsFamilly.value(), 0 /* Queue index */);
            m_presentationQueue = m_vkDevice->getQueue(physicalDevice.second.presentationFamilly.value(), 0 /* Queue index */);
        }
        spdlog::info("DEVICE CREATION ENDED\n");
    }
} // namespace vr
