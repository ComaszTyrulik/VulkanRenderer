#pragma once
#include <vulkan/vulkan.hpp>
#include <VulkanRenderer/Vulkan/Shader.h>
#include <glfw/glfw3.h>
#include <optional>
#include <glm/glm.hpp>
#include "VulkanRenderer/Paths.h"

namespace vr
{
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static vk::VertexInputBindingDescription getBindingDescription()
        {
            vk::VertexInputBindingDescription bindingDescription;
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = vk::VertexInputRate::eVertex;

            return bindingDescription;
        }

        static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions;
            // Position
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            // Color
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            // UV
            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }
    };

    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

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
        const std::string MODEL_PATH = VK_GET_MODEL_PATH("viking_room.obj");
        const std::string TEXTURE_PATH = VK_GET_TEXTURE_PATH("viking_room.png");

        Vulkan(std::string appName, GLFWwindow* window);
        ~Vulkan();

        void CreateInstance();
        void CreateSurface();
        void CreateLogicalDevice();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateRenderPass();
        void CreateDescriptorSetLayout();
        void CreateGraphicsPipeline();
        void CreateCommandPool();
        void CreateColorResources();
        void CreateDepthResources();
        void CreateFramebuffers();
        void CreateTextureImage();
        void CreateTextureImageView();
        void CreateTextureSampler();
        void LoadModel();
        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void CreateUniformBuffers();
        void CreateDescriptorPool();
        void CreateDescriptorSets();
        void CreateCommandBuffers();
        void CreateSyncObjects();

        void DrawFrame();
        void UpdateUniformBuffer(uint32_t currentImage);
        void WaitForDevice();
        
        void CleanupSwapChain();
        void RecreateSwapChain();
        void ResizeFramebuffers();

    private:
        void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
        void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
        void CreateImage(
            uint32_t width,
            uint32_t height,
            vk::SampleCountFlagBits numSumples,
            vk::Format format,
            vk::ImageTiling tiling,
            vk::ImageUsageFlags usage,
            vk::MemoryPropertyFlags properties,
            vk::Image& image,
            vk::DeviceMemory& imageMemory
        );
        void TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
        void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

        vk::CommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);

        vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags);

        vk::Format FindDepthFormat();
        vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
        bool HasStencilComponent(vk::Format format);

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

        uint32_t FindMemoryType(uint32_t requiredType, vk::MemoryPropertyFlags requiredProperties);

        vk::SampleCountFlagBits GetMaxUsableSampleCount();

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
        bool m_shouldResizeFramebuffer = false;

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
        vk::DescriptorSetLayout m_descriptorSetLayout;
        vk::DescriptorPool m_descriptorPool;
        std::vector<vk::DescriptorSet> m_descriptorSets;
        vk::PipelineLayout m_pipelineLayout;

        /** Commands related */
        vk::CommandPool m_commandPool;
        std::vector<vk::CommandBuffer> m_commandBuffers;
        std::vector<vk::Semaphore> m_imageAvailableSemaphores;
        std::vector<vk::Semaphore> m_renderFinishedSemaphores;
        std::vector<vk::Fence> m_inFlightFences;
        std::vector<vk::Fence> m_imagesInFlight;

        std::vector<Vertex> m_vertices;
        std::vector<uint32_t> m_indices;

        UniformBufferObject m_mvpUBO;

        /** Buffers related */
        vk::Buffer m_vertexBuffer;
        vk::DeviceMemory m_vertexBufferMemory;
        vk::Buffer m_indexBuffer;
        vk::DeviceMemory m_indexBufferMemory;

        std::vector<vk::Buffer> m_uniformBuffers;
        std::vector<vk::DeviceMemory> m_uniformBuffersMemory;

        /** Textures related */
        vk::Image m_textureImage;
        vk::DeviceMemory m_textureImageMemory;
        vk::ImageView m_textureImageView;
        vk::Sampler m_textureSampler;

        vk::Image m_depthImage;
        vk::DeviceMemory m_depthImageMemory;
        vk::ImageView m_depthImageView;

        vk::SampleCountFlagBits m_msaaSamples = vk::SampleCountFlagBits::e1;
        vk::Image m_colorImage;
        vk::DeviceMemory m_colorImageMemory;
        vk::ImageView m_colorImageView;
    };
} // namespace vr
