#pragma once
#include <shade/core/render/shader/Shader.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace shade
{
	namespace shader_resource
	{
		struct StorageBuffer
		{
			std::uint32_t Binding;
			std::uint32_t Set;
			std::uint32_t Size = 1;
			std::string Name;
			VkShaderStageFlagBits ShaderType = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};
		struct UniformBuffer
		{
			std::uint32_t Binding;
			std::uint32_t Set;
			std::uint32_t Size = 1;
			std::string Name;
			VkShaderStageFlagBits ShaderType = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};
		struct ImageSampler
		{
			std::uint32_t Binding;
			std::uint32_t Set;
			std::uint32_t Size = 1;
			std::string Name;
			VkShaderStageFlagBits ShaderType = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};
		struct ImageStorage
		{
			std::uint32_t Binding;
			std::uint32_t Set;
			std::uint32_t Size = 1;
			std::string Name;
			VkShaderStageFlagBits ShaderType = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};
		struct PushConstant
		{
			std::uint32_t Set;
			std::uint32_t Size = 1;
			std::string Name;
			VkShaderStageFlagBits ShaderType = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};
	};
	class VulkanShader : public Shader
	{
	public:
		struct ShaderResources
		{
			// Name -> Binding -> Buffer
			std::unordered_map<std::string, shader_resource::StorageBuffer> StorageBuffers;
			std::unordered_map<std::string, shader_resource::UniformBuffer> UniformBuffers;
			std::unordered_map<std::string, shader_resource::ImageSampler>  ImageSamplers;
			std::unordered_map<std::string, shader_resource::ImageStorage>  ImageStorage;
			std::unordered_map<std::string, shader_resource::PushConstant>  PushConstants;
		};
		//TIP: for ssbo for now
		struct ReflectedData 
		{
			Shader::Type ShaderType;
			std::uint32_t Binding;
			std::uint32_t Size;
			std::string Name;
		};
	public:
		VulkanShader(const std::string& filePath);
		virtual ~VulkanShader();

		std::vector<VkPipelineShaderStageCreateInfo>& GetPipelineShaderStageCreateInfo();
		// TODO: Need to expance more!
		static VkFormat GetShaderDataToVulkanFormat(const Shader::DataType& type);
		std::map<std::uint32_t, ShaderResources>& GetReflectedData();
		static VkShaderStageFlagBits FromShaderTypeToVkShaderType(const shade::Shader::Type& type);

		virtual void Recompile() override;

		VkShaderStageFlags GetStages() const;
		virtual void TryToFindInCacheAndCompile() override;
	private:
		void CreateShader();
		VkShaderModule CreateShaderModule(const VulkanContext::VulkanInstance& instance,  VkDevice device, const std::vector<std::uint32_t>& code);
	private:
		std::vector<VkPipelineShaderStageCreateInfo> m_PipelineShaderStageCreateInfo;
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
		std::vector<VkDescriptorSet> m_DescriptorSet;
		VkShaderStageFlags m_Stages = 0;


		spirv_cross::ShaderResources m_SprvReflectionData;
		// Set -> Data
		std::map<std::uint32_t, ShaderResources> m_ReflectedData;
		std::unordered_map<Shader::Type, std::vector<uint32_t>> m_VulkanSPIRV;
	private:
		void Reflect(Shader::Type type, const std::vector<uint32_t>& shaderData);
		
	};

}