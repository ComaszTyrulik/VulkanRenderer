#pragma once
#include <vulkan/vulkan.hpp>
#include <VulkanRenderer/Vulkan/Shader.h>
#include <glfw/glfw3.h>
#include <optional>

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

        std::vector<uint32_t> List() const
        {
            return std::vector{graphicsFamily.value(), presentationFamily.value()};
        }
    };

	class Vulkan
    {
    public:
        Vulkan(std::string appName, GLFWwindow* window);
        ~Vulkan();

        void CreateInstance();
        void CreateSurface();
        void CreateLogicalDevice();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateRenderPass();
        void CreateGraphicsPipeline(const std::unique_ptr<Shader>& vertexShader, const std::unique_ptr<Shader>& fragmentShader);
        void CreateFramebuffers();
        void CreateCommandPool();
        void CreateCommandBuffers();
        void CreateSyncObjects();

        void DrawFrame();
        void WaitForDevice();

        std::unique_ptr<Shader> CreateShader(const std::string& shaderName, ShaderType type) const;

    private:
        /** Extensions, Layers and Debugging */
        std::vector<const char*> GetRequiredInstanceExtensions();
        void CheckIfValidationLayersAreAvailable();
        void SetUpDebugCallback();
        vk::DebugUtilsMessengerCreateInfoEXT GetDebugMessengerCreateInfo();
        /*************************************/

        /** Device and Swap chain */
        void ChoosePhysicalDeviceAndQueueFamilies();
        QueueFamilies GetDeviceQueueFamilies(const vk::PhysicalDevice& device);
        bool IsDeviceSuitable(const vk::PhysicalDevice& device, const QueueFamilies& deviceQueueFamilies);
        SwapChainSupportDetails GetSwapChainSupportDetails(const vk::PhysicalDevice& device);
        bool AreDeviceQueueFamiliesSupported(const QueueFamilies& deviceQueueFamilies);
        bool DoesDeviceSupportRequiredExtensions(const vk::PhysicalDevice& device);
        /**************************/

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        );

    private:
        std::string m_appName;
        GLFWwindow* m_window;
        std::size_t m_currentFrame = 0;

        /** Instance related */
        vk::PhysicalDevice m_physicalDevice;
        vk::UniqueInstance m_instance;
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        vk::DispatchLoaderDynamic m_dldi;
        vk::SurfaceKHR m_surface;

        /** Device related */
        QueueFamilies m_queueFamilies;
        vk::UniqueDevice m_logicalDevice;
        vk::Queue m_graphicsQueue;
        vk::Queue m_presentationQueue;

        /** SwapChain related */
        vk::SwapchainKHR m_swapChain;
        std::vector<vk::Image> m_swapChainImages;
        std::vector<vk::ImageView> m_swapChainImageViews;
        vk::Format m_swapChainImagesFormat;
        vk::Extent2D m_swapChainImagesExtent;
        std::vector<vk::Framebuffer> m_swapChainFramebuffers;

        /** Render pass related */
        vk::RenderPass m_renderPass;

        /** Graphics pipeline related */
        vk::Viewport m_viewport;
        vk::Rect2D m_scissors;
        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipelineLayout;

        /** Commands related */
        vk::CommandPool m_commandPool;
        std::vector<vk::CommandBuffer> m_commandBuffers;
        std::vector<vk::Semaphore> m_imageAvailableSemaphores;
        std::vector<vk::Semaphore> m_renderFinishedSemaphores;
        std::vector<vk::Fence> m_inFlightFences;
        std::vector<vk::Fence> m_imagesInFlight;
    };
} // namespace vr
