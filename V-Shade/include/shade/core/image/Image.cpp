#include "shade_pch.h"
#include "Image.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/VulkanImage2D.h>

constexpr const char UNDEFINED_DIFFUSE_IMAGE_DATA_1_1[]{ 
0x44, 0x44, 0x53, 0x20, 0x7C, 0x00, 0x00, 0x00, 0x07, 0x10, 0x0A, 0x00, 0x01, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
0x04, 0x00, 0x00, 0x00, 0x44, 0x58, 0x54, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x10, 0x84, 0xEF, 0x7B, 0xAA, 0xBA, 0xAA, 0xAE };

shade::SharedPointer<shade::render::Image2D> shade::render::Image2D::Create(Image& source)
{
	switch (RenderAPI::GetCurrentAPI())
	{
	case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	case RenderAPI::API::Vulkan: return SharedPointer<VulkanImage2D>::Create(VulkanContext::GetLogicalDevice()->GetDevice(), VulkanContext::GetInstance(), source);
	default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

shade::SharedPointer<shade::render::Image2D> shade::render::Image2D::Create(const Image::Specification& spec, const void* source)
{
	switch (RenderAPI::GetCurrentAPI())
	{
	case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	case RenderAPI::API::Vulkan: return SharedPointer<VulkanImage2D>::Create(VulkanContext::GetLogicalDevice()->GetDevice(), VulkanContext::GetInstance(), spec, source);
	default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

shade::SharedPointer<shade::render::Image2D> shade::render::Image2D::Create(const Image::Specification& spec)
{
	switch (RenderAPI::GetCurrentAPI())
	{
	case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	case RenderAPI::API::Vulkan: return SharedPointer<VulkanImage2D>::Create(VulkanContext::GetLogicalDevice()->GetDevice(), VulkanContext::GetInstance(), spec);
	default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

shade::render::Image::Specification& shade::render::Image2D::GetSpecification()
{
	return m_Specification;
}

const shade::render::Image::Specification& shade::render::Image2D::GetSpecification() const
{
	return m_Specification;
}

shade::render::Image::~Image()
{
	m_ImageData.Delete();
}

void shade::render::Image::GenerateDiffuseTexture()
{
	std::stringstream stream;
	stream.write(UNDEFINED_DIFFUSE_IMAGE_DATA_1_1, ARRAYSIZE(UNDEFINED_DIFFUSE_IMAGE_DATA_1_1));
	Header		header;
	ReadHeader(stream, header);
	if (memcmp(&header.Magic, "DDS ", 4) == 0) // if magic is DDS
	{
		m_ImageData.Width = header.Width;
		m_ImageData.Height = header.Height;

		/*If texture contains compressed RGB data; dwFourCC contains valid data.*/
		if (header.Dspf.dwFlags == 0x4) // DDPF_FOURCC = 0x4 
			m_ImageData.Compression = static_cast<ImageData::DXTCompression>(header.Dspf.dwFourCC);

		m_ImageData.MipMapCount = header.MipMapCount;

		auto begin = stream.tellg();
		stream.seekg(0, std::ios::end);
		auto end = stream.tellg();
		stream.seekg(begin);
		std::uint32_t imageSize = static_cast<std::uint32_t>(end - begin);

		m_ImageData.Size = imageSize;

		m_ImageData.Data = new std::uint8_t[m_ImageData.Size];
		Serializer::Deserialize(stream, *m_ImageData.Data, m_ImageData.Size);
	}
	else
	{
		SHADE_CORE_WARNING("Wrong image header !: {0}", header.Magic);
	}
}

shade::render::Image::ImageData& shade::render::Image::GetImageData()
{
	return m_ImageData;
}

std::size_t shade::render::Image::Serialize(std::ostream& stream) const
{
	return std::size_t();
}

void shade::render::Image::ReadHeader(std::istream& stream, Header& header)
{
	Serializer::Deserialize(stream, header);
}

std::size_t shade::render::Image::Deserialize(std::istream& stream)
{
	Header		header;
	ImageData	data;
	ReadHeader(stream, header);
	if (memcmp(&header.Magic, "DDS ", 4) == 0) // if magic is DDS
	{

		m_ImageData.Width = header.Width;
		m_ImageData.Height = header.Height;

		/*If texture contains compressed RGB data; dwFourCC contains valid data.*/
		if (header.Dspf.dwFlags == 0x4) // DDPF_FOURCC = 0x4 
			m_ImageData.Compression = static_cast<ImageData::DXTCompression>(header.Dspf.dwFourCC);

		m_ImageData.MipMapCount = header.MipMapCount;

		// Image size
		auto begin = stream.tellg();
		stream.seekg(0, std::ios::end);
		auto end = stream.tellg();
		stream.seekg(begin);
		std::uint32_t imageSize = static_cast<std::uint32_t>(end - begin);

		// Create image buffer
		m_ImageData.Data = new std::uint8_t[imageSize];
		m_ImageData.Size = imageSize;
		// Read buffer
		Serializer::Deserialize(stream, *m_ImageData.Data, imageSize);
		return imageSize;
	}
	else
	{
		SHADE_CORE_WARNING("Wrong image header !: {0}", header.Magic);
	}

	return 0;
}
