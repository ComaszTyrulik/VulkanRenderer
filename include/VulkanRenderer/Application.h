#pragma once
#include "VulkanRenderer/Vulkan/Initializer.h"
#include "VulkanRenderer/Vulkan/Instance.h"

#include <GLFW/glfw3.h>

namespace vr
{
    class Application
    {
    public:
        Application(int windowWidth, int windowHeight);
        ~Application();

        void Run();

    private:
        void InitWindow();
        void InitVulkan();

        void MainLoop();
        void Cleanup();

    private:
        VulkanInitializer m_vulkanInitializer;
        std::unique_ptr<Instance> m_instance;
        GLFWwindow* m_window;
        
        const int WINDOW_WIDTH;
        const int WINDOW_HEIGHT;
    };
} // namespace vr