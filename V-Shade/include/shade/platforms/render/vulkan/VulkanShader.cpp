#include "shade_pch.h"
#include "VulkanShader.h"
#include <shade/core/application/Application.h>
#include <shade/platforms/render/vulkan/VulkanSwapChain.h>
#include <glm/glm/gtx/hash.hpp>

namespace utils
{
    static const char* GetCachedFileExtension(const shade::Shader::Type& type)
    {
        switch (type)
        {
            case shade::Shader::Type::Vertex:return ".vk.c.vert";
            case shade::Shader::Type::Fragment:return ".vk.c.frag";
            case shade::Shader::Type::Geometry:return ".vk.c.geom";
            case shade::Shader::Type::Compute:return ".vk.c.comp";
            default: return ".vk.c.undefined";
        }
    }
    static shaderc_shader_kind ToShaderCShaderType(const shade::Shader::Type& type)
    {
        switch (type)
        {
            case shade::Shader::Type::Vertex:return shaderc_glsl_vertex_shader;
            case shade::Shader::Type::Fragment:return shaderc_glsl_fragment_shader;
            case shade::Shader::Type::Geometry:return shaderc_glsl_geometry_shader;
            case shade::Shader::Type::Compute:return shaderc_glsl_compute_shader;
        default: return (shaderc_shader_kind)0;
        }
    }
    static const char* ShaderCSTypeToString(const shaderc_shader_kind& type)
    {
        switch (type)
        {
        case shaderc_glsl_vertex_shader:    return "Vertex shader";
        case shaderc_glsl_fragment_shader:  return "Fragment shader";
        case shaderc_glsl_geometry_shader:  return "Geometry shader";
        case shaderc_glsl_compute_shader:   return "Compute shader";
        default: return "Undefined shader";
        }
    }
    static const char* ShaderTypeToString(const shade::Shader::Type& type)
    {
        switch (type)
        {
        case shade::Shader::Type::Vertex:    return "Vertex shader";
        case shade::Shader::Type::Fragment:  return "Fragment shader";
        case shade::Shader::Type::Geometry:  return "Geometry shader";
        case shade::Shader::Type::Compute:   return "Compute shader";
        default: return "Undefined shader";
        }
    }
}

shade::VulkanShader::VulkanShader(const std::string& filePath): 
    Shader(filePath)
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions options; 

    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

    //options.SetOptimizationLevel(shaderc_optimization_level_performance);
    
    std::filesystem::path cacheDirectory = GetShaderCacheDirectory();

    auto& shaderData = m_VulkanSPIRV;
    for (auto&& [stage, source] : m_SourceCode)
    {
        std::filesystem::path shaderFilePath = GetFilePath();
        std::filesystem::path cachedPath = cacheDirectory / (GetFileName() + utils::GetCachedFileExtension(stage));
        std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
 
        // Always recompile shaders !
#if 1
        if (in.is_open())
        {
            in.seekg(0, std::ios::end);
            auto size = in.tellg();
            in.seekg(0, std::ios::beg);

            auto& data = shaderData[stage];
            data.resize(size / sizeof(uint32_t));
            in.read((char*)data.data(), size);
        }
        else
        {
            shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, utils::ToShaderCShaderType(stage), GetFilePath().c_str(), options);

            if (module.GetCompilationStatus() != shaderc_compilation_status_success)
            {
                SHADE_CORE_ERROR(module.GetErrorMessage());
            }

            shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

            std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
            if (out.is_open())
            {
                auto& data = shaderData[stage];
                out.write((char*)data.data(), data.size() * sizeof(uint32_t));
                out.flush();
                out.close();
            }
        }
#else
        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, utils::ToShaderCShaderType(stage), GetFilePath().c_str(), options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            SHADE_CORE_ERROR(module.GetErrorMessage());
        }

        shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

        std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
        if (out.is_open())
        {
            auto& data = shaderData[stage];
            out.write((char*)data.data(), data.size() * sizeof(uint32_t));
            out.flush();
            out.close();
        }
