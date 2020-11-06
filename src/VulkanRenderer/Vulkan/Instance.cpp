#include "VulkanRenderer/Vulkan/Instance.h"

#include <spdlog/spdlog.h>
#include <glfw/glfw3.h>

namespace vr
{
#ifndef NDEBUG
    const bool VR_ENABLE_VALIDATION_LAYERS = true;
    static const std::vector<const char*> VR_VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
#else
    const bool VR_ENABLE_VALIDATION_LAYERS = false;
    static const std::vector<const char*> VR_VALIDATION_LAYERS = {};
#endif

    static const std::vector<const char*> VR_REQUIRED_DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    Instance::Instance(const std::string& appName)
    {
        if (VR_ENABLE_VALIDATION_LAYERS)
        {
            CheckIfValidationLayersAreAvailable();
        }

        /**
         * Apparentlly, this step is required. Otherwise, there will be the access violation when initializing DynamicLoader with vk::Instance,
         * because Vulkan won't be able to find 'vkGetInstanceProcAddr' function
         **/
        vk::DynamicLoader dl;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        m_dldi.init(vkGetInstanceProcAddr);

        CreateInstance(appName);
    }

    Instance::~Instance()
    {
        m_instance->destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, m_dldi);
        m_instance->destroySurfaceKHR(m_surface);
    }

    void Instance::CreateSurface(GLFWwindow* window)
    {
        spdlog::info("SURFACE CREATION STARTED");
        {
            if (glfwCreateWindowSurface(m_instance.get(), window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&m_surface)) != VK_SUCCESS)
            {
                throw std::runtime_error("Could not create window surface!");
            }
        }
        spdlog::info("SURFACE CREATION ENDED\n");
    }

    std::unique_ptr<Device> Instance::CreateDevice(GLFWwindow* window)
    {
        const auto physicalDevice = GetValidPhysicalDevice();
        auto device = std::make_unique<Device>(physicalDevice, VR_REQUIRED_DEVICE_EXTENSIONS);
        device->CreateSwapChain(QuerySwapChainSupport(physicalDevice.first), window, m_surface);

        return device;
    }

    /***********************************
     **************PRIVATE**************
     ***********************************/
    void vr::Instance::CreateInstance(const std::string& appName)
    {
        spdlog::info("INSTANCE CREATION STARTED");
        {
            vk::ApplicationInfo appInfo = vk::ApplicationInfo(appName.c_str(), VK_MAKE_VERSION(1, 0, 0));
            appInfo.setApiVersion(VK_API_VERSION_1_2);

            const auto extensions = GetRequiredExtensions();
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

    std::vector<const char*> Instance::GetRequiredExtensions()
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

    void Instance::CheckIfValidationLayersAreAvailable()
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

    void Instance::SetUpDebugCallback()
    {
        if (!VR_ENABLE_VALIDATION_LAYERS)
        {
            return;
        }

        const auto messengerCreateInfo = GetDebugMessengerCreateInfo();
        m_debugMessenger = m_instance->createDebugUtilsMessengerEXT(messengerCreateInfo, nullptr, m_dldi);
    }

    vk::DebugUtilsMessengerCreateInfoEXT Instance::GetDebugMessengerCreateInfo()
    {
        return vk::DebugUtilsMessengerCreateInfoEXT(
            {},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            DebugCallback);
    }

    std::pair<vk::PhysicalDevice, QueueFamilies> Instance::GetValidPhysicalDevice()
    {
        const auto presentDevices = m_instance->enumeratePhysicalDevices(m_dldi);
        for (const auto& device : presentDevices)
        {
            auto deviceQueueFamilies = GetDeviceQueueFamilies(device);
            if (AreDeviceQueueFamiliesSupported(deviceQueueFamilies) && DoesDeviceSupportRequiredExtensions(device))
            {
                return std::make_pair(device, std::move(deviceQueueFamilies));
            }
        }

        throw std::runtime_error("No device supports required queue families!");
    }

    QueueFamilies Instance::GetDeviceQueueFamilies(const vk::PhysicalDevice& device)
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

    bool Instance::IsDeviceSuitable(const vk::PhysicalDevice& device, const QueueFamilies& deviceQueueFamilies)
    {
        const auto swapChainSupport = QuerySwapChainSupport(device);
        bool isSwapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

        return AreDeviceQueueFamiliesSupported(deviceQueueFamilies) && DoesDeviceSupportRequiredExtensions(device) && isSwapChainAdequate;
    }

    SwapChainSupportDetails Instance::QuerySwapChainSupport(const vk::PhysicalDevice& device)
    {
        SwapChainSupportDetails details;
        details.capabilities = device.getSurfaceCapabilitiesKHR(m_surface);
        details.formats = device.getSurfaceFormatsKHR(m_surface);
        details.presentModes = device.getSurfacePresentModesKHR(m_surface);

        return details;
    }

    bool Instance::AreDeviceQueueFamiliesSupported(const QueueFamilies& deviceQueueFamilies)
    {
        return deviceQueueFamilies.graphicsFamily.has_value() && deviceQueueFamilies.presentationFamily.has_value();
    }

    bool Instance::DoesDeviceSupportRequiredExtensions(const vk::PhysicalDevice& device)
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

    VKAPI_ATTR VkBool32 VKAPI_CALL Instance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
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
            DebugBreak();
            break;
        }

        return VK_FALSE;
    }
} // namespace vr