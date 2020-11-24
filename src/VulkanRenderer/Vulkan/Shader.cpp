#include "VulkanRenderer/Vulkan/Shader.h"

#include <fstream>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace vr
{
    Shader::Shader(const std::string& shaderName, const vk::UniqueDevice& device, ShaderType type)
        : m_shaderType(SHADER_TYPES_MAP.at(type))
    {
        spdlog::info("{} SHADER CREATION STARTED", shaderName);
        {
            const auto byteCode = LoadCode(VK_GET_SHADER_PATH(shaderName));
            m_shaderModule = device->createShaderModuleUnique(CreateShaderModule(byteCode));
        }
        spdlog::info("{} SHADER CREATION ENDED\n", shaderName);
    }

    vk::PipelineShaderStageCreateInfo Shader::GetPipelineShaderStageInfo() const
    {
        return vk::PipelineShaderStageCreateInfo({}, m_shaderType, m_shaderModule.get(), "main");
    }
    
    std::vector<char> Shader::LoadCode(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error(fmt::format("Failed to open '{}' shader file!", filename));
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        
        return buffer;
    }
    
    vk::ShaderModuleCreateInfo Shader::CreateShaderModule(const std::vector<char>& byteCode)
    {
        return vk::ShaderModuleCreateInfo({}, byteCode.size(), reinterpret_cast<const uint32_t*>(byteCode.data()));
    }
} // namespace vr
