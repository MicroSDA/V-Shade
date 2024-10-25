#include "shade_pch.h"
#include "VulkanImage2D.h"

#include <shade/core/render/RenderAPI.h>

shade::VulkanImage2D::VulkanImage2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, const render::Image::Specification& specification) :
	m_VkDevice(device), m_VkInstance(instnace)
{
	Invalidate(specification);
}

shade::VulkanImage2D::VulkanImage2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, render::Image& image) :
	m_VkDevice(device), m_VkInstance(instnace)
{
	Invalidate(image);
}

shade::VulkanImage2D::VulkanImage2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, const render::Image::Specification& specification, const void* source) :
	m_VkDevice(device), m_VkInstance(instnace)
{
	Invalidate(specification, source);
}

void shade::VulkanImage2D::Invalidate(const render::Image::Specification& specification)
{
	m_Specification					= specification;
	// Make sure that mips count is in range !
	const std::uint32_t MaxMip		= std::uint32_t(std::floor(std::log2(std::max(m_Specification.Width, m_Specification.Height))) + 1);
	m_Specification.MipLevels		= (m_Specification.MipLevels <= MaxMip) ? m_Specification.MipLevels : MaxMip;

	m_ImageLayout					= VK_IMAGE_LAYOUT_UNDEFINED;

	m_IsDepth						= VKUtils::IsDepthFormat(m_Specification.Format);
	m_IsDepthStencil				= VKUtils::IsDepthStencilFormat(m_Specification.Format);
	m_ImageFormat					= VKUtils::ToVulkanImageFormat(m_Specification.Format);
	// Determine the image aspects to be used, whether it is for color or depth

	//m_AspectFlags = (m_IsDepth) ? VK_IMAGE_ASPECT_DEPTH_BIT : (m_IsDepthStencil) ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	m_AspectFlags = m_AspectFlags = (m_IsDepth || m_IsDepthStencil) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;;

	VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;

	if (m_Specification.Usage == render::Image::Usage::Attachment)
	{
		if (m_IsDepth || m_IsDepthStencil) 
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	}
	else if (m_Specification.Usage == render::Image::Usage::Texture)
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	else if (m_Specification.Usage == render::Image::Usage::Storage)
	{
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	// Set image create info
	VkImageCreateInfo imageCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = (m_Specification.IsCubeMap) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : static_cast<VkImageCreateFlags>(0x0),
		.imageType = VK_IMAGE_TYPE_2D,
		.format = m_ImageFormat,
		.extent = { m_Specification.Width, m_Specification.Height, 1 },
		.mipLevels = m_Specification.MipLevels,
		.arrayLayers = m_Specification.Layers,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usageFlags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE, // TODO: Take a look !!
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = VK_NULL_HANDLE,
		.initialLayout = m_ImageLayout
	};

	// Create image
	VK_CHECK_RESULT(vkCreateImage(m_VkDevice, &imageCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImage), "Failed to create image!");

	// Set image debug name
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Image", (std::uint32_t)&m_VkImage), m_VkDevice, VK_OBJECT_TYPE_IMAGE, m_VkImage);

	// Get memory requirements
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_VkDevice, m_VkImage, &memRequirements);

	// Set memory allocation info
	VkMemoryAllocateInfo memoryAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = VKUtils::FindMemoryType(VulkanContext::GetPhysicalDevice()->GetDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	// Allocate memory
	VK_CHECK_RESULT(vkAllocateMemory(m_VkDevice, &memoryAllocateInfo, VulkanContext::GetInstance().AllocationCallbaks, &m_VkImageMemory), "Failed to allocate image memory!");

	// Set image memory debug name
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Image memory", (std::uint32_t)&m_VkImageMemory), m_VkDevice, VK_OBJECT_TYPE_DEVICE_MEMORY, m_VkImageMemory);

	// Bind image memory
	vkBindImageMemory(m_VkDevice, m_VkImage, m_VkImageMemory, 0);

	auto commandBuffer = RenderCommandBuffer::Create(RenderCommandBuffer::Type::Primary, RenderCommandBuffer::Family::Transfer);

	switch (m_Specification.Usage)
	{
		case render::Image::Usage::Texture:
		{
			// If it is texture we need to set layout to shader read only
			LayoutTransition(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT,

				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,

				VK_IMAGE_ASPECT_COLOR_BIT,
				0, m_Specification.MipLevels, 0, m_Specification.Layers);
			break;
		}
		case render::Image::Usage::Attachment:
		{
			LayoutTransition(commandBuffer, (m_IsDepth) ? VK_IMAGE_LAYOUT_GENERAL : (m_IsDepthStencil) ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_GENERAL, // VK_IMAGE_LAYOUT_GENERAL
				VK_ACCESS_NONE,
				VK_ACCESS_NONE,

				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,

				(m_IsDepth) ? VK_IMAGE_ASPECT_DEPTH_BIT : (m_IsDepthStencil) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT,
				0, m_Specification.MipLevels, 0, m_Specification.Layers);
			break;
		}
		case render::Image::Usage::Storage: // TODO: Finish for storage
		{
			LayoutTransition(commandBuffer, VK_IMAGE_LAYOUT_GENERAL, // VK_IMAGE_LAYOUT_GENERAL
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_TRANSFER_READ_BIT,

				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,

				(m_IsDepth) ? VK_IMAGE_ASPECT_DEPTH_BIT : (m_IsDepthStencil) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT,
				0, m_Specification.MipLevels, 0, m_Specification.Layers);
			break;
		}
	default: SHADE_CORE_ERROR("Invalid Image Usage parametr = 'None'!"); break;
	}
	
	// Set a debug object name for the VKUtils object instance being used 
	// This is done to help debugging if the program fails 
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Image", (std::uint32_t)&m_VkImage), m_VkDevice, VK_OBJECT_TYPE_IMAGE, m_VkImage);

	// Create a struct containing information needed to create an image view   
	VkImageViewCreateInfo imageViewCreateInfo
	{
	   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // The current structure type
	   .pNext = VK_NULL_HANDLE, // A pointer to the next structure if there's any other info to be filled
	   .flags = 0, // Optional flags
	   .image = m_VkImage, // The VkImage to create an image view for 
	   .viewType = (m_Specification.Layers > 1) ? (m_Specification.IsCubeMap) ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_2D_ARRAY : (m_Specification.IsCubeMap) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D, // If the image has layers, create a 2D ImageArray, else, create a 2D image view 
	   .format = m_ImageFormat, // The format of the image view to be created 
	   .components = // The component map of the view
		  {
			.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A
		  },
	   .subresourceRange =  // The range of the subresource to be accessed
		  {
			.aspectMask = m_AspectFlags, // The aspect of the subresource to access, which could be COLOR, or DEPTH
			.baseMipLevel = 0,  // Starting point of the level of the mip to access
			.levelCount = m_Specification.MipLevels, // Number of levels in the mip chain
			.baseArrayLayer = 0, // Starting point of the layer to be accessed
			.layerCount = m_Specification.Layers // Number of layers to be accessed 
		  },
	};

	// Create the image view based on the info passed
	VK_CHECK_RESULT(vkCreateImageView(m_VkDevice, &imageViewCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageView), "Failed to create image view!");
	// Set a debug object name for the VKUtils object instance being used 
	// This is done to help debugging if the program fails
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge view", (std::uint32_t)&m_VkImageView), m_VkDevice, VK_OBJECT_TYPE_IMAGE_VIEW, m_VkImageView);

	// Create view per mip level;
	for (std::uint32_t i = 0; i < m_Specification.MipLevels; i++)
	{
		// Create a struct containing information needed to create an image view   
		VkImageViewCreateInfo imageViewCreateInfo
		{
		   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // The current structure type
		   .pNext = VK_NULL_HANDLE, // A pointer to the next structure if there's any other info to be filled
		   .flags = 0, // Optional flags
		   .image = m_VkImage, // The VkImage to create an image view for 
		   .viewType = (m_Specification.Layers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, // If the image has layers, create a 2D ImageArray, else, create a 2D image view 
		   .format = m_ImageFormat, // The format of the image view to be created 
		   .components = // The component map of the view
			  {
				.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A
			  },
		   .subresourceRange =  // The range of the subresource to be accessed
			  {
				.aspectMask = m_AspectFlags, // The aspect of the subresource to access, which could be COLOR, or DEPTH
				.baseMipLevel = i,  // Starting point of the level of the mip to access
				.levelCount = 1, // Number of levels in the mip chain
				.baseArrayLayer = 0, // Starting point of the layer to be accessed
				.layerCount = m_Specification.Layers // Number of layers to be accessed 
			  }
		};

		// Create the image view based on the info passed
		VK_CHECK_RESULT(vkCreateImageView(m_VkDevice, &imageViewCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageViewsMips.emplace_back()), "Failed to create image view!");
		// Set a debug object name for the VKUtils object instance being used 
		// This is done to help debugging if the program fails
		VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge view", (std::uint32_t)&m_VkImageViewsMips.back()), m_VkDevice, VK_OBJECT_TYPE_IMAGE_VIEW, m_VkImageViewsMips.back());
	}
	// Create view per layer;
	for (std::uint32_t i = 0; i < m_Specification.Layers; i++)
	{
		// Create a struct containing information needed to create an image view   
		VkImageViewCreateInfo imageViewCreateInfo
		{
		   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // The current structure type
		   .pNext = VK_NULL_HANDLE, // A pointer to the next structure if there's any other info to be filled
		   .flags = 0, // Optional flags
		   .image = m_VkImage, // The VkImage to create an image view for 
		   .viewType = VK_IMAGE_VIEW_TYPE_2D,
		   .format = m_ImageFormat, // The format of the image view to be created 
		   .components = // The component map of the view
			  {
				.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A
			  },
		   .subresourceRange =  // The range of the subresource to be accessed
			  {
				.aspectMask = m_AspectFlags,	// The aspect of the subresource to access, which could be COLOR, or DEPTH
				.baseMipLevel = 0,				// Starting point of the level of the mip to access
				.levelCount = 1,				// Number of levels in the mip chain
				.baseArrayLayer = i,			// Starting point of the layer to be accessed
				.layerCount = 1					// Number of layers to be accessed 
			  }
		};

		// Create the image view based on the info passed
		VK_CHECK_RESULT(vkCreateImageView(m_VkDevice, &imageViewCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageViewsLayers.emplace_back()), "Failed to create image view!");
		// Set a debug object name for the VKUtils object instance being used 
		// This is done to help debugging if the program fails
		VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge view", (std::uint32_t)&m_VkImageViewsLayers.back()), m_VkDevice, VK_OBJECT_TYPE_IMAGE_VIEW, m_VkImageViewsLayers.back());
	}
}

void shade::VulkanImage2D::Invalidate(render::Image& image)
{
	// Get image data from image
	m_ImageData					= image.GetImageData();

	m_ImageLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	m_ImageFormat	= VK_FORMAT_MAX_ENUM;
	// Set image specifications
	m_Specification.Width		= m_ImageData.Width;
	m_Specification.Height		= m_ImageData.Height;
	m_Specification.Layers		= 1;
	m_Specification.MipLevels	= m_ImageData.MipMapCount;
	m_Specification.Usage		= render::Image::Usage::Texture;
	m_Specification.Format		= render::Image::Format::RGBA;

	// Determine the image aspects to be used, whether it is for color or depth

	m_AspectFlags = (m_IsDepth) ? VK_IMAGE_ASPECT_DEPTH_BIT : (m_IsDepthStencil) ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;

	std::uint32_t blockSize = 0;

	// Set image format and block size based on data compression
	switch (m_ImageData.Compression)
	{
	case render::Image::ImageData::DXTCompression::DXT1:
		if (m_ImageData.HasAlpha)
		{
			m_ImageFormat = (m_ImageData.SRGB) ? VK_FORMAT_BC1_RGBA_SRGB_BLOCK : VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		}
		else
		{
			m_ImageFormat = (m_ImageData.SRGB) ? VK_FORMAT_BC1_RGB_SRGB_BLOCK : VK_FORMAT_BC1_RGB_UNORM_BLOCK;
		}
		blockSize = 8;
		break;
	case render::Image::ImageData::DXTCompression::DXT3:
		m_ImageFormat = (m_ImageData.SRGB) ? VK_FORMAT_BC2_SRGB_BLOCK : VK_FORMAT_BC2_UNORM_BLOCK;
		blockSize = 16;
		break;
	case render::Image::ImageData::DXTCompression::DXT5:
		m_ImageFormat = (m_ImageData.SRGB) ? VK_FORMAT_BC3_SRGB_BLOCK : VK_FORMAT_BC3_UNORM_BLOCK;
		blockSize = 16;
		break;
		// TODO: Need test this
	case render::Image::ImageData::DXTCompression::BC5LU:
		m_ImageFormat = VK_FORMAT_BC5_UNORM_BLOCK;
		blockSize = 16;
		break;
	case render::Image::ImageData::DXTCompression::BC5LS:
		m_ImageFormat = VK_FORMAT_BC5_SNORM_BLOCK;
		blockSize = 16;
		break;
	default:
		SHADE_CORE_WARNING("Unsupported texture format in '{0}'.", m_ImageData.Compression);
		return;
	}
	

	if (m_Specification.Usage == render::Image::Usage::Attachment)
	{
		if (m_IsDepth || m_IsDepthStencil)
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	}
	else if (m_Specification.Usage == render::Image::Usage::Texture)
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	// Set image create info
	VkImageCreateInfo imageCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = m_ImageFormat,
		.extent = { m_Specification.Width, m_Specification.Height, 1 },
		.mipLevels = m_Specification.MipLevels,
		.arrayLayers = m_Specification.Layers,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usageFlags,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = VK_NULL_HANDLE,
		.initialLayout = m_ImageLayout
	};

	/*vkCmdUpdateImage();
	vkCmdCopyBufferToImage()*/
	// Create image
	VK_CHECK_RESULT(vkCreateImage(m_VkDevice, &imageCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImage), "Failed to create image!");

	// Set image debug name
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Image", (std::uint32_t)&m_VkImage), m_VkDevice, VK_OBJECT_TYPE_IMAGE, m_VkImage);

	// Get memory requirements
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_VkDevice, m_VkImage, &memRequirements);

	// Set memory allocation info
	VkMemoryAllocateInfo memoryAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = VKUtils::FindMemoryType(VulkanContext::GetPhysicalDevice()->GetDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	// Allocate memory
	VK_CHECK_RESULT(vkAllocateMemory(m_VkDevice, &memoryAllocateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageMemory), "Failed to allocate image memory!");

	// Set image memory debug name
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Image memory", (std::uint32_t)&m_VkImageMemory), m_VkDevice, VK_OBJECT_TYPE_DEVICE_MEMORY, m_VkImageMemory);

	// Bind image memory
	vkBindImageMemory(m_VkDevice, m_VkImage, m_VkImageMemory, 0);

	{
		// Create a buffer for use in the device with properties to be used as a index buffer.
		VkBufferCreateInfo stagingbufferCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.size = static_cast<std::uint64_t>(m_ImageData.Size),
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = VK_NULL_HANDLE
		};

		// Create a VK buffer for staging and allocate memory to it
		auto buffer = CreateBuffer(m_VkDevice, m_VkInstance.AllocationCallbaks, &stagingbufferCreateInfo,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VkBuffer stagingBuffer = std::get<0>(buffer);
		VkDeviceMemory stagingBufferMemory = std::get<1>(buffer);

		// Map the staging buffer memory
		void* data;
		vkMapMemory(m_VkDevice, stagingBufferMemory, 0, m_ImageData.Size, 0, &data);
		// Copy the texture data to the staging buffer memory (assuming you have the texture data in a byte array called 'textureData')
		memcpy(data, m_ImageData.Data, m_ImageData.Size);
		// Unmap the staging buffer memory

		vkUnmapMemory(m_VkDevice, stagingBufferMemory);

		auto commandBuffer = RenderCommandBuffer::Create(RenderCommandBuffer::Type::Primary, RenderCommandBuffer::Family::Transfer);
		// Transition the texture image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		LayoutTransition(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, m_AspectFlags, 0, m_ImageData.MipMapCount);

		std::vector<VkBufferImageCopy> bufferCopyRegions;
		std::uint32_t mipWidth = m_ImageData.Width, mipHeight = m_ImageData.Height, bufferOffset = 0;

		for (std::uint16_t i = 0; i < m_ImageData.MipMapCount; i++)
		{
			auto size = ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * blockSize;

			VkBufferImageCopy copyRegion
			{
				.bufferOffset = bufferOffset,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource =
					{
						.aspectMask = m_AspectFlags,
						.mipLevel = std::uint32_t(i),
						.baseArrayLayer = 0,
						.layerCount = 1,
					},
				.imageOffset = { 0, 0, 0 },
				.imageExtent = { mipWidth, mipHeight,  1 }
			};
			bufferCopyRegions.push_back(copyRegion);
			// Calculate the buffer offset for the next mip level
			bufferOffset += size; // Assuming RGBA8 format
			// Calculate the dimensions for the next mip level
			mipWidth = std::max(1u, mipWidth / 2);
			mipHeight = std::max(1u, mipHeight / 2);
		}

		commandBuffer->Begin();
		vkCmdCopyBufferToImage(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(), stagingBuffer, m_VkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<std::uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());
		commandBuffer->End();
		commandBuffer->Submit();

		// Transition the texture image layout to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		LayoutTransition(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, 0, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, m_AspectFlags, 0, m_ImageData.MipMapCount);

		// Clean up the staging buffer
		vkDestroyBuffer(m_VkDevice, stagingBuffer, m_VkInstance.AllocationCallbaks);
		vkFreeMemory(m_VkDevice, stagingBufferMemory, m_VkInstance.AllocationCallbaks);
	}

	// Set a debug object name for the VKUtils object instance being used 
	// This is done to help debugging if the program fails 
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Image", (std::uint32_t)&m_VkImage), m_VkDevice, VK_OBJECT_TYPE_IMAGE, m_VkImage);

	// Create a struct containing information needed to create an image view   
	VkImageViewCreateInfo imageViewCreateInfo
	{
	   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // The current structure type
	   .pNext = VK_NULL_HANDLE, // A pointer to the next structure if there's any other info to be filled
	   .flags = 0, // Optional flags
	   .image = m_VkImage, // The VkImage to create an image view for 
	   .viewType = (m_Specification.Layers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, // If the image has layers, create a 2D ImageArray, else, create a 2D image view 
	   .format = m_ImageFormat, // The format of the image view to be created 
	   .components = // The component map of the view
		  {
			.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A
		  },
	   .subresourceRange =  // The range of the subresource to be accessed
		  {
			.aspectMask = m_AspectFlags, // The aspect of the subresource to access, which could be COLOR, or DEPTH
			.baseMipLevel = 0,  // Starting point of the level of the mip to access
			.levelCount = m_Specification.MipLevels, // Number of levels in the mip chain
			.baseArrayLayer = 0, // Starting point of the layer to be accessed
			.layerCount = m_Specification.Layers // Number of layers to be accessed 
		  }
	};

	// Create the image view based on the info passed
	VK_CHECK_RESULT(vkCreateImageView(m_VkDevice, &imageViewCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageView), "Failed to create image view!");
	// Set a debug object name for the VKUtils object instance being used 
	// This is done to help debugging if the program fails
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge view", (std::uint32_t)&m_VkImageView), m_VkDevice, VK_OBJECT_TYPE_IMAGE_VIEW, m_VkImageView);

	// Create view per mip level;
	for (std::uint32_t i = 0; i < m_Specification.MipLevels; i++)
	{
		// Create a struct containing information needed to create an image view   
		VkImageViewCreateInfo imageViewCreateInfo
		{
		   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // The current structure type
		   .pNext = VK_NULL_HANDLE, // A pointer to the next structure if there's any other info to be filled
		   .flags = 0, // Optional flags
		   .image = m_VkImage, // The VkImage to create an image view for 
		   .viewType = (m_Specification.Layers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, // If the image has layers, create a 2D ImageArray, else, create a 2D image view 
		   .format = m_ImageFormat, // The format of the image view to be created 
		   .components = // The component map of the view
			  {
				.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A
			  },
		   .subresourceRange =  // The range of the subresource to be accessed
			  {
				.aspectMask = m_AspectFlags, // The aspect of the subresource to access, which could be COLOR, or DEPTH
				.baseMipLevel = i,  // Starting point of the level of the mip to access
				.levelCount = 1, // Number of levels in the mip chain
				.baseArrayLayer = 0, // Starting point of the layer to be accessed
				.layerCount = m_Specification.Layers // Number of layers to be accessed 
			  }
		};

		// Create the image view based on the info passed
		VK_CHECK_RESULT(vkCreateImageView(m_VkDevice, &imageViewCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageViewsMips.emplace_back()), "Failed to create image view!");
		// Set a debug object name for the VKUtils object instance being used 
		// This is done to help debugging if the program fails
		VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge view", (std::uint32_t)&m_VkImageViewsMips.back()), m_VkDevice, VK_OBJECT_TYPE_IMAGE_VIEW, m_VkImageViewsMips.back());
	}
	// Create view per layer;
	for (std::uint32_t i = 0; i < m_Specification.Layers; i++)
	{
		// Create a struct containing information needed to create an image view   
		VkImageViewCreateInfo imageViewCreateInfo
		{
		   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // The current structure type
		   .pNext = VK_NULL_HANDLE, // A pointer to the next structure if there's any other info to be filled
		   .flags = 0, // Optional flags
		   .image = m_VkImage, // The VkImage to create an image view for 
		   .viewType = VK_IMAGE_VIEW_TYPE_2D, // If the image has layers, create a 2D ImageArray, else, create a 2D image view 
		   .format = m_ImageFormat, // The format of the image view to be created 
		   .components = // The component map of the view
			  {
				.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A
			  },
		   .subresourceRange =  // The range of the subresource to be accessed
			  {
				.aspectMask = m_AspectFlags,	// The aspect of the subresource to access, which could be COLOR, or DEPTH
				.baseMipLevel = 0,				// Starting point of the level of the mip to access
				.levelCount = 1,				// Number of levels in the mip chain
				.baseArrayLayer = i,			// Starting point of the layer to be accessed
				.layerCount = 1					// Number of layers to be accessed 
			  }
		};

		// Create the image view based on the info passed
		VK_CHECK_RESULT(vkCreateImageView(m_VkDevice, &imageViewCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageViewsLayers.emplace_back()), "Failed to create image view!");
		// Set a debug object name for the VKUtils object instance being used 
		// This is done to help debugging if the program fails
		VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge view", (std::uint32_t)&m_VkImageViewsLayers.back()), m_VkDevice, VK_OBJECT_TYPE_IMAGE_VIEW, m_VkImageViewsLayers.back());
	}
}
void shade::VulkanImage2D::Invalidate(const render::Image::Specification& specification, const void* source)
{
	// For swapchain creating 

	m_Specification		= specification;
	// Make sure that mips count is in range !

	m_ImageLayout		= VK_IMAGE_LAYOUT_UNDEFINED;
	m_ImageFormat		= VK_FORMAT_MAX_ENUM;

	m_IsDepth			= VKUtils::IsDepthFormat(m_Specification.Format);
	m_IsDepthStencil	= VKUtils::IsDepthStencilFormat(m_Specification.Format);
	m_ImageFormat		= VKUtils::ToVulkanImageFormat(m_Specification.Format);
	// Determine the image aspects to be used, whether it is for color or depth

	m_AspectFlags = (m_IsDepth) ? VK_IMAGE_ASPECT_DEPTH_BIT : (m_IsDepthStencil) ? VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;

	// In case where image was created in swapchain
	m_RemouteDestorying = true;
	m_VkImage = reinterpret_cast<VkImage>(const_cast<void*>(source));

	if (m_Specification.Usage == render::Image::Usage::Attachment)
	{
		if (m_IsDepth || m_IsDepthStencil)
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		else
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
	}
	else if (m_Specification.Usage == render::Image::Usage::Texture)
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	else if (m_Specification.Usage == render::Image::Usage::Storage)
	{
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	// Set a debug object name for the VKUtils object instance being used 
	// This is done to help debugging if the program fails 
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Image", (std::uint32_t)&m_VkImage), m_VkDevice, VK_OBJECT_TYPE_IMAGE, m_VkImage);

	// Create a struct containing information needed to create an image view   
	VkImageViewCreateInfo imageViewCreateInfo
	{
	   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // The current structure type
	   .pNext = VK_NULL_HANDLE, // A pointer to the next structure if there's any other info to be filled
	   .flags = 0, // Optional flags
	   .image = m_VkImage, // The VkImage to create an image view for 
	   .viewType = (m_Specification.Layers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, // If the image has layers, create a 2D ImageArray, else, create a 2D image view 
	   .format = m_ImageFormat, // The format of the image view to be created 
	   .components = // The component map of the view
		  {
			.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A
		  },
	   .subresourceRange =  // The range of the subresource to be accessed
		  {
			.aspectMask = m_AspectFlags, // The aspect of the subresource to access, which could be COLOR, or DEPTH
			.baseMipLevel = 0,  // Starting point of the level of the mip to access
			.levelCount = m_Specification.MipLevels, // Number of levels in the mip chain
			.baseArrayLayer = 0, // Starting point of the layer to be accessed
			.layerCount = m_Specification.Layers // Number of layers to be accessed 
		  }
	};

	// Create the image view based on the info passed
	VK_CHECK_RESULT(vkCreateImageView(m_VkDevice, &imageViewCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageView), "Failed to create image view!");
	// Set a debug object name for the VKUtils object instance being used 
	// This is done to help debugging if the program fails
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge view", (std::uint32_t)&m_VkImageView), m_VkDevice, VK_OBJECT_TYPE_IMAGE_VIEW, m_VkImageView);

	// Create view per mip level;
	for (std::uint32_t i = 0; i < m_Specification.MipLevels; i++)
	{
		// Create a struct containing information needed to create an image view   
		VkImageViewCreateInfo imageViewCreateInfo
		{
		   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // The current structure type
		   .pNext = VK_NULL_HANDLE, // A pointer to the next structure if there's any other info to be filled
		   .flags = 0, // Optional flags
		   .image = m_VkImage, // The VkImage to create an image view for 
		   .viewType = (m_Specification.Layers > 1) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, // If the image has layers, create a 2D ImageArray, else, create a 2D image view 
		   .format = m_ImageFormat, // The format of the image view to be created 
		   .components = // The component map of the view
			  {
				.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A
			  },
		   .subresourceRange =  // The range of the subresource to be accessed
			  {
				.aspectMask = m_AspectFlags, // The aspect of the subresource to access, which could be COLOR, or DEPTH
				.baseMipLevel = i,  // Starting point of the level of the mip to access
				.levelCount = 1, // Number of levels in the mip chain
				.baseArrayLayer = 0, // Starting point of the layer to be accessed
				.layerCount = m_Specification.Layers // Number of layers to be accessed 
			  }
		};

		// Create the image view based on the info passed
		VK_CHECK_RESULT(vkCreateImageView(m_VkDevice, &imageViewCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageViewsMips.emplace_back()), "Failed to create image view!");
		// Set a debug object name for the VKUtils object instance being used 
		// This is done to help debugging if the program fails
		VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge view", (std::uint32_t)&m_VkImageViewsMips.back()), m_VkDevice, VK_OBJECT_TYPE_IMAGE_VIEW, m_VkImageViewsMips.back());
	}
	// Create view per layer;
	for (std::uint32_t i = 0; i < m_Specification.Layers; i++)
	{
		// Create a struct containing information needed to create an image view   
		VkImageViewCreateInfo imageViewCreateInfo
		{
		   .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // The current structure type
		   .pNext = VK_NULL_HANDLE, // A pointer to the next structure if there's any other info to be filled
		   .flags = 0, // Optional flags
		   .image = m_VkImage, // The VkImage to create an image view for 
		   .viewType = VK_IMAGE_VIEW_TYPE_2D, // If the image has layers, create a 2D ImageArray, else, create a 2D image view 
		   .format = m_ImageFormat, // The format of the image view to be created 
		   .components = // The component map of the view
			  {
				.r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A
			  },
		   .subresourceRange =  // The range of the subresource to be accessed
			  {
				.aspectMask = m_AspectFlags,	// The aspect of the subresource to access, which could be COLOR, or DEPTH
				.baseMipLevel = 0,				// Starting point of the level of the mip to access
				.levelCount = 1,				// Number of levels in the mip chain
				.baseArrayLayer = i,			// Starting point of the layer to be accessed
				.layerCount = 1					// Number of layers to be accessed 
			  }
		};

		// Create the image view based on the info passed
		VK_CHECK_RESULT(vkCreateImageView(m_VkDevice, &imageViewCreateInfo, m_VkInstance.AllocationCallbaks, &m_VkImageViewsLayers.emplace_back()), "Failed to create image view!");
		// Set a debug object name for the VKUtils object instance being used 
		// This is done to help debugging if the program fails
		VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge view", (std::uint32_t)&m_VkImageViewsLayers.back()), m_VkDevice, VK_OBJECT_TYPE_IMAGE_VIEW, m_VkImageViewsLayers.back());
	}
}

std::tuple<VkBuffer, VkDeviceMemory> shade::VulkanImage2D::CreateBuffer(VkDevice device, const VkAllocationCallbacks* pAllocator, const VkBufferCreateInfo* pCreateInfo, const VkMemoryPropertyFlags& properties)
{
	// creates a buffer
	VkBuffer buffer;
	VK_CHECK_RESULT(vkCreateBuffer(device, pCreateInfo, pAllocator, &buffer), "Failed to create buffer!");

	// sets a debug name for the newly created buffer
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "ImageBuffer", (std::uint32_t)buffer), device, VK_OBJECT_TYPE_BUFFER, buffer);

	// retrieves memory requirements for the buffer
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	// allocates memory for the buffer
	VkMemoryAllocateInfo memoryAllocateInfo
	{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		VK_NULL_HANDLE,
		memRequirements.size,
		VKUtils::FindMemoryType(VulkanContext::GetPhysicalDevice()->GetDevice(), memRequirements.memoryTypeBits, properties)
	};
	VkDeviceMemory bufferMemory;
	VK_CHECK_RESULT(vkAllocateMemory(device, &memoryAllocateInfo, pAllocator, &bufferMemory), "Failed to allocate memory!");

	// sets a debug name for the buffer memory
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "ImageBuffer memory", (std::uint32_t)buffer), device, VK_OBJECT_TYPE_DEVICE_MEMORY, bufferMemory);

	// binds the buffer with the allocated memory
	vkBindBufferMemory(device, buffer, bufferMemory, 0);

	// returns the newly created buffer and its assigned memory
	return { buffer, bufferMemory };
}

