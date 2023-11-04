#include "shade_pch.h"
#include "VulkanImGuiRender.h"

#include <ImGui/backends/imgui_impl_glfw.cpp>
#include <ImGui/backends/imgui_impl_vulkan.cpp>
#include <shade/core/application/Application.h>

shade::VulkanImGuiRender::VulkanImGuiRender()
{
	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = 1000,
		.poolSizeCount = static_cast<std::uint32_t>(std::size(poolSizes)),
		.pPoolSizes = poolSizes
	};

	VK_CHECK_RESULT(vkCreateDescriptorPool(VulkanContext::GetDevice()->GetLogicalDevice(), &descriptorPoolCreateInfo, nullptr, &m_DescriptorPool), "Failed to create descriptor pool");

	ImGui_ImplVulkan_InitInfo imguiImplVulkanInfo
	{
		.Instance = VulkanContext::GetInstance().Instance,
		.PhysicalDevice = VulkanContext::GetPhysicalDevice()->GetDevice(),
		.Device = VulkanContext::GetDevice()->GetLogicalDevice(),
		.Queue = VulkanContext::GetDevice()->GetGraphicsQueue(),
		.DescriptorPool = m_DescriptorPool,
		.MinImageCount = 2,
		.ImageCount = 3,
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.Allocator = VulkanContext::GetInstance().AllocationCallbaks,
	};

	m_ImGuiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(m_ImGuiContext);

	ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow()), true);
	auto& swapchain = Application::GetWindow()->GetSwapChain()->As<VulkanSwapChain>();
	ImGui_ImplVulkan_Init(&imguiImplVulkanInfo, swapchain.GetColorFormat(), VulkanContext::GetPhysicalDevice()->GetDepthForamt());

	// Create imgui fonts !
	auto commandBuffer = RenderCommandBuffer::Create(RenderCommandBuffer::Type::Primary, RenderCommandBuffer::Family::Graphic);
	commandBuffer->Begin();
	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer());
	commandBuffer->End();
	commandBuffer->Submit();
	ImGui_ImplVulkan_DestroyFontUploadObjects();
	// !Create imgui fonts
	m_CommandBuffer = RenderCommandBuffer::CreateFromSwapChain();
}

shade::VulkanImGuiRender::~VulkanImGuiRender()
{
	vkDestroyDescriptorPool(VulkanContext::GetDevice()->GetLogicalDevice(), m_DescriptorPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
	ImGui::DestroyContext(m_ImGuiContext); 
}

void shade::VulkanImGuiRender::BeginRender()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
}

void shade::VulkanImGuiRender::EndRender()
{
	auto frameIndex = Renderer::GetCurrentFrameIndex();
	auto& swapChain = Application::GetWindow()->GetSwapChain()->As<VulkanSwapChain>();
	m_CommandBuffer->Begin(frameIndex);
	//
	auto& vulkanFrameBuffer = swapChain.GetFrameBuffer()->As<VulkanFrameBuffer>();
	auto renderingInfo = vulkanFrameBuffer.GetRenderingInfo();
	auto vulkanCommandBuffer = m_CommandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex);

	vkCmdBeginRendering(vulkanCommandBuffer, &renderingInfo);

	//VkViewport viewport
	//{ 0.f, static_cast<float>(vulkanFrameBuffer.GetSpecification().Height), // X and Y position
	//  static_cast<float>(vulkanFrameBuffer.GetSpecification().Width), -static_cast<float>(vulkanFrameBuffer.GetSpecification().Height), // Size
	//  0.f, 1.f // Min and Max depth values
	//};
	//VkRect2D scissor{ {0, 0} /*offset*/, { vulkanFrameBuffer.GetSpecification().Width, vulkanFrameBuffer.GetSpecification().Height } /*extent*/ };

	//vkCmdSetViewport(vulkanCommandBuffer, 0, 1, &viewport);
	//vkCmdSetScissor(vulkanCommandBuffer, 0, 1, &scissor);

	VkClearRect clearRect
	{
		.rect = renderingInfo.renderArea,
		.baseArrayLayer = 0,
		.layerCount = renderingInfo.layerCount,
	};

	vkCmdClearAttachments(vulkanCommandBuffer, static_cast<std::uint32_t>(vulkanFrameBuffer.GetClearAttachments().size()), vulkanFrameBuffer.GetClearAttachments().data(), 1, &clearRect);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_CommandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), m_ImGuiPipeline);

	vkCmdEndRendering(m_CommandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex));

	m_CommandBuffer->End(frameIndex);
}

void shade::VulkanImGuiRender::DrawImage(SharedPointer<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor)
{
	// Сделать леют транизшин пер мип, сайчас можно тодлько для диапазона а не для конкретной мипы
	auto commandBuffer = RenderCommandBuffer::Create();
	auto& image = texture->GetImage()->As<VulkanImage2D>();
	// VK_IMAGE_LAYOUT_GENERAL because we are using it as storage in compute shader color correction!
	image.LayoutTransition(commandBuffer,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT);
	
	if (m_Images.find(texture) == m_Images.end())
	{
		m_Images[texture] = { ImGui_ImplVulkan_AddTexture(texture->As<VulkanTexture2D>().GetSampler(), image.GetImageView(), image.GetImageLayout()), image.GetImageView() };
	}
	else if (m_Images.at(texture).second != image.GetImageView())
	{
		m_Images[texture] = { ImGui_ImplVulkan_AddTexture(texture->As<VulkanTexture2D>().GetSampler(), image.GetImageView(), image.GetImageLayout()),image.GetImageView() };
	}
	ImGui::Image(m_Images[texture].first, size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), borderColor);
}


void shade::VulkanImGuiRender::DrawImage(SharedPointer<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor, std::uint32_t mip)
{
	// Сделать леют транизшин пер мип, сайчас можно тодлько для диапазона а не для конкретной мипы
	auto commandBuffer = RenderCommandBuffer::Create();
	auto& image = texture->GetImage()->As<VulkanImage2D>();
	// VK_IMAGE_LAYOUT_GENERAL because we are using it as storage in compute shader color correction!
	image.LayoutTransition(commandBuffer,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT, mip, 1);

	if (m_Images.find(texture) == m_Images.end())
	{
		m_Images[texture] = { ImGui_ImplVulkan_AddTexture(texture->As<VulkanTexture2D>().GetSampler(), image.GetImageViewPerMipLevel(mip), image.GetImageLayout()), image.GetImageViewPerMipLevel(mip) };
	}
	else if (m_Images.at(texture).second != image.GetImageViewPerMipLevel(mip))
	{
		m_Images[texture] = { ImGui_ImplVulkan_AddTexture(texture->As<VulkanTexture2D>().GetSampler(), image.GetImageViewPerMipLevel(mip), image.GetImageLayout()), image.GetImageViewPerMipLevel(mip) };
	}
	ImGui::Image(m_Images[texture].first, size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), borderColor);
}