#endif // 1

    }

    for (auto&& [stage, data] : shaderData)
        Reflect(stage, data);

	CreateShader();
}

void shade::VulkanShader::Reflect(shade::Shader::Type type, const std::vector<uint32_t>& shaderData)
{
    spirv_cross::Compiler compiler(shaderData);
    m_SprvReflectionData = compiler.get_shader_resources();

    SHADE_CORE_TRACE("Shader type = '{0}':----------------", GetTypeAsString(type));
    for (const auto& resource : m_SprvReflectionData.storage_buffers)
    {
        const auto& bufferType = compiler.get_type(resource.base_type_id);
        std::uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        std::uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        std::size_t seed = set, hash = binding; glm::detail::hash_combine(seed, hash);
        std::string name = (resource.name.size()) ? resource.name : std::to_string(seed);

        auto& buffer = m_ReflectedData[set].StorageBuffers[name];
    
        buffer.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        buffer.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        buffer.Size = compiler.get_declared_struct_size(bufferType);
        buffer.ShaderType = FromShaderTypeToVkShaderType(type);
        buffer.Name = name;
        
        SHADE_CORE_TRACE("   StorageBuffer: Name = {0}, Set = {1}, Binding = {2}, Size = {3}, ", buffer.Name, buffer.Set, buffer.Binding, buffer.Size);
    }
    for (const auto& resource : m_SprvReflectionData.uniform_buffers)
    {
        const auto& bufferType = compiler.get_type(resource.base_type_id);
        std::uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        std::uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        std::size_t seed = set, hash = binding; glm::detail::hash_combine(seed, hash);
        std::string name = (resource.name.size()) ? resource.name : std::to_string(seed);

        auto& buffer = m_ReflectedData[set].UniformBuffers[name];

        buffer.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        buffer.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        buffer.Size = compiler.get_declared_struct_size(bufferType);
        buffer.ShaderType = FromShaderTypeToVkShaderType(type);
        buffer.Name = name;

        SHADE_CORE_TRACE("   UniformBuffer: Name = {0}, Set = {1}, Binding = {2}, Size = {3}, ", buffer.Name, buffer.Set, buffer.Binding, buffer.Size);
    }

    for (const auto& resource : m_SprvReflectionData.sampled_images)
    {
        const auto& bufferType = compiler.get_type(resource.base_type_id);
        std::uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        std::uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        std::size_t seed = set, hash = binding; glm::detail::hash_combine(seed, hash);
        std::string name = (resource.name.size()) ? resource.name : std::to_string(seed);

        auto& image = m_ReflectedData[set].ImageSamplers[name];

        image.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        image.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        image.ShaderType = FromShaderTypeToVkShaderType(type);
        image.Name = name;

        SHADE_CORE_TRACE("   SampledImage: Name = {0}, Set = {1}, Binding = {2}", image.Name, image.Set, image.Binding);
    }
    for (const auto& resource : m_SprvReflectionData.storage_images)
    {
        const auto& bufferType = compiler.get_type(resource.base_type_id);
        std::uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        std::uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        std::size_t seed = set, hash = binding; glm::detail::hash_combine(seed, hash);
        std::string name = (resource.name.size()) ? resource.name : std::to_string(seed);

        auto& image = m_ReflectedData[set].ImageStorage[name];

        image.Binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        image.Set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        image.ShaderType = FromShaderTypeToVkShaderType(type);
        image.Name = name;

        SHADE_CORE_TRACE("   StorageImage: Name = {0}, Set = {1}, Binding = {2}", image.Name, image.Set, image.Binding);
    }
    for (const auto& resource : m_SprvReflectionData.push_constant_buffers)
    {
        const auto& bufferType = compiler.get_type(resource.base_type_id);
        std::uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        std::uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

        std::size_t seed = compiler.get_declared_struct_size(bufferType), hash = binding; glm::detail::hash_combine(seed, hash);
        std::string name = (resource.name.size()) ? resource.name : std::to_string(seed);

        auto& constant = m_ReflectedData[set].PushConstants[name];
        constant.Set = set;
        constant.Size = compiler.get_declared_struct_size(bufferType);
        constant.ShaderType = FromShaderTypeToVkShaderType(type);
        constant.Name = name;

        SHADE_CORE_TRACE("   Uniform: Name = {0}, Set = {1}, Size = {2}", constant.Name, constant.Set, constant.Size);
    }

}
shade::VulkanShader::~VulkanShader()
{
    for (auto& shader : m_PipelineShaderStageCreateInfo)
        vkDestroyShaderModule(VulkanContext::GetDevice()->GetLogicalDevice(), shader.module, VulkanContext::GetInstance().AllocationCallbaks); 
}

