#include "shade_pch.h"
#include "VulkanFrameBuffer.h"
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>

shade::VulkanFrameBuffer::VulkanFrameBuffer(const VkDevice& device, const VulkanContext::VulkanInstance& instance, const FrameBuffer::Specification& specification) :
	m_VkDevice(device), m_VkInstance(instance)
{
	Invalidate(specification, nullptr);
}
shade::VulkanFrameBuffer::VulkanFrameBuffer(const VkDevice& device, const VulkanContext::VulkanInstance& instance, const FrameBuffer::Specification& specification, const std::vector<SharedPointer<render::Image2D>>& images) :
	m_VkDevice(device), m_VkInstance(instance)
{
	Invalidate(specification, &images);
}

void shade::VulkanFrameBuffer::Resize(std::uint32_t width, uint32_t height)
{
	m_Specification.Width = width;
	m_Specification.Height = height;
	Invalidate(m_Specification, nullptr);
}

void shade::VulkanFrameBuffer::Resize(std::uint32_t width, uint32_t height, const std::vector<SharedPointer<render::Image2D>>& images)
{
	m_Specification.Width = width;
	m_Specification.Height = height;
	Invalidate(m_Specification, &images);
}

void shade::VulkanFrameBuffer::Invalidate(const FrameBuffer::Specification& specification, const std::vector<SharedPointer<render::Image2D>>* images)
{
	m_Specification = specification; 
	m_TextureAttachments.clear();
	m_DepthAttachments.clear();

	m_ColorAttachmentsViews.clear();
	m_DepthAttachmentsViews.clear();

	m_RenderingColorAttachmentInfos.clear();
	m_ClearAttachments.clear(); 
	m_AttachmentDescriptions.clear();
	
	std::uint32_t colorAttachmentCount = 0, depthAttachmentCount = 0;

	if (images) 
	{
		for (auto i = 0; i < m_Specification.Attachments.TextureAttachments.size(); i++) 
		{
			//m_Specification.Attachments.TextureAttachments[i] = (*images)[i]->GetSpecification();
			// if it is not a depth format attachment
			if (!VKUtils::IsDepthFormat(m_Specification.Attachments.TextureAttachments[i].Format) && !VKUtils::IsDepthStencilFormat(m_Specification.Attachments.TextureAttachments[i].Format))
			{
				// increment colorAttachmentCount
				++colorAttachmentCount; 
				m_TextureAttachments.emplace_back(Texture2D::CreateEXP((*images)[i]));
				m_ColorAttachmentsViews.emplace_back(m_TextureAttachments.back()->GetImage()->As<VulkanImage2D>().GetImageView());
			}
			else // if it is a depth format attachment
			{
				// increment depthAttachmentCount
				++depthAttachmentCount;
				m_DepthAttachments.emplace_back(Texture2D::CreateEXP((*images)[i]));
				m_DepthAttachmentsViews.emplace_back(m_DepthAttachments.back()->GetImage()->As<VulkanImage2D>().GetImageView());
			}
		}
	}
	else
	{
		for (auto i = 0; i < m_Specification.Attachments.TextureAttachments.size(); i++) 
		{
			m_Specification.Attachments.TextureAttachments[i].Width  = m_Specification.Width;
			m_Specification.Attachments.TextureAttachments[i].Height = m_Specification.Height;
			m_Specification.Attachments.TextureAttachments[i].Usage  = render::Image::Usage::Attachment;
			// if it is not a depth format attachment
			if (!VKUtils::IsDepthFormat(m_Specification.Attachments.TextureAttachments[i].Format) && !VKUtils::IsDepthStencilFormat(m_Specification.Attachments.TextureAttachments[i].Format))
			{
				// increment colorAttachmentCount
				++colorAttachmentCount;
				m_TextureAttachments.emplace_back(Texture2D::CreateEXP(render::Image2D::Create(m_Specification.Attachments.TextureAttachments[i])));
				m_ColorAttachmentsViews.emplace_back(m_TextureAttachments.back()->GetImage()->As<VulkanImage2D>().GetImageView());
			}
			else // if it is a depth format attachment
			{
				// increment depthAttachmentCount
				++depthAttachmentCount;
				m_DepthAttachments.emplace_back(Texture2D::CreateEXP(render::Image2D::Create(m_Specification.Attachments.TextureAttachments[i])));
				m_DepthAttachmentsViews.emplace_back(m_DepthAttachments.back()->GetImage()->As<VulkanImage2D>().GetImageView());
			}
		}

	}

	m_RenderingColorAttachmentInfos.resize(colorAttachmentCount); 
	m_RenderingDepthAttachmentInfos.resize(depthAttachmentCount);

	m_ClearAttachments.resize(colorAttachmentCount); // Depth vill be adde later

	for (auto i = 0; i < m_RenderingColorAttachmentInfos.size(); i++) 
	{
		m_RenderingColorAttachmentInfos[i] = 
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = VK_NULL_HANDLE,
			.imageView = m_ColorAttachmentsViews[i],
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.resolveImageView = VK_NULL_HANDLE, 
			.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue =
			{
				m_Specification.ClearColor.r,
				m_Specification.ClearColor.g,
				m_Specification.ClearColor.b,
				m_Specification.ClearColor.a
			}
		};

		m_ClearAttachments[i] = 
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.colorAttachment = static_cast<std::uint32_t>(i),
			.clearValue = m_RenderingColorAttachmentInfos[i].clearValue,
		};
	}

	for (auto i = 0; i < m_RenderingDepthAttachmentInfos.size(); i++)
	{
		m_RenderingDepthAttachmentInfos[i] =
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = VK_NULL_HANDLE,
			.imageView = m_DepthAttachmentsViews[i],
			.imageLayout = (VKUtils::IsDepthFormat(m_DepthAttachments[i]->GetImage()->GetSpecification().Format)) ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.resolveImageView = VK_NULL_HANDLE,
			.resolveImageLayout = (VKUtils::IsDepthFormat(m_DepthAttachments[i]->GetImage()->GetSpecification().Format)) ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = m_Specification.DepthClearValue,
		};
		// Depth should be always at the end of array
		m_ClearAttachments.emplace_back() =
		{
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.colorAttachment = static_cast<std::uint32_t>(m_ClearAttachments.size() - 1), // No one garanti that this will be coorect !
			.clearValue = m_RenderingDepthAttachmentInfos[i].clearValue,
		};
	}

	m_RenderingInfo =
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.renderArea = { { 0, 0 }, { m_Specification.Width, m_Specification.Height }},
		// If only depth exist get depth as layered, if color attahcment exist take the first attachment as layered !
		.layerCount = (!colorAttachmentCount && depthAttachmentCount) ? m_DepthAttachments[0]->GetImage()->GetSpecification().Layers : m_TextureAttachments[0]->GetImage()->GetSpecification().Layers,
		.viewMask = 0,
		.colorAttachmentCount	= colorAttachmentCount,
		.pColorAttachments		= (colorAttachmentCount) ? m_RenderingColorAttachmentInfos.data() : VK_NULL_HANDLE,
		.pDepthAttachment		= (depthAttachmentCount) ? m_RenderingDepthAttachmentInfos.data() : VK_NULL_HANDLE,
		.pStencilAttachment		= VK_NULL_HANDLE
	};
}

shade::VulkanFrameBuffer::~VulkanFrameBuffer()
{
	m_ColorAttachmentsViews.clear();
	m_DepthAttachmentsViews.clear();

	m_TextureAttachments.clear();
	m_DepthAttachments.clear();

	m_RenderingColorAttachmentInfos.clear();
	m_ClearAttachments.clear();
	m_AttachmentDescriptions.clear();
}

const std::vector<VkImageView>& shade::VulkanFrameBuffer::GetAttachmentViews() const
{
	return m_ColorAttachmentsViews;
}

const VkRenderingInfo& shade::VulkanFrameBuffer::GetRenderingInfo() const
{
	return m_RenderingInfo;
}

shade::SharedPointer<shade::Texture2D>& shade::VulkanFrameBuffer::GetTextureAttachment(std::uint32_t index)
{
	if (index >= m_TextureAttachments.size())
		SHADE_CORE_ERROR("Wrong image attahcment index!");

	return m_TextureAttachments.at(index);
}

shade::SharedPointer<shade::Texture2D>& shade::VulkanFrameBuffer::GetDepthAttachment(std::uint32_t index)
{
	if (index >= m_DepthAttachments.size())
		SHADE_CORE_ERROR("Wrong depth attahcment index!");

	return m_DepthAttachments.at(index);
}

std::vector<shade::SharedPointer<shade::Texture2D>>& shade::VulkanFrameBuffer::GetTextureAttachments()
{
	return m_TextureAttachments;
}

std::vector<shade::SharedPointer<shade::Texture2D>>& shade::VulkanFrameBuffer::GetDepthAttachments()
{
	return m_DepthAttachments;
}

std::uint32_t shade::VulkanFrameBuffer::GetWidth() const
{
	return m_Specification.Width;
}

std::uint32_t shade::VulkanFrameBuffer::GetHeight() const
{
	return m_Specification.Height;
}

const std::vector<VkClearAttachment>& shade::VulkanFrameBuffer::GetClearAttachments() const
{
	return m_ClearAttachments;
}

void shade::VulkanFrameBuffer::AllocateImageMemory(VkDevice device, VkImage image)
{
	auto& instance = VulkanContext::GetInstance();
	// Memory allocation
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		VK_NULL_HANDLE,
		memRequirements.size,
		VKUtils::FindMemoryType(VulkanContext::GetPhysicalDevice()->GetDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	VkDeviceMemory imageMemory;
	VK_CHECK_RESULT(vkAllocateMemory(device, &memoryAllocateInfo, VulkanContext::GetInstance().AllocationCallbaks, &imageMemory), "Failed to allocate image memory!");
	VKUtils::SetDebugObjectName(instance.Instance, std::format("{0}:{1}", "Frame buffer image memory", (std::uint32_t)imageMemory), device, VK_OBJECT_TYPE_DEVICE_MEMORY, imageMemory);
	vkBindImageMemory(device, image, imageMemory, 0);
}

