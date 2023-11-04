#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/image/Image.h>
#include <shade/core/image/Texture.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp >

namespace shade
{
	class SHADE_API FrameBuffer
	{
	public:
		struct TextureSpecification
		{
			TextureSpecification(const render::Image::Format& format = render::Image::Format::Undefined): Format(format) {}
			render::Image::Format Format;
		};
		struct AttachmentsSpecification
		{
			AttachmentsSpecification() = default;
			AttachmentsSpecification(const std::initializer_list<render::Image::Specification>& attachments)
				: TextureAttachments(attachments) {}

			std::vector<render::Image::Specification> TextureAttachments;
		};
		struct Specification
		{
			std::uint32_t Width = 1, Height = 1;
			glm::vec4 ClearColor  = { 0.0f, 0.0f, 0.0f, 1.0f }; // TODO:
			float DepthClearValue = 0.0f;
			AttachmentsSpecification Attachments;
		};
	public:
		static SharedPointer<FrameBuffer> Create(const FrameBuffer::Specification& specification);
		static SharedPointer<FrameBuffer> Create(const FrameBuffer::Specification& specification, const std::vector<SharedPointer<render::Image2D>>& images);
		static std::vector<SharedPointer<FrameBuffer>> CreateFromSwapChain();
		virtual ~FrameBuffer() = default;

		virtual std::uint32_t GetWidth()  const = 0;
		virtual std::uint32_t GetHeight() const = 0;

		virtual void Resize(std::uint32_t width, uint32_t height) = 0;
		virtual void Resize(std::uint32_t width, uint32_t height, const std::vector<SharedPointer<render::Image2D>>& images) = 0;

		FrameBuffer::Specification& GetSpecification();
		const FrameBuffer::Specification& GetSpecification() const;

		virtual SharedPointer<Texture2D>& GetTextureAttachment(std::uint32_t index = 0)= 0;
		virtual SharedPointer<Texture2D>& GetDepthAttachment(std::uint32_t index = 0) = 0;

		template<typename T>
		T& As();
	protected:
		FrameBuffer::Specification m_Specification;
	};
	template<typename T>
	inline T& FrameBuffer::As()
	{
		static_assert(std::is_base_of<FrameBuffer, T>::value, "");
		return static_cast<T&>(*this);
	}
}