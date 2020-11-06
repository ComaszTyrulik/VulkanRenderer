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
        std::unique_ptr<Device> CreateDevice();

    private:
        void CreateInstance(const std::string& appName);

        std::vector<const char*> GetRequiredExtensions();
        void CheckIfValidationLayersAreAvailable();
        void SetUpDebugCallback();
        vk::DebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();

        std::pair<vk::PhysicalDevice, QueueFamilies> GetValidPhysicalDevice();
        QueueFamilies GetDeviceQueueFamilies(const vk::PhysicalDevice& device);
        bool AreDeviceQueueFamiliesSupported(const QueueFamilies& deviceQueueFamilies);

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