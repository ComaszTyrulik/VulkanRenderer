#include "VulkanRenderer/Vulkan/Device.h"
#include <spdlog/spdlog.h>
#include <set>

namespace vr
{
    Device::Device(const std::pair<vk::PhysicalDevice, QueueFamilies>& physicalDevice, const std::vector<const char*>& requiredExtensions)
    {
        m_queueFamilies = physicalDevice.second;
        spdlog::info("DEVICE CREATION STARTED");
        {
            const std::vector<float> queuePriorities = {1.0f};

            const std::set<uint32_t> queueFamiliesIndices = {m_queueFamilies.graphicsFamily.value(), m_queueFamilies.presentationFamily.value()};
            std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
            for (const auto& famillyIndex : queueFamiliesIndices)
            {
                deviceQueueCreateInfos.push_back(vk::DeviceQueueCreateInfo({}, famillyIndex, queuePriorities));
            }

            const vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfos, {}, requiredExtensions);
            m_vkDevice = physicalDevice.first.createDeviceUnique(deviceCreateInfo);

            m_graphicsQueue = m_vkDevice->getQueue(m_queueFamilies.graphicsFamily.value(), 0 /* Queue index */);
            m_presentationQueue = m_vkDevice->getQueue(m_queueFamilies.presentationFamily.value(), 0 /* Queue index */);

        }
        spdlog::info("DEVICE CREATION ENDED\n");
    }
    
    void Device::CreateSwapChain(const SwapChainSupportDetails& swapChainDetails, GLFWwindow* window, const vk::SurfaceKHR& surface)
    {
        spdlog::info("SWAP CHAIN CREATION STARTED");
        {
            const auto surfaceFormat = GetSwapSurfaceFormat(swapChainDetails.formats);
            const auto presentMode = GetSwapPresentMode(swapChainDetails.presentModes);
            const auto swapExtent = ChooseSwapExtent(swapChainDetails.capabilities, window);

            // How many image will be created in the swap chain (something like how many render targets will be available?)
            uint32_t swapChainImageCount = std::min(swapChainDetails.capabilities.minImageCount + 1, swapChainDetails.capabilities.maxImageCount);

            vk::SwapchainCreateInfoKHR swapChainCreateInfo({}, surface, swapChainImageCount, surfaceFormat.format, surfaceFormat.colorSpace, swapExtent);
            swapChainCreateInfo.setImageArrayLayers(1);
            swapChainCreateInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
            swapChainCreateInfo.setPreTransform(swapChainDetails.capabilities.currentTransform);
            swapChainCreateInfo.setPresentMode(presentMode);
            swapChainCreateInfo.setClipped(VK_TRUE);

            const auto queueFamilyIndices = m_queueFamilies.List();
            if (m_queueFamilies.graphicsFamily != m_queueFamilies.presentationFamily)
            {
                // If two queues are from different families, we want to use concurrent mode, to avoid explicit ownership transfering.
                // However, probably in a need of very performent code, we might want to always use VK_SHARING_MODE_EXCLUSIVE
                // because it's the most perfoment one
                swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
                swapChainCreateInfo.setQueueFamilyIndexCount(queueFamilyIndices.size());
                swapChainCreateInfo.setQueueFamilyIndices(queueFamilyIndices);
            }
            else
            {
                swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
            }

            m_swapChain = m_vkDevice->createSwapchainKHRUnique(swapChainCreateInfo);
            m_swapChainImages = m_vkDevice->getSwapchainImagesKHR(m_swapChain.get());

            m_swapChainImagesFormat = surfaceFormat.format;
            m_swapChainImagesExtent = swapExtent;
        }
        spdlog::info("SWAP CHAIN CREATION ENDED\n");
    }
    
    vk::SurfaceFormatKHR Device::GetSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == vk::Format::eR8G8B8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }
    
    vk::PresentModeKHR Device::GetSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            // Choose tripple buffering mode if available
            if (availablePresentMode == vk::PresentModeKHR::eMailbox)
            {
                return availablePresentMode;
            }
        }

        // V-SYNC like
        return vk::PresentModeKHR::eFifo;
    }
    
    vk::Extent2D Device::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
    {
        // If extent is not MAX INT, then it must remain as the surface's current extent is.
        // Otherwise, the extent will be automatically set so that it fits the window's surface resolution
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Actual extent cannot be bigger nor smaller than the available extent values
        vk::Extent2D actualExtent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
} // namespace vr
