#include "shade_pch.h"
#include "VertexBuffer.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/buffers/VulkanVertexBuffer.h>

shade::SharedPointer<shade::VertexBuffer> shade::VertexBuffer::Create(Usage usage, std::uint32_t size, std::size_t resizeThreshold, const void* data)
{
	switch (RenderAPI::GetCurrentAPI())
	{
	case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	case RenderAPI::API::Vulkan: return SharedPointer<VulkanVertexBuffer>::Create(VulkanContext::GetLogicalDevice()->GetDevice(), VulkanContext::GetInstance(), usage, size, resizeThreshold, data);
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

std::uint32_t shade::VertexBuffer::Layout::GetStride(std::size_t layout)
{
	return m_Strides[layout];
}

std::uint32_t shade::VertexBuffer::Layout::GetCount()
{
	std::uint32_t count = 0;
	for (const auto& elementLayout : m_ElementLayouts)
		for (const auto& element : elementLayout.Elements)
			count++;

	return count;
}

const std::vector<shade::VertexBuffer::Layout::ElementsLayout>& shade::VertexBuffer::Layout::GetElementLayouts() const
{
	return m_ElementLayouts;
}

void shade::VertexBuffer::Layout::ComputeOffsetAndStride()
{
	m_Strides.resize(m_ElementLayouts.size(), 0u);

	for (std::uint32_t layoutIndex = 0; layoutIndex < m_ElementLayouts.size(); ++layoutIndex)
	{
		std::uint32_t offset = 0;

		for (auto& element : m_ElementLayouts[layoutIndex].Elements)
		{
			element.Offset = offset;
			offset += element.Size;
			m_Strides[layoutIndex] += element.Size;
		}
	}

	/*std::uint32_t perVertexOffset = 0, perInstanceOffset = 0;
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
	}*/
}