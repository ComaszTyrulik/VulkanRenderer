#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>
#include <glfw/glfw3.h>

namespace vr
{
    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct QueueFamilies
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentationFamily;

        std::vector<uint32_t> List()
        {
            return std::vector{graphicsFamily.value(), presentationFamily.value()};
        }
    };

    /** This represents D3D11::DeviceContext */
	class Device
    {
        friend class Instance;

    public:
        Device(const std::pair<vk::PhysicalDevice, QueueFamilies>& physicalDevice, const std::vector<const char*>& requiredExtensions);

    private:
        void CreateSwapChain(const SwapChainSupportDetails& swapChainDetails, GLFWwindow* window, const vk::SurfaceKHR& surface);

        // Choose format of the swap chain images. R8G8B8A8 and wheter it should be a linear od non-linear color space
        vk::SurfaceFormatKHR GetSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

        // Choose how swap chain will present upcomming images to the window - v-sync stuff
        vk::PresentModeKHR GetSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

        // Choose the swap chain's images' resolution o_O
        vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    private:
        QueueFamilies m_queueFamilies;
        vk::UniqueDevice m_vkDevice;
        vk::Queue m_graphicsQueue;
        vk::Queue m_presentationQueue;
        vk::UniqueSwapchainKHR m_swapChain;

        std::vector<vk::Image> m_swapChainImages;
        vk::Format m_swapChainImagesFormat;
        vk::Extent2D m_swapChainImagesExtent;
    };
} // namespace vr
