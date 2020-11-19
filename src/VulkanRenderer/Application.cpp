#include "VulkanRenderer/Application.h"
#include "VulkanRenderer/Vulkan/Initializer.h"

#include <spdlog/spdlog.h>

namespace vr
{
    Application::Application(int windowWidth, int windowHeight)
        : WINDOW_WIDTH(windowWidth), WINDOW_HEIGHT(windowHeight)
    {
        InitWindow();
        InitVulkan();
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
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Renderer", nullptr, nullptr);
    }

    void Application::InitVulkan()
    {
        m_vulkan = m_vulkanInitializer.Init("Vulkan Hello Triangle", m_window);
        m_vulkanInitializer.ListAvailableVulkanExtensions();

        m_vulkan->CreateInstance();
        m_vulkan->CreateSurface();
        m_vulkan->CreateLogicalDevice();
        
        const auto vertexShader = m_vulkan->CreateShader("vert.spv", vr::ShaderType::VR_VERTEX_SHADER);
        const auto fragmentShader = m_vulkan->CreateShader("frag.spv", vr::ShaderType::VR_FRAGMENT_SHADER);

        m_vulkan->CreateSwapChain();
        m_vulkan->CreateImageViews();
        m_vulkan->CreateRenderPass();
        m_vulkan->CreateGraphicsPipeline(vertexShader, fragmentShader);
        m_vulkan->CreateFramebuffers();
        m_vulkan->CreateCommandPool();
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
        
        m_vulkan->WaitForDevice();
    }

    void Application::Cleanup()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
} // namespace vr