// Project includes
#include "VulkanRenderer/Application.h"

// Vendors includes
#include <spdlog/spdlog.h>

// STL includes
#include <stdexcept>
#include <cstdlib>

int main(int, char**)
{
    const int WIDTH = 800;
    const int HEIGHT = 600;

    try
    {
        vr::Application app(WIDTH, HEIGHT);
        app.Run();
    }
    catch (const std::exception& e)
    {
        spdlog::error(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}