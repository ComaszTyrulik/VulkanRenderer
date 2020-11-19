#include "VulkanRenderer/Vulkan/Vulkan.h"

#include <spdlog/spdlog.h>
#include <glfw/glfw3.h>
#include <set>

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

    Vulkan::~Vulkan()
    {
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            m_logicalDevice->destroySemaphore(m_imageAvailableSemaphores[i]);
            m_logicalDevice->destroySemaphore(m_renderFinishedSemaphores[i]);
            m_logicalDevice->destroyFence(m_inFlightFences[i]);
        }

        m_logicalDevice->destroyCommandPool(m_commandPool);
        for (auto framebuffer : m_swapChainFramebuffers)
        {
            m_logicalDevice->destroyFramebuffer(std::move(framebuffer));
        }

        m_logicalDevice->destroyPipeline(m_pipeline);
        m_logicalDevice->destroyPipelineLayout(m_pipelineLayout);
        m_logicalDevice->destroyRenderPass(m_renderPass);

        for (auto imageView : m_swapChainImageViews)
        {
            m_logicalDevice->destroyImageView(std::move(imageView));
        }

        m_logicalDevice->destroySwapchainKHR(m_swapChain);

        m_instance->destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, m_dldi);
        m_instance->destroySurfaceKHR(m_surface);
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

            const vk::DeviceCreateInfo deviceCreateInfo({}, deviceQueueCreateInfos, {} /*This should be empty*/, VR_REQUIRED_DEVICE_EXTENSIONS);
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
                const vk::ComponentMapping componentMapping;
                vk::ImageSubresourceRange subresourceRange(vk::ImageAspectFlagBits::eColor);
                subresourceRange
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1);

                vk::ImageViewCreateInfo imageViewCreateInfo;
                imageViewCreateInfo
                    .setImage(image)
                    .setViewType(vk::ImageViewType::e2D)
                    .setFormat(m_swapChainImagesFormat)
                    .setComponents(componentMapping)
                    .setSubresourceRange(subresourceRange);

                m_swapChainImageViews.push_back(std::move(m_logicalDevice->createImageView(imageViewCreateInfo)));
            }
        }
        spdlog::info("IMAGE VIEWS CREATION ENDED\n");
    }

    void Vulkan::CreateRenderPass()
    {
        spdlog::info("RENDER PASS CREATION STARTED");
        {
            vk::AttachmentDescription attachmentDescription({}, m_swapChainImagesFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear);
            attachmentDescription
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

            vk::AttachmentReference attachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);

            vk::SubpassDescription subpassDescription;
            subpassDescription
                .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                .setColorAttachments(attachmentReference);

            vk::SubpassDependency subpassDependency;
            subpassDependency
                .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                .setDstSubpass(0)
                .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setSrcAccessMask(vk::AccessFlags())
                .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

            vk::RenderPassCreateInfo renderPassCreateInfo;
            renderPassCreateInfo
                .setAttachments(attachmentDescription)
                .setSubpasses(subpassDescription)
                .setDependencies(subpassDependency);

            m_renderPass = m_logicalDevice->createRenderPass(renderPassCreateInfo);
        }
        spdlog::info("RENDER PASS CREATION ENDED");
    }

    void Vulkan::CreateGraphicsPipeline(const std::unique_ptr<Shader>& vertexShader, const std::unique_ptr<Shader>& fragmentShader)
    {
        spdlog::info("PIPELINE CREATION STARTED");
        {
            spdlog::info("PIPELINE LAYOUT CREATION STARTED");
            {
                m_pipelineLayout = m_logicalDevice->createPipelineLayout({});
            }
            spdlog::info("PIPELINE LAYOUT CREATION ENDED");

            const vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo;
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
                    .setRasterizationSamples(vk::SampleCountFlagBits::e1);

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

            std::vector<vk::PipelineShaderStageCreateInfo> shaderStagesCreateInfos = {
                vertexShader->GetPipelineShaderStageInfo(),
                fragmentShader->GetPipelineShaderStageInfo(),
            };

            vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
            pipelineCreateInfo
                .setStageCount(2)
                .setPVertexInputState(&vertexInputStateInfo)
                .setPInputAssemblyState(&inputAssemblyStateInfo)
                .setPViewportState(&viewportState)
                .setPRasterizationState(&rasterizationStateInfo)
                .setPMultisampleState(&multisampleStateInfo)
                .setPColorBlendState(&colorBlendState)
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
                std::vector<vk::ImageView> imageViews = {m_swapChainImageViews[i]};

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
                vk::RenderPassBeginInfo renderPassBeginInfo(
                    m_renderPass,
                    m_swapChainFramebuffers[i],
                    vk::Rect2D({0, 0}, m_swapChainImagesExtent),
                    vk::ClearValue(clearColorValue));

                commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
                {
                    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
                    commandBuffer.draw(3, 1, 0, 0);
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

    void Vulkan::DrawFrame()
    {
        m_logicalDevice->waitForFences(m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

        auto& imageAvailableSemaphore = m_imageAvailableSemaphores[m_currentFrame];
        auto& renderFinishedSemaphore = m_renderFinishedSemaphores[m_currentFrame];

        uint32_t imageIndex = m_logicalDevice->acquireNextImageKHR(m_swapChain, UINT64_MAX, imageAvailableSemaphore, {});
        if (m_imagesInFlight[imageIndex])
        {
            m_logicalDevice->waitForFences(m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

        const std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vk::SubmitInfo submitInfo;
        submitInfo
            .setWaitSemaphores(imageAvailableSemaphore)
            .setWaitDstStageMask(waitStages)
            .setCommandBuffers(m_commandBuffers[imageIndex])
            .setSignalSemaphores(renderFinishedSemaphore);

        m_logicalDevice->resetFences(m_inFlightFences[m_currentFrame]);
        m_graphicsQueue.submit(submitInfo, m_inFlightFences[m_currentFrame]);

        const vk::PresentInfoKHR presentInfo(renderFinishedSemaphore, m_swapChain, imageIndex);
        m_presentationQueue.presentKHR(presentInfo);

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Vulkan::WaitForDevice()
    {
        m_logicalDevice->waitIdle();
    }

    std::unique_ptr<Shader> Vulkan::CreateShader(const std::string& shaderName, ShaderType type) const
    {
        return std::make_unique<Shader>(shaderName, m_logicalDevice, type);
    }

    /***********************************/
    /**************PRIVATE**************/
    /***********************************/
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
            auto deviceQueueFamilies = GetDeviceQueueFamilies(device);
            if (AreDeviceQueueFamiliesSupported(deviceQueueFamilies) && DoesDeviceSupportRequiredExtensions(device))
            {
                m_physicalDevice = device;
                m_queueFamilies = std::move(deviceQueueFamilies);

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
} // namespace vr