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
        m_instance = m_vulkanInitializer.CreateInstance("Vulkan Hello Triangle");
        m_instance->CreateSurface(m_window);

        m_vulkanInitializer.ListAvailableVulkanExtensions();
    }

    void Application::MainLoop()
    {
        const auto device = m_instance->CreateDevice();
        while (!glfwWindowShouldClose(m_window))
        {
            glfwPollEvents();
        }
    }

    void Application::Cleanup()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }
} // namespace vr