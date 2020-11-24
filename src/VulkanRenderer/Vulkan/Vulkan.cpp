#include "VulkanRenderer/Vulkan/Vulkan.h"
#include "VulkanRenderer/Paths.h"

#include <spdlog/spdlog.h>
#include <glfw/glfw3.h>
#include <set>

#ifndef GLM_FORCE_RADIANS
    #define GLM_FORCE_RADIANS
#endif
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <VulkanRenderer/Vendors/tiny_obj_loader.h>

namespace vr
{
#ifndef NDEBUG
    const bool VR_ENABLE_VALIDATION_LAYERS = true;
    static const std::vector<const char*> VR_VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
#else
    const bool VR_ENABLE_VALIDATION_LAYERS = false;
    static const std::vector<const char*> VR_VALIDATION_LAYERS = {};
#endif

    static const std::vector<const char*> VR_REQUIRED_DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME /* Required to be able to set viewport's height to negative value, in order to get NDC with +y facing upwards*/
    };

    static const int MAX_FRAMES_IN_FLIGHT = 2;

    Vulkan::Vulkan(std::string appName, GLFWwindow* window)
        : m_appName(std::move(appName)), m_window(window)
    {
        if (VR_ENABLE_VALIDATION_LAYERS)
        {
            CheckIfValidationLayersAreAvailable();
        }

        /**
         * Apparentlly, this step is required. Otherwise, there will be the access violation when initializing DynamicLoader with vk::Vulkan,
         * because Vulkan won't be able to find 'vkGetInstanceProcAddr' function
         **/
        vk::DynamicLoader dl;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        m_dldi.init(vkGetInstanceProcAddr);
    }

    void vr::Vulkan::CreateInstance()
    {
        spdlog::info("INSTANCE CREATION STARTED");
        {
            vk::ApplicationInfo appInfo = vk::ApplicationInfo(m_appName.c_str(), VK_MAKE_VERSION(1, 0, 0));
            appInfo.setApiVersion(VK_API_VERSION_1_2);

            const auto extensions = GetRequiredInstanceExtensions();
            vk::InstanceCreateInfo instanceInfo({}, &appInfo, VR_VALIDATION_LAYERS, extensions);
            vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
            if (VR_ENABLE_VALIDATION_LAYERS)
            {
                debugMessengerCreateInfo = GetDebugMessengerCreateInfo();
                instanceInfo.setPNext(&debugMessengerCreateInfo);
            }

            m_instance = vk::createInstanceUnique(instanceInfo);
            m_dldi.init(m_instance.get());
            SetUpDebugCallback();
        }
        spdlog::info("INSTANCE CREATION ENDED\n");
    }

    void Vulkan::CreateSurface()
    {
        spdlog::info("SURFACE CREATION STARTED");
        {
            if (glfwCreateWindowSurface(m_instance.get(), m_window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_surface)) != VK_SUCCESS)
            {
                throw std::runtime_error("Could not create window surface!");
            }
        }
        spdlog::info("SURFACE CREATION ENDED\n");
    }

    void Vulkan::CreateLogicalDevice()
    {
        spdlog::info("DEVICE CREATION STARTED");
        {
            spdlog::info("CHOSING PHYSICAL DEVICE AND QUEUE FAMILIES STARTED");
            {
                ChoosePhysicalDeviceAndQueueFamilies();
            }
            spdlog::info("CHOSING PHYSICAL DEVICE AND QUEUE FAMILIES ENDED\n");

            const std::vector<float> queuePriorities = {1.0f};
            const std::set<uint32_t> queueFamiliesIndices = {m_queueFamilies.graphicsFamily.value(), m_queueFamilies.presentationFamily.value()};
            std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
            for (const auto& famillyIndex : queueFamiliesIndices)
            {
                deviceQueueCreateInfos.push_back(vk::DeviceQueueCreateInfo({}, famillyIndex, queuePriorities));
            }

            vk::PhysicalDeviceFeatures deviceFeatures;
            deviceFeatures.setSamplerAnisotropy(VK_TRUE);

            const vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfos, {} /*This should be empty*/, VR_REQUIRED_DEVICE_EXTENSIONS, &deviceFeatures);
            m_logicalDevice = m_physicalDevice.createDeviceUnique(deviceCreateInfo);

            m_graphicsQueue = m_logicalDevice->getQueue(m_queueFamilies.graphicsFamily.value(), 0 /* Queue index */);
            m_presentationQueue = m_logicalDevice->getQueue(m_queueFamilies.presentationFamily.value(), 0 /* Queue index */);
        }
        spdlog::info("DEVICE CREATION ENDED\n");
    }

    void Vulkan::CreateSwapChain()
    {
        spdlog::info("SWAP CHAIN CREATION STARTED");
        {
            const auto swapChainSupportDetails = GetSwapChainSupportDetails(m_physicalDevice);
            const auto& swapChainCapabilities = swapChainSupportDetails.capabilities;

            /** Get swap chain surface format */
            auto surfaceFormat = swapChainSupportDetails.formats[0];
            {
                for (const auto& availableFormat : swapChainSupportDetails.formats)
                {
                    if (availableFormat.format == vk::Format::eR8G8B8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                    {
                        surfaceFormat = availableFormat;
                        break;
                    }
                }
            }

            /** Get swap chain present mode */
            auto presentMode = vk::PresentModeKHR::eFifo; // V-SYNC like
            {
                for (const auto& availablePresentMode : swapChainSupportDetails.presentModes)
                {
                    // Choose tripple buffering mode if available
                    if (availablePresentMode == vk::PresentModeKHR::eMailbox)
                    {
                        presentMode = availablePresentMode;
                        break;
                    }
                }
            }

            /** Get swap chain extent */
            {
                // If extent is not MAX INT, then it must remain as the surface's current extent is.
                // Otherwise, the extent will be automatically set so that it fits the window's surface resolution
                if (swapChainCapabilities.currentExtent.width != UINT32_MAX)
                {
                    m_swapChainImagesExtent = swapChainCapabilities.currentExtent;
                }
                else
                {
                    int width, height;
                    glfwGetFramebufferSize(m_window, &width, &height);

                    // Actual extent cannot be bigger nor smaller than the available extent values
                    vk::Extent2D actualExtent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
                    actualExtent.width = std::clamp(actualExtent.width, swapChainCapabilities.minImageExtent.width, swapChainCapabilities.maxImageExtent.width);
                    actualExtent.height = std::clamp(actualExtent.height, swapChainCapabilities.minImageExtent.height, swapChainCapabilities.maxImageExtent.height);

                    m_swapChainImagesExtent = actualExtent;
                }
            }

            // How many images will be created in the swap chain (something like how many render targets will be available?)
            uint32_t swapChainImageCount = std::min(swapChainCapabilities.minImageCount + 1, swapChainCapabilities.maxImageCount);

            vk::SwapchainCreateInfoKHR swapChainCreateInfo({}, m_surface, swapChainImageCount, surfaceFormat.format, surfaceFormat.colorSpace, m_swapChainImagesExtent);
            swapChainCreateInfo.setImageArrayLayers(1);
            swapChainCreateInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
            swapChainCreateInfo.setPreTransform(swapChainCapabilities.currentTransform);
            swapChainCreateInfo.setPresentMode(presentMode);
            swapChainCreateInfo.setClipped(VK_TRUE);

            const auto queueFamilyIndices = m_queueFamilies.List();
            if (m_queueFamilies.graphicsFamily != m_queueFamilies.presentationFamily)
            {
                // If two queues are from different families, we want to use concurrent mode to avoid explicit ownership transfering.
                // However, probably in a need of very performant code, we might want to always use VK_SHARING_MODE_EXCLUSIVE
                // because it's the most performant one
                swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
                swapChainCreateInfo.setQueueFamilyIndexCount(queueFamilyIndices.size());
                swapChainCreateInfo.setQueueFamilyIndices(queueFamilyIndices);
            }
            else
            {
                swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
            }

            m_swapChain = m_logicalDevice->createSwapchainKHR(swapChainCreateInfo);
            m_swapChainImages = m_logicalDevice->getSwapchainImagesKHR(m_swapChain);

            m_swapChainImagesFormat = surfaceFormat.format;
        }
        spdlog::info("SWAP CHAIN CREATION ENDED\n");
    }

    void Vulkan::CreateImageViews()
    {
        spdlog::info("IMAGE VIEWS CREATION STARTED");
        {
            for (const auto& image : m_swapChainImages)
            {
                m_swapChainImageViews.push_back(CreateImageView(image, m_swapChainImagesFormat, vk::ImageAspectFlagBits::eColor));
            }
        }
        spdlog::info("IMAGE VIEWS CREATION ENDED\n");
    }

    void Vulkan::CreateRenderPass()
    {
        spdlog::info("RENDER PASS CREATION STARTED");
        {
            vk::AttachmentDescription depthAttachment;
            depthAttachment.format = FindDepthFormat();
            depthAttachment.samples = m_msaaSamples;
            depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
            depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
            depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            vk::AttachmentReference depthAttachmentReference(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

            vk::AttachmentDescription colorAttachment({}, m_swapChainImagesFormat, m_msaaSamples, vk::AttachmentLoadOp::eClear);
            colorAttachment
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

            vk::AttachmentReference colorAttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);

            vk::AttachmentDescription colorAttachmentResolve;
            colorAttachmentResolve.format = m_swapChainImagesFormat;
            colorAttachmentResolve.samples = vk::SampleCountFlagBits::e1;
            colorAttachmentResolve.loadOp = vk::AttachmentLoadOp::eDontCare;
            colorAttachmentResolve.storeOp = vk::AttachmentStoreOp::eStore;
            colorAttachmentResolve.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            colorAttachmentResolve.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            colorAttachmentResolve.initialLayout = vk::ImageLayout::eUndefined;
            colorAttachmentResolve.finalLayout = vk::ImageLayout::ePresentSrcKHR;
            
            vk::AttachmentReference colorAttachmentResolveRef;
            colorAttachmentResolveRef.attachment = 2;
            colorAttachmentResolveRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

            vk::SubpassDescription subpassDescription;
            subpassDescription
                .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachments(colorAttachmentReference)
                .setPDepthStencilAttachment(&depthAttachmentReference)
                .setResolveAttachments(colorAttachmentResolveRef);

            vk::SubpassDependency subpassDependency;
            subpassDependency
                .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
                .setSrcAccessMask(vk::AccessFlags())
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

            std::array<vk::AttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
            vk::RenderPassCreateInfo renderPassCreateInfo;
            renderPassCreateInfo
                .setAttachments(attachments )
                .setSubpasses(subpassDescription)
                .setDependencies(subpassDependency);

            m_renderPass = m_logicalDevice->createRenderPass(renderPassCreateInfo);
        }
        spdlog::info("RENDER PASS CREATION ENDED");
    }

    void Vulkan::CreateDescriptorSetLayout()
    {
        /** UBO */
        vk::DescriptorSetLayoutBinding uboLayoutBinding;
        uboLayoutBinding.setBinding(0);
        uboLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        uboLayoutBinding.setDescriptorCount(1);
        uboLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

        /** Sampler */
        vk::DescriptorSetLayoutBinding samplerLayoutBinding;
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        const std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        vk::DescriptorSetLayoutCreateInfo dslCreateInfo({}, bindings);

        m_descriptorSetLayout = m_logicalDevice->createDescriptorSetLayout(dslCreateInfo);
    }

    void Vulkan::CreateGraphicsPipeline()
    {
        spdlog::info("PIPELINE CREATION STARTED");
        {
            spdlog::info("PIPELINE LAYOUT CREATION STARTED");
            {
                const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo({}, m_descriptorSetLayout);
                m_pipelineLayout = m_logicalDevice->createPipelineLayout(pipelineLayoutCreateInfo);
            }
            spdlog::info("PIPELINE LAYOUT CREATION ENDED");

            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();

            const vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo({}, bindingDescription, attributeDescriptions);
            const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

            /** Get viewport state info */
            m_viewport = vk::Viewport(0.0f, static_cast<float>(m_swapChainImagesExtent.height), static_cast<float>(m_swapChainImagesExtent.width), -static_cast<float>(m_swapChainImagesExtent.height), 0.0f, 1.0f);
            m_scissors = vk::Rect2D({0, 0}, m_swapChainImagesExtent);
            const auto viewportState = vk::PipelineViewportStateCreateInfo().setViewports(m_viewport).setScissors(m_scissors);

            /** Get resterization state info */
            const auto rasterizationStateInfo =
                vk::PipelineRasterizationStateCreateInfo()
                    .setDepthClampEnable(VK_FALSE)
                    .setRasterizerDiscardEnable(VK_FALSE)
                    .setPolygonMode(vk::PolygonMode::eFill)
                    .setLineWidth(1.0f)
                    .setCullMode(vk::CullModeFlagBits::eBack)
                    .setFrontFace(vk::FrontFace::eCounterClockwise)
                    .setDepthBiasEnable(VK_FALSE);

            /** Get multisample state info */
            const auto multisampleStateInfo =
                vk::PipelineMultisampleStateCreateInfo()
                    .setSampleShadingEnable(VK_FALSE)
                    .setRasterizationSamples(m_msaaSamples);

            /** Get color blend attachment state info */
            auto colorBlendAttachmentState =
                vk::PipelineColorBlendAttachmentState()
                    .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                    .setBlendEnable(VK_FALSE)
                    .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                    .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                    .setColorBlendOp(vk::BlendOp::eAdd)
                    .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                    .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                    .setAlphaBlendOp(vk::BlendOp::eAdd);

            /** Get color blend state info */
            vk::PipelineColorBlendStateCreateInfo colorBlendState;
            colorBlendState
                .setLogicOpEnable(VK_FALSE)
                .setAttachments(colorBlendAttachmentState)
                .setLogicOp(vk::LogicOp::eCopy)
                .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

            const auto vertexShader = Shader("vert.spv", m_logicalDevice, ShaderType::VR_VERTEX_SHADER);
            const auto fragmentShader = Shader("frag.spv", m_logicalDevice, ShaderType::VR_FRAGMENT_SHADER);
            std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesCreateInfos = {
                vertexShader.GetPipelineShaderStageInfo(),
                fragmentShader.GetPipelineShaderStageInfo(),
            };

            vk::PipelineDepthStencilStateCreateInfo depthStencilState;
            depthStencilState.depthTestEnable = VK_TRUE;
            depthStencilState.depthWriteEnable = VK_TRUE;
            depthStencilState.depthCompareOp = vk::CompareOp::eLess;
            depthStencilState.stencilTestEnable = VK_FALSE;

            vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
            pipelineCreateInfo
                .setStageCount(2)
                .setPVertexInputState(&vertexInputStateInfo)
                .setPInputAssemblyState(&inputAssemblyStateInfo)
                .setPViewportState(&viewportState)
                .setPRasterizationState(&rasterizationStateInfo)
                .setPMultisampleState(&multisampleStateInfo)
                .setPColorBlendState(&colorBlendState)
                .setPDepthStencilState(&depthStencilState)
                .setStages(shaderStagesCreateInfos)
                .setLayout(m_pipelineLayout)
                .setRenderPass(m_renderPass)
                .setSubpass(0);

            m_pipeline = m_logicalDevice->createGraphicsPipeline({}, pipelineCreateInfo).value;
        }
        spdlog::info("PIPELINE CREATION ENDED\n");
    }

    void Vulkan::CreateFramebuffers()
    {
        spdlog::info("FRAMEBUFFER CREATION STARTED");
        {
            m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
            for (std::size_t i = 0; i < m_swapChainImageViews.size(); ++i)
            {
                std::vector<vk::ImageView> imageViews = {m_colorImageView, m_depthImageView, m_swapChainImageViews[i]};

                vk::FramebufferCreateInfo framebufferInfo;
                framebufferInfo
                    .setRenderPass(m_renderPass)
                    .setAttachmentCount(1)
                    .setAttachments(imageViews)
                    .setWidth(m_swapChainImagesExtent.width)
                    .setHeight(m_swapChainImagesExtent.height)
                    .setLayers(1);

                m_swapChainFramebuffers[i] = m_logicalDevice->createFramebuffer(framebufferInfo);
            }
        }
        spdlog::info("FRAMEBUFFER CREATION ENDED\n");
    }

    void Vulkan::CreateCommandPool()
    {
        spdlog::info("COMMAND POOL CREATION STARTED");
        {
            vk::CommandPoolCreateInfo commandPoolCreateInfo({}, m_queueFamilies.graphicsFamily.value());
            m_commandPool = m_logicalDevice->createCommandPool(commandPoolCreateInfo);
        }
        spdlog::info("COMMAND POOL CREATION ENDED\n");
    }

    void Vulkan::CreateColorResources()
    {
        vk::Format colorFormat = m_swapChainImagesFormat;

        CreateImage(m_swapChainImagesExtent.width, m_swapChainImagesExtent.height, m_msaaSamples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, m_colorImage, m_colorImageMemory);
        m_colorImageView = CreateImageView(m_colorImage, colorFormat, vk::ImageAspectFlagBits::eColor);
    }

    void Vulkan::CreateDepthResources()
    {
        const auto depthFormat = FindDepthFormat();
        CreateImage(m_swapChainImagesExtent.width, m_swapChainImagesExtent.height, m_msaaSamples, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, m_depthImage, m_depthImageMemory);
        m_depthImageView = CreateImageView(m_depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
    }

    void Vulkan::CreateCommandBuffers()
    {
        const auto commandBuffersCount = static_cast<uint32_t>(m_swapChainFramebuffers.size());
        spdlog::info("COMMAND BUFFERS CREATION STARTED");
        {
            const vk::CommandBufferAllocateInfo commandBufferAllocateInfo(m_commandPool, vk::CommandBufferLevel::ePrimary, commandBuffersCount);
            m_commandBuffers = m_logicalDevice->allocateCommandBuffers(commandBufferAllocateInfo);
        }
        spdlog::info("COMMAND BUFFERS CREATION ENDED. CREATED {} CBs\n", commandBuffersCount);

        for (uint32_t i = 0; i < commandBuffersCount; ++i)
        {
            const auto& commandBuffer = m_commandBuffers[i];

            const vk::CommandBufferBeginInfo beginInfo;
            commandBuffer.begin(beginInfo);
            {
                vk::ClearColorValue clearColorValue;
                clearColorValue.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
                vk::ClearDepthStencilValue clearDepthStencilValue(1.0f, 0.0f);
                
                std::array<vk::ClearValue, 2> clearValues = {clearColorValue, clearDepthStencilValue};
                vk::RenderPassBeginInfo renderPassBeginInfo(
                    m_renderPass,
                    m_swapChainFramebuffers[i],
                    vk::Rect2D({0, 0}, m_swapChainImagesExtent),
                    clearValues
                );

                commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
                {
                    vk::DeviceSize offset = 0;

                    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
                    commandBuffer.bindVertexBuffers(0, m_vertexBuffer, offset);
                    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_descriptorSets[i], {});
                    commandBuffer.bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint32);
                    commandBuffer.drawIndexed(m_indices.size(), 1, 0, 0, 0);
                }
                commandBuffer.endRenderPass();
            }
            commandBuffer.end();
        }
    }

    void Vulkan::CreateSyncObjects()
    {
        const vk::SemaphoreCreateInfo semaphoreCreateInfo;
        const vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_imageAvailableSemaphores.push_back(m_logicalDevice->createSemaphore(semaphoreCreateInfo));
            m_renderFinishedSemaphores.push_back(m_logicalDevice->createSemaphore(semaphoreCreateInfo));
            m_inFlightFences.push_back(m_logicalDevice->createFence(fenceCreateInfo));
        }

        m_imagesInFlight.resize(m_swapChainImages.size(), vk::Fence());
    }

    void Vulkan::CreateTextureImage()
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = static_cast<vk::DeviceSize>(texWidth) * texHeight * 4;

        if (!pixels)
        {
            throw std::runtime_error("failed to load texture image!");
        }

        vk::Buffer imageStagingBuffer;
        vk::DeviceMemory imageStagingBufferMemory;

        CreateBuffer(
            imageSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            imageStagingBuffer, imageStagingBufferMemory
        );

        void* data = m_logicalDevice->mapMemory(imageStagingBufferMemory, 0, imageSize);
        memcpy(data, pixels, static_cast<std::size_t>(imageSize));
        m_logicalDevice->unmapMemory(imageStagingBufferMemory);

        stbi_image_free(pixels);

        CreateImage(
            texWidth,
            texHeight,
            vk::SampleCountFlagBits::e1,
            vk::Format::eR8G8B8A8Srgb,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            m_textureImage,
            m_textureImageMemory
        );

        TransitionImageLayout(m_textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        CopyBufferToImage(imageStagingBuffer, m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        TransitionImageLayout(m_textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        m_logicalDevice->freeMemory(imageStagingBufferMemory);
        m_logicalDevice->destroyBuffer(imageStagingBuffer);
    }

    void Vulkan::CreateTextureImageView()
    {
        m_textureImageView = CreateImageView(m_textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
    }

    void Vulkan::CreateTextureSampler()
    {
        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.setMagFilter(vk::Filter::eLinear);
        samplerInfo.setMinFilter(vk::Filter::eLinear);
        samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
        samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
        samplerInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
        samplerInfo.setAnisotropyEnable(VK_TRUE);
        samplerInfo.setMaxAnisotropy(16.0f);
        samplerInfo.setUnnormalizedCoordinates(VK_FALSE);

        m_textureSampler = m_logicalDevice->createSampler(samplerInfo);
    }

    void Vulkan::LoadModel()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex;
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                m_vertices.push_back(vertex);
                m_indices.push_back(m_indices.size());
            }
        }
    }

    void Vulkan::DrawFrame()
    {
        m_logicalDevice->waitForFences(m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        auto& imageAvailableSemaphore = m_imageAvailableSemaphores[m_currentFrame];
        auto& renderFinishedSemaphore = m_renderFinishedSemaphores[m_currentFrame];

        uint32_t imageIndex;
        try
        {
            imageIndex = m_logicalDevice->acquireNextImageKHR(m_swapChain, UINT64_MAX, imageAvailableSemaphore, {});
        }
        catch (const std::exception& ex)
        {
            RecreateSwapChain();
            return;
        }

        if (m_imagesInFlight[imageIndex])
        {
            m_logicalDevice->waitForFences(m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

        UpdateUniformBuffer(imageIndex);

        const std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vk::SubmitInfo submitInfo;
        submitInfo
            .setWaitSemaphores(imageAvailableSemaphore)
            .setWaitDstStageMask(waitStages)
            .setCommandBuffers(m_commandBuffers[imageIndex])
            .setSignalSemaphores(renderFinishedSemaphore);

        m_logicalDevice->resetFences(m_inFlightFences[m_currentFrame]);
        m_graphicsQueue.submit(submitInfo, m_inFlightFences[m_currentFrame]);

        try
        {
            const vk::PresentInfoKHR presentInfo(renderFinishedSemaphore, m_swapChain, imageIndex);
            if (m_presentationQueue.presentKHR(presentInfo) == vk::Result::eSuboptimalKHR || m_shouldResizeFramebuffer)
            {
                m_shouldResizeFramebuffer = false;
                RecreateSwapChain();
            }
        }
        catch (const std::exception& ex)
        {
            RecreateSwapChain();
        }

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Vulkan::UpdateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        const auto currentTime = std::chrono::high_resolution_clock::now();
        const auto time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        m_mvpUBO.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_mvpUBO.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_mvpUBO.proj = glm::perspective(glm::radians(45.0f), m_swapChainImagesExtent.width / (float)m_swapChainImagesExtent.height, 0.1f, 10.0f);

        void* data = m_logicalDevice->mapMemory(m_uniformBuffersMemory[currentImage], 0, sizeof(m_mvpUBO));
        memcpy(data, &m_mvpUBO, sizeof(m_mvpUBO));
        m_logicalDevice->unmapMemory(m_uniformBuffersMemory[currentImage]);
    }

    void Vulkan::WaitForDevice()
    {
        m_logicalDevice->waitIdle();
    }

    void Vulkan::RecreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(m_window, &width, &height);
            glfwWaitEvents();
        }

        WaitForDevice();
        CleanupSwapChain();

        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateGraphicsPipeline();
        CreateColorResources();
        CreateDepthResources();
        CreateFramebuffers();
        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateDescriptorSets();
        CreateCommandBuffers();
    }

    void Vulkan::ResizeFramebuffers()
    {
        m_shouldResizeFramebuffer = true;
    }

    void Vulkan::CreateVertexBuffer()
    {
        const vk::DeviceSize bufferSize = sizeof(Vertex) * m_vertices.size();

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

        void* bufferData = m_logicalDevice->mapMemory(stagingBufferMemory, 0, bufferSize);
        memcpy(bufferData, m_vertices.data(), static_cast<std::size_t>(bufferSize));
        m_logicalDevice->unmapMemory(stagingBufferMemory);

        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, m_vertexBuffer, m_vertexBufferMemory);
        CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

        m_logicalDevice->freeMemory(stagingBufferMemory);
        m_logicalDevice->destroyBuffer(stagingBuffer);
    }

    void Vulkan::CreateIndexBuffer()
    {
        const vk::DeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

        void* bufferData = m_logicalDevice->mapMemory(stagingBufferMemory, 0, bufferSize);
        memcpy(bufferData, m_indices.data(), static_cast<std::size_t>(bufferSize));
        m_logicalDevice->unmapMemory(stagingBufferMemory);

        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, m_indexBuffer, m_indexBufferMemory);
        CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

        m_logicalDevice->freeMemory(stagingBufferMemory);
        m_logicalDevice->destroyBuffer(stagingBuffer);
    }

    void Vulkan::CreateUniformBuffers()
    {
        const vk::DeviceSize bufferSize = sizeof(m_mvpUBO);
        const auto numberOfUBOs = m_swapChainImages.size();

        m_uniformBuffers.resize(numberOfUBOs);
        m_uniformBuffersMemory.resize(numberOfUBOs);

        for (std::size_t i = 0; i < numberOfUBOs; ++i)
        {
            CreateBuffer(
                bufferSize,
                vk::BufferUsageFlagBits::eUniformBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                m_uniformBuffers[i],
                m_uniformBuffersMemory[i]
            );
        }
    }

    void Vulkan::CreateDescriptorPool()
    {
        vk::DescriptorPoolSize uboSize(vk::DescriptorType::eUniformBuffer, m_swapChainImages.size());
        vk::DescriptorPoolSize samplerSize(vk::DescriptorType::eCombinedImageSampler, m_swapChainImages.size());
        const std::array<vk::DescriptorPoolSize, 2> poolSizes = {uboSize, samplerSize};

        vk::DescriptorPoolCreateInfo dpCreateInfo;
        dpCreateInfo.setPoolSizes(poolSizes);
        dpCreateInfo.setMaxSets(m_swapChainImages.size());

        m_descriptorPool = m_logicalDevice->createDescriptorPool(dpCreateInfo);
    }

    void Vulkan::CreateDescriptorSets()
    {
        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts(m_swapChainImages.size(), m_descriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo;
        allocInfo.setDescriptorPool(m_descriptorPool);
        allocInfo.setDescriptorSetCount(static_cast<uint32_t>(m_swapChainImages.size()));
        allocInfo.setSetLayouts(descriptorSetLayouts);

        m_descriptorSets = m_logicalDevice->allocateDescriptorSets(allocInfo);

        for (std::size_t i = 0; i < m_swapChainImages.size(); ++i)
        {
            vk::DescriptorBufferInfo bufferInfo;
            bufferInfo.setBuffer(m_uniformBuffers[i]);
            bufferInfo.setOffset(0);
            bufferInfo.setRange(sizeof(m_mvpUBO));

            vk::DescriptorImageInfo imageInfo;
            imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            imageInfo.setImageView(m_textureImageView);
            imageInfo.setSampler(m_textureSampler);

            vk::WriteDescriptorSet bufferDescriptorWrite;
            bufferDescriptorWrite.setDstSet(m_descriptorSets[i]);
            bufferDescriptorWrite.setDstBinding(0);
            bufferDescriptorWrite.setDstArrayElement(0);
            bufferDescriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer);
            bufferDescriptorWrite.setDescriptorCount(1);
            bufferDescriptorWrite.setBufferInfo(bufferInfo);

            vk::WriteDescriptorSet imageSamplerDescriptorWrite;
            imageSamplerDescriptorWrite.setDstSet(m_descriptorSets[i]);
            imageSamplerDescriptorWrite.setDstBinding(1);
            imageSamplerDescriptorWrite.setDstArrayElement(0);
            imageSamplerDescriptorWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
            imageSamplerDescriptorWrite.setDescriptorCount(1);
            imageSamplerDescriptorWrite.setImageInfo(imageInfo);

            const std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {bufferDescriptorWrite, imageSamplerDescriptorWrite};
            m_logicalDevice->updateDescriptorSets(descriptorWrites, {});
        }
    }

    void Vulkan::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
    {
        vk::BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.setSize(size);
        bufferCreateInfo.setUsage(usage);
        bufferCreateInfo.setSharingMode(vk::SharingMode::eExclusive);

        buffer = m_logicalDevice->createBuffer(bufferCreateInfo);

        const auto bufferMemoryRequirements = m_logicalDevice->getBufferMemoryRequirements(buffer);
        vk::MemoryAllocateInfo memoryAllocateInfo;
        memoryAllocateInfo.setAllocationSize(bufferMemoryRequirements.size);
        memoryAllocateInfo.setMemoryTypeIndex(
            FindMemoryType(bufferMemoryRequirements.memoryTypeBits, properties)
        );

        bufferMemory = m_logicalDevice->allocateMemory(memoryAllocateInfo);
        m_logicalDevice->bindBufferMemory(buffer, bufferMemory, 0);
    }

    void Vulkan::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
    {
        auto transferCommandBuffer = BeginSingleTimeCommands();
        {
            vk::BufferCopy copyInfo;
            copyInfo.setSrcOffset(0);
            copyInfo.setDstOffset(0);
            copyInfo.setSize(size);
            transferCommandBuffer.copyBuffer(srcBuffer, dstBuffer, copyInfo);
        }
        EndSingleTimeCommands(std::move(transferCommandBuffer));
    }

    void Vulkan::CreateImage(uint32_t width, uint32_t height, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory)
    {
        vk::Extent3D imageExtent(width, height, 1);

        vk::ImageCreateInfo imageInfo;
        imageInfo.setImageType(vk::ImageType::e2D);
        imageInfo.setExtent(imageExtent);
        imageInfo.setMipLevels(1);
        imageInfo.setArrayLayers(1);
        imageInfo.setFormat(format);
        imageInfo.setTiling(tiling);
        imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);
        imageInfo.setUsage(usage);
        imageInfo.setSharingMode(vk::SharingMode::eExclusive);
        imageInfo.setSamples(numSamples);

        image = m_logicalDevice->createImage(imageInfo);
        const auto memRequirements = m_logicalDevice->getImageMemoryRequirements(image);
        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        imageMemory = m_logicalDevice->allocateMemory(allocInfo);
        m_logicalDevice->bindImageMemory(image, imageMemory, 0);
    }

    void Vulkan::TransitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
    {
        auto commandBuffer = BeginSingleTimeCommands();
        {
            vk::ImageSubresourceRange subresourceRange;
            subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
            subresourceRange.setBaseMipLevel(0);
            subresourceRange.setLevelCount(1);
            subresourceRange.setBaseArrayLayer(0);
            subresourceRange.setLayerCount(1);

            vk::ImageMemoryBarrier memoryBarrier;
            memoryBarrier.setOldLayout(oldLayout);
            memoryBarrier.setNewLayout(newLayout);
            memoryBarrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            memoryBarrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            memoryBarrier.setImage(image);
            memoryBarrier.setSubresourceRange(subresourceRange);

            vk::PipelineStageFlags sourceStage;
            vk::PipelineStageFlags destinationStage;

            if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
            {
                memoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

                sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
                destinationStage = vk::PipelineStageFlagBits::eTransfer;
            }
            else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
            {
                memoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                memoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                sourceStage = vk::PipelineStageFlagBits::eTransfer;
                destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
            }
            else
            {
                throw std::invalid_argument("Unsupported layout transition!");
            }

            commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, memoryBarrier);
        }
        EndSingleTimeCommands(std::move(commandBuffer));
    }

    void Vulkan::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
    {
        auto commandBuffer = BeginSingleTimeCommands();
        {
            vk::BufferImageCopy region;
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = vk::Offset3D(0, 0, 0);
            region.imageExtent = vk::Extent3D(width, height, 1);

            commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
        }
        EndSingleTimeCommands(std::move(commandBuffer));
    }

    vk::CommandBuffer Vulkan::BeginSingleTimeCommands()
    {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
        allocInfo.setCommandPool(m_commandPool);
        allocInfo.setCommandBufferCount(1);

        const auto commandBuffer = m_logicalDevice->allocateCommandBuffers(allocInfo)[0];

        const vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        commandBuffer.begin(beginInfo);

        return commandBuffer;
    }

    void Vulkan::EndSingleTimeCommands(vk::CommandBuffer commandBuffer)
    {
        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(commandBuffer);

        m_graphicsQueue.submit(submitInfo, vk::Fence());
        m_graphicsQueue.waitIdle();

        m_logicalDevice->freeCommandBuffers(m_commandPool, commandBuffer);
    }

    vk::ImageView Vulkan::CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
    {
        const vk::ComponentMapping componentMapping;
        vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor);
        subresourceRange
            .setAspectMask(aspectFlags)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);

        vk::ImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo
            .setImage(std::move(image))
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(format)
            .setComponents(componentMapping)
            .setSubresourceRange(subresourceRange);

        return m_logicalDevice->createImageView(imageViewCreateInfo);
    }

    vk::Format Vulkan::FindDepthFormat()
    {
        return FindSupportedFormat(
            {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
            vk::ImageTiling::eOptimal,
            vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );
    }

    vk::Format Vulkan::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
    {
        for (const auto format : candidates)
        {
            const auto props = m_physicalDevice.getFormatProperties(format);
            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    bool Vulkan::HasStencilComponent(vk::Format format)
    {
        return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
    }

    std::vector<const char*> Vulkan::GetRequiredInstanceExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        // These are the extensions required by glfw
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        // Additional Debug extensions
        if (VR_ENABLE_VALIDATION_LAYERS)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    void Vulkan::CheckIfValidationLayersAreAvailable()
    {
        const auto availableLayers = vk::enumerateInstanceLayerProperties();
        for (const auto* validationLayer : VR_VALIDATION_LAYERS)
        {
            const auto foundIt = std::find_if(
                availableLayers.begin(),
                availableLayers.end(),
                [&](const vk::LayerProperties& layer) {
                    return (strcmp(validationLayer, layer.layerName) == 0);
                });

            if (foundIt == availableLayers.end())
            {
                throw std::runtime_error(fmt::format("{} not available!", validationLayer));
            }
        }
    }

    void Vulkan::SetUpDebugCallback()
    {
        if (!VR_ENABLE_VALIDATION_LAYERS)
        {
            return;
        }

        const auto messengerCreateInfo = GetDebugMessengerCreateInfo();
        m_debugMessenger = m_instance->createDebugUtilsMessengerEXT(messengerCreateInfo, nullptr, m_dldi);
    }

    vk::DebugUtilsMessengerCreateInfoEXT Vulkan::GetDebugMessengerCreateInfo()
    {
        return vk::DebugUtilsMessengerCreateInfoEXT(
            {},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            DebugCallback);
    }

    void Vulkan::ChoosePhysicalDeviceAndQueueFamilies()
    {
        const auto presentDevices = m_instance->enumeratePhysicalDevices(m_dldi);
        for (const auto& device : presentDevices)
        {
            const auto features = device.getFeatures();

            auto deviceQueueFamilies = GetDeviceQueueFamilies(device);
            if (AreDeviceQueueFamiliesSupported(deviceQueueFamilies) && DoesDeviceSupportRequiredExtensions(device) && features.samplerAnisotropy)
            {
                m_physicalDevice = device;
                m_queueFamilies = std::move(deviceQueueFamilies);
                m_msaaSamples = GetMaxUsableSampleCount();

                return;
            }
        }

        throw std::runtime_error("No device supports required queue families!");
    }

    QueueFamilies Vulkan::GetDeviceQueueFamilies(const vk::PhysicalDevice& device)
    {
        QueueFamilies deviceFamilies;
        const auto availableFamilies = device.getQueueFamilyProperties();
        for (uint32_t index = 0; index < availableFamilies.size(); ++index)
        {
            if (availableFamilies[index].queueCount <= 0)
            {
                continue;
            }

            // Check support for graphics queue
            if (availableFamilies[index].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                deviceFamilies.graphicsFamily = index;
            }

            // Check support for presentation queue
            if (device.getSurfaceSupportKHR(index, m_surface))
            {
                deviceFamilies.presentationFamily = index;
            }
        }

        return deviceFamilies;
    }

    bool Vulkan::IsDeviceSuitable(const vk::PhysicalDevice& device, const QueueFamilies& deviceQueueFamilies)
    {
        const auto swapChainSupport = GetSwapChainSupportDetails(device);
        bool isSwapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

        return AreDeviceQueueFamiliesSupported(deviceQueueFamilies) && DoesDeviceSupportRequiredExtensions(device) && isSwapChainAdequate;
    }

    SwapChainSupportDetails Vulkan::GetSwapChainSupportDetails(const vk::PhysicalDevice& device)
    {
        SwapChainSupportDetails details;
        details.capabilities = device.getSurfaceCapabilitiesKHR(m_surface);
        details.formats = device.getSurfaceFormatsKHR(m_surface);
        details.presentModes = device.getSurfacePresentModesKHR(m_surface);

        return details;
    }

    bool Vulkan::AreDeviceQueueFamiliesSupported(const QueueFamilies& deviceQueueFamilies)
    {
        return deviceQueueFamilies.graphicsFamily.has_value() && deviceQueueFamilies.presentationFamily.has_value();
    }

    bool Vulkan::DoesDeviceSupportRequiredExtensions(const vk::PhysicalDevice& device)
    {
        const auto availableExtensions = device.enumerateDeviceExtensionProperties();
        for (const auto* requiredExtension : VR_REQUIRED_DEVICE_EXTENSIONS)
        {
            const auto foundIt =
                std::find_if(availableExtensions.begin(), availableExtensions.end(), [&](const vk::ExtensionProperties& extension) {
                    return (strcmp(extension.extensionName, requiredExtension) == 0);
                });

            if (foundIt == availableExtensions.end())
            {
                return false;
            }
        }

        return true;
    }

    uint32_t Vulkan::FindMemoryType(uint32_t requiredType, vk::MemoryPropertyFlags requiredProperties)
    {
        const auto memoryProperties = m_physicalDevice.getMemoryProperties();
        for (uint32_t memoryIndex = 0; memoryIndex < memoryProperties.memoryTypeCount; ++memoryIndex)
        {
            const auto memoryTypeBit = (1 << memoryIndex);
            if ((requiredType & memoryTypeBit) && (memoryProperties.memoryTypes[memoryIndex].propertyFlags & requiredProperties) == requiredProperties)
            {
                return memoryIndex;
            }
        }

        return -1;
    }

    vk::SampleCountFlagBits Vulkan::GetMaxUsableSampleCount()
    {
        const auto physicalDeviceProperties = m_physicalDevice.getProperties();
        vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & vk::SampleCountFlagBits::e64)
        {
            return vk::SampleCountFlagBits::e64;
        }
        if (counts & vk::SampleCountFlagBits::e32)
        {
            return vk::SampleCountFlagBits::e32;
        }
        if (counts & vk::SampleCountFlagBits::e16)
        {
            return vk::SampleCountFlagBits::e16;
        }
        if (counts & vk::SampleCountFlagBits::e8)
        {
            return vk::SampleCountFlagBits::e8;
        }
        if (counts & vk::SampleCountFlagBits::e4)
        {
            return vk::SampleCountFlagBits::e4;
        }
        if (counts & vk::SampleCountFlagBits::e2)
        {
            return vk::SampleCountFlagBits::e2;
        }

        return vk::SampleCountFlagBits::e1;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        if (strcmp(
                pCallbackData->pMessage,
                "loader_get_json: Failed to open JSON file D:\\Gry i Programy\\Epic Games\\Launcher\\Portal\\Extras\\Overlay\\EOSOverlayVkLayer-Win32.json") == 0)
        {
            return VK_FALSE;
        }

        if (strcmp(
                pCallbackData->pMessage,
                "loader_get_json: Failed to open JSON file D:\\Gry i Programy\\Epic Games\\Launcher\\Portal\\Extras\\Overlay\\EOSOverlayVkLayer-Win64.json") == 0)
        {
            return VK_FALSE;
        }

        switch (static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity))
        {
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            spdlog::info("Vulkan has triggered an info: {}", pCallbackData->pMessage);
            break;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            spdlog::info("Vulkan has triggered a verbose message: {}", pCallbackData->pMessage);
            break;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            spdlog::warn("Vulkan has triggered a warning: {}", pCallbackData->pMessage);
            break;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            spdlog::error("Vulkan has triggered an error: {}", pCallbackData->pMessage);
            break;
        }

        return VK_FALSE;
    }

    void Vulkan::CleanupSwapChain()
    {
        m_logicalDevice->destroyImageView(m_colorImageView);
        m_logicalDevice->freeMemory(m_colorImageMemory);
        m_logicalDevice->destroyImage(m_colorImage);

        m_logicalDevice->destroyImageView(m_depthImageView);
        m_logicalDevice->freeMemory(m_depthImageMemory);
        m_logicalDevice->destroyImage(m_depthImage);

        m_logicalDevice->freeCommandBuffers(m_commandPool, m_commandBuffers);

        for (auto framebuffer : m_swapChainFramebuffers)
        {
            m_logicalDevice->destroyFramebuffer(std::move(framebuffer));
        }
        m_swapChainFramebuffers.clear();
        m_swapChainFramebuffers.resize(0);

        m_logicalDevice->destroyPipeline(m_pipeline);
        m_logicalDevice->destroyPipelineLayout(m_pipelineLayout);
        m_logicalDevice->destroyRenderPass(m_renderPass);

        for (auto imageView : m_swapChainImageViews)
        {
            m_logicalDevice->destroyImageView(std::move(imageView));
        }
        m_swapChainImageViews.clear();
        m_swapChainImageViews.resize(0);

        if (m_swapChain)
        {
            m_logicalDevice->destroySwapchainKHR(m_swapChain);
        }

        for (std::size_t i = 0; i < m_swapChainImages.size(); ++i)
        {
            m_logicalDevice->freeMemory(m_uniformBuffersMemory[i]);
            m_logicalDevice->destroyBuffer(m_uniformBuffers[i]);
        }

        m_logicalDevice->destroyDescriptorPool(m_descriptorPool);
    }

    Vulkan::~Vulkan()
    {
        WaitForDevice();

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_logicalDevice->destroySemaphore(m_imageAvailableSemaphores[i]);
            m_logicalDevice->destroySemaphore(m_renderFinishedSemaphores[i]);
            m_logicalDevice->destroyFence(m_inFlightFences[i]);
        }

        CleanupSwapChain();

        m_logicalDevice->destroySampler(m_textureSampler);
        m_logicalDevice->destroyImageView(m_textureImageView);
        m_logicalDevice->destroyImage(m_textureImage);
        m_logicalDevice->freeMemory(m_textureImageMemory);
        
        m_logicalDevice->destroyDescriptorSetLayout(m_descriptorSetLayout);

        m_logicalDevice->freeMemory(m_vertexBufferMemory);
        m_logicalDevice->destroyBuffer(m_vertexBuffer);
        m_logicalDevice->freeMemory(m_indexBufferMemory);
        m_logicalDevice->destroyBuffer(m_indexBuffer);

        m_logicalDevice->destroyCommandPool(m_commandPool);

        m_instance->destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, m_dldi);
        m_instance->destroySurfaceKHR(m_surface);
    }
} // namespace vr