#pragma once
#include "VulkanRenderer/Paths.h"
#include <vulkan/vulkan.hpp>

namespace vr
{
    enum class ShaderType
    {
        VR_VERTEX_SHADER,
        VR_FRAGMENT_SHADER
    };

    class Shader
    {
    public:
        Shader(const std::string& shaderName, const vk::UniqueDevice& device, ShaderType type);
        vk::PipelineShaderStageCreateInfo GetPipelineShaderStageInfo() const;

    private:
        std::vector<char> LoadCode(const std::string& filename);
        vk::ShaderModuleCreateInfo CreateShaderModule(const std::vector<char>& byteCode);

    private:
        vk::ShaderStageFlagBits m_shaderType;
        vk::UniqueShaderModule m_shaderModule;

        inline static const std::unordered_map<ShaderType, vk::ShaderStageFlagBits> SHADER_TYPES_MAP = {
            {ShaderType::VR_VERTEX_SHADER, vk::ShaderStageFlagBits::eVertex},
            {ShaderType::VR_FRAGMENT_SHADER, vk::ShaderStageFlagBits::eFragment},
        };
    };
} // namespace vr