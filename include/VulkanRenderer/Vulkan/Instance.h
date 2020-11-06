#pragma once
#include <vulkan/vulkan.hpp>
#include <VulkanRenderer/Vulkan/Device.h>
#include <glfw/glfw3.h>
#include <optional>

namespace vr
{
    class Instance
    {
    public:
        Instance(const std::string& appName);
        ~Instance();

        void CreateSurface(GLFWwindow* window);
        std::unique_ptr<Device> CreateDevice(GLFWwindow* window);

    private:
        void CreateInstance(const std::string& appName);

        /** Extensions, Layers and Debugging */
        std::vector<const char*> GetRequiredExtensions();
        void CheckIfValidationLayersAreAvailable();
        void SetUpDebugCallback();
        vk::DebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();
        /*************************************/

        /** Device and Swap chain */
        std::pair<vk::PhysicalDevice, QueueFamilies> GetValidPhysicalDevice();
        QueueFamilies GetDeviceQueueFamilies(const vk::PhysicalDevice& device);
        bool IsDeviceSuitable(const vk::PhysicalDevice& device, const QueueFamilies& deviceQueueFamilies);
        SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice& device);
        bool AreDeviceQueueFamiliesSupported(const QueueFamilies& deviceQueueFamilies);
        bool DoesDeviceSupportRequiredExtensions(const vk::PhysicalDevice& device);
        /**************************/

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

    private:
        vk::UniqueInstance m_instance;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        vk::DispatchLoaderDynamic m_dldi;
        vk::SurfaceKHR m_surface;
    };
} // namespace vr