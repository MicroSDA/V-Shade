#include "shade_pch.h"
#include "VertexBuffer.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/buffers/VulkanVertexBuffer.h>

shade::SharedPointer<shade::VertexBuffer> shade::VertexBuffer::Create(Usage usage, std::uint32_t size, std::size_t resizeThreshold, const void* data)
{
	switch (RenderAPI::GetCurrentAPI())
	{
	case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	case RenderAPI::API::Vulkan: return SharedPointer<VulkanVertexBuffer>::Create(VulkanContext::GetDevice()->GetLogicalDevice(), VulkanContext::GetInstance(), usage, size, resizeThreshold, data);
	default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

std::uint32_t shade::VertexBuffer::GetSize() const
{
	return m_Size;
}

bool shade::VertexBuffer::HasToBeResized(std::uint32_t oldSize, std::uint32_t newSize, std::uint32_t threshold)
{
	// Checks if the new size is larger than the old size
	if (oldSize < newSize)
	{
		return true;
	}
	// Checks if the increase in size is more than the given threshold percentage of old size
	else if (oldSize > (newSize + ((oldSize / 100) * threshold)))
	{
		return true;
	}

	// If none of the above conditions are no true, returns false
	return false;
}

std::uint32_t shade::VertexBuffer::Layout::Element::GetComponentCount(Shader::DataType type)
{
	switch (type)
	{
	case Shader::DataType::Float:   return 1;
	case Shader::DataType::Float2:  return 2;
	case Shader::DataType::Float3:  return 3;
	case Shader::DataType::Float4:  return 4;
	case Shader::DataType::Mat3:    return 3 * 3;
	case Shader::DataType::Mat4:    return 4 * 4;
	case Shader::DataType::Int:     return 1;
	case Shader::DataType::Int2:    return 2;
	case Shader::DataType::Int3:    return 3;
	case Shader::DataType::Int4:    return 4;
	case Shader::DataType::Bool:    return 1;
	default: return 0;
	}
}

std::uint32_t shade::VertexBuffer::Layout::GetStride(Layout::Usage usage)
{
	switch (usage)
	{
	case shade::VertexBuffer::Layout::Usage::PerVertex:return m_PerVertexStride;
	case shade::VertexBuffer::Layout::Usage::PerInstance:return m_PerInstanceStride;
	default:
		return 0;
	}
}

std::uint32_t shade::VertexBuffer::Layout::GetCount()
{
	return static_cast<std::uint32_t>(m_Elements.size());
}

const std::vector<shade::VertexBuffer::Layout::Element>& shade::VertexBuffer::Layout::GetElements() const
{
	return m_Elements;
}

void shade::VertexBuffer::Layout::ComputeOffsetAndStride()
{
	std::uint32_t perVertexOffset = 0, perInstanceOffset = 0;
	m_PerVertexStride = 0, m_PerInstanceStride = 0;

	for (auto& element : m_Elements)
	{
		if (element.Usage == Usage::PerVertex)
		{
			element.Offset = perVertexOffset;
			perVertexOffset += element.Size;
			m_PerVertexStride += element.Size;
		}
		if (element.Usage == Usage::PerInstance)
		{
			element.Offset = perInstanceOffset;
			perInstanceOffset += element.Size;
			m_PerInstanceStride += element.Size;
		}
	}
}