std::vector<VkPipelineShaderStageCreateInfo>& shade::VulkanShader::GetPipelineShaderStageCreateInfo()
{
    return m_PipelineShaderStageCreateInfo;
}

void shade::VulkanShader::CreateShader()
{
    auto& device = VulkanContext::GetDevice()->GetLogicalDevice();
    auto& instance = VulkanContext::GetInstance();
    for (auto& [type, shader] : m_VulkanSPIRV)
    {
        m_Stages |= FromShaderTypeToVkShaderType(type);
        /*  This VkPipelineShaderStageCreateInfo will hold information about a single shader stage for the pipeline. 
            We build it from a shader stage and a shader module. */
        m_PipelineShaderStageCreateInfo.emplace_back() = 
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType
            VK_NULL_HANDLE, // pNext
            0, // Flags should be 0
            FromShaderTypeToVkShaderType(type), // 
            CreateShaderModule(instance, device, shader), // Create shader module.
            "main", // Shader's entry point name, usual a main function.
            VK_NULL_HANDLE 
        };

        // Set debug name.
        VKUtils::SetDebugObjectName(instance.Instance, utils::ShaderTypeToString(type), device, VK_OBJECT_TYPE_SHADER_MODULE, m_PipelineShaderStageCreateInfo.back().module);
    } 
}

VkShaderModule shade::VulkanShader::CreateShaderModule(const VulkanContext::VulkanInstance& instance, VkDevice device, const std::vector<std::uint32_t>& code)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo
    {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // sType
        VK_NULL_HANDLE, // pNext
        0, // flags
        sizeof(std::uint32_t) * code.size(), // codeSize
        code.data() // pCode
    };
	VkShaderModule shaderModule; VK_CHECK_RESULT(vkCreateShaderModule(device, &shaderModuleCreateInfo, instance.AllocationCallbaks, &shaderModule), "Failed to crate shader module!");
	return shaderModule;
}

VkShaderStageFlagBits shade::VulkanShader::FromShaderTypeToVkShaderType(const shade::Shader::Type& type)
{
    switch (type)
    {
    case shade::Shader::Type::Vertex:    return VK_SHADER_STAGE_VERTEX_BIT;
    case shade::Shader::Type::Fragment:  return VK_SHADER_STAGE_FRAGMENT_BIT;
    case shade::Shader::Type::Geometry:  return VK_SHADER_STAGE_GEOMETRY_BIT;
    case shade::Shader::Type::Compute:   return VK_SHADER_STAGE_COMPUTE_BIT;
    default: return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM; // Undefined 
    }
}

void shade::VulkanShader::Recompile()
{

}

VkShaderStageFlags shade::VulkanShader::GetStages() const
{
    return m_Stages;
}

VkFormat shade::VulkanShader::GetShaderDataToVulkanFormat(const Shader::DataType& type)
{
    switch (type)
    {
    case Shader::DataType::Float:     return VK_FORMAT_R32_SFLOAT;
    case Shader::DataType::Float2:    return VK_FORMAT_R32G32_SFLOAT;
    case Shader::DataType::Float3:    return VK_FORMAT_R32G32B32_SFLOAT;
    case Shader::DataType::Float4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Shader::DataType::Mat4:      return VK_FORMAT_R32G32B32A32_SFLOAT;
    default: return VK_FORMAT_UNDEFINED;
    }
}
std::map<std::uint32_t, shade::VulkanShader::ShaderResources>& shade::VulkanShader::GetReflectedData()
{
    return m_ReflectedData;
}
