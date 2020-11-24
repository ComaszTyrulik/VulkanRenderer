#include "VulkanRenderer/Application.h"
#include "VulkanRenderer/Vulkan/Initializer.h"

#include <spdlog/spdlog.h>

namespace vr
{
    static void OnFramebufferResized(GLFWwindow* window, int width, int height)
    {
        auto vulkan = reinterpret_cast<Vulkan*>(glfwGetWindowUserPointer(window));
        vulkan->ResizeFramebuffers();
    }

    Application::Application(int windowWidth, int windowHeight)
        : WINDOW_WIDTH(windowWidth), WINDOW_HEIGHT(windowHeight)
    {
        InitWindow();
        InitVulkan();

        glfwSetWindowUserPointer(m_window, m_vulkan.get());
    }

    Application::~Application()
    {
        Cleanup();
    }

    void Application::Run()
    {
        MainLoop();
    }

    void Application::InitWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Renderer", nullptr, nullptr);
        glfwSetFramebufferSizeCallback(m_window, OnFramebufferResized);
    }

    void Application::InitVulkan()
    {
        m_vulkan = m_vulkanInitializer.Init("Vulkan Hello Triangle", m_window);
        m_vulkanInitializer.ListAvailableVulkanExtensions();

        m_vulkan->CreateInstance();
        m_vulkan->CreateSurface();
        m_vulkan->CreateLogicalDevice();
        m_vulkan->CreateSwapChain();
        m_vulkan->CreateImageViews();
        m_vulkan->CreateRenderPass();
        m_vulkan->CreateDescriptorSetLayout();
        m_vulkan->CreateGraphicsPipeline();
        m_vulkan->CreateCommandPool();
        m_vulkan->CreateColorResources();
        m_vulkan->CreateDepthResources();
        m_vulkan->CreateFramebuffers();
        m_vulkan->CreateTextureImage();
        m_vulkan->CreateTextureImageView();
        m_vulkan->CreateTextureSampler();
        m_vulkan->LoadModel();
        m_vulkan->CreateVertexBuffer();
        m_vulkan->CreateIndexBuffer();
        m_vulkan->CreateUniformBuffers();
        m_vulkan->CreateDescriptorPool();
        m_vulkan->CreateDescriptorSets();
        m_vulkan->CreateCommandBuffers();
        m_vulkan->CreateSyncObjects();

        spdlog::info("APP IS UP AN RUNNING");
    }

    void Application::MainLoop()
    {
        while (!glfwWindowShouldClose(m_window))
        {
            glfwPollEvents();
            m_vulkan->DrawFrame();
        }
    }

    void Application::Cleanup()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
} // namespace vr