shade::VulkanImage2D::~VulkanImage2D()
{
	if (!m_RemouteDestorying)
	{
		// Free memory
		if (m_VkImageMemory != VK_NULL_HANDLE)
			vkFreeMemory(m_VkDevice, m_VkImageMemory, m_VkInstance.AllocationCallbaks);
		// Remove image
		if (m_VkImage != VK_NULL_HANDLE)
			vkDestroyImage(m_VkDevice, m_VkImage, m_VkInstance.AllocationCallbaks);
	}
	for (auto& view : m_VkImageViewsMips)
		if (view != VK_NULL_HANDLE)
			vkDestroyImageView(m_VkDevice, view, m_VkInstance.AllocationCallbaks);
	for (auto& view : m_VkImageViewsLayers)
		if (view != VK_NULL_HANDLE)
			vkDestroyImageView(m_VkDevice, view, m_VkInstance.AllocationCallbaks);

	if (m_VkImageView != VK_NULL_HANDLE)
		vkDestroyImageView(m_VkDevice, m_VkImageView, m_VkInstance.AllocationCallbaks);
}

void shade::VulkanImage2D::Resize(std::uint32_t width, std::uint32_t height, std::uint32_t mipCount)
{
	assert(m_Specification.Usage != render::Image::Usage::Attachment || !m_RemouteDestorying, "Image Usage != Attachment and cannot be implicitly resized.");

	// Free memory
	if (m_VkImageMemory != VK_NULL_HANDLE)
		vkFreeMemory(m_VkDevice, m_VkImageMemory, m_VkInstance.AllocationCallbaks);
	
	// Remove image
	if(m_VkImage != VK_NULL_HANDLE)
		vkDestroyImage(m_VkDevice, m_VkImage, m_VkInstance.AllocationCallbaks);

	// Remove view
	if (m_VkImageView != VK_NULL_HANDLE)
		vkDestroyImageView(m_VkDevice, m_VkImageView, m_VkInstance.AllocationCallbaks);

	for (auto& view : m_VkImageViewsMips)
		if (view != VK_NULL_HANDLE)
			vkDestroyImageView(m_VkDevice, view, m_VkInstance.AllocationCallbaks);
	for (auto& view : m_VkImageViewsLayers)
		if (view != VK_NULL_HANDLE)
			vkDestroyImageView(m_VkDevice, view, m_VkInstance.AllocationCallbaks);

	m_VkImageViewsMips.clear();
	m_VkImageViewsLayers.clear();

	m_Specification.Width = width; m_Specification.Height = height;
	m_Specification.MipLevels = (mipCount == UINT32_MAX) ? m_Specification.MipLevels : mipCount;

	Invalidate(m_Specification);
}

void shade::VulkanImage2D::LayoutTransition(SharedPointer<RenderCommandBuffer>& commandBuffer, 
	VkImageLayout newLayout, 
	VkAccessFlags srcAccessMask,
	VkAccessFlags dstAccessMask,
	VkPipelineStageFlags srcStage,
	VkPipelineStageFlags dstStage, 
	VkImageAspectFlags aspectMask, 
	std::uint32_t mipBaselevel, 
	std::uint32_t mipLevels, 
	std::uint32_t layerBseLevel,
	std::uint32_t layerCount, 
	std::uint32_t frameIndex)
{
	VkImageMemoryBarrier barrier
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = VK_NULL_HANDLE,
		.srcAccessMask = srcAccessMask,
		.dstAccessMask = dstAccessMask,
		.oldLayout = m_ImageLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = m_VkImage,
		.subresourceRange =
		{
				aspectMask, // aspectMask
				mipBaselevel, // baseMipLevel
				mipLevels, // levelCount
				layerBseLevel, // baseArrayLayer
				layerCount // layerCount
		}
	};

	commandBuffer->Begin(frameIndex);
	vkCmdPipelineBarrier(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	commandBuffer->End(frameIndex);
	commandBuffer->Submit(frameIndex);

	m_ImageLayout = newLayout;
}

VkImageLayout shade::VulkanImage2D::GetImageLayout() const
{
	return m_ImageLayout;
}

VkImageView shade::VulkanImage2D::GetImageViewPerMipLevel(std::uint32_t mip) const
{
	return m_VkImageViewsMips[mip];
}

VkImageView shade::VulkanImage2D::GetImageViewPerLayer(std::uint32_t layer) const
{
	return m_VkImageViewsLayers[layer];
}

VkImageView shade::VulkanImage2D::GetImageView() const
{
	return m_VkImageView;
}

VkImage shade::VulkanImage2D::GetImage() const
{
	return m_VkImage;
}

VkImageAspectFlags shade::VulkanImage2D::GetAspectMask() const
{
	return m_AspectFlags;
}

