#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/serializing/Serializer.h>

namespace shade
{
	namespace render
	{
		class SHADE_API Image
		{
		public:
			enum class Format
			{
				Undefined = 0,
				// Int
				RED8UN,
				RED8UI,
				RED16UI,
				RED32UI,
				DEPTH32FSTENCIL8UINT,
				// Float
				RED32F,
				RG8,
				RG16F,
				RG32F,
				RGB,
				RGBA,
				BGRA,
				RGBA16F,
				RGBA32F,
				B10R11G11UF,
				SRGB,
				DEPTH32F,
				DEPTH24STENCIL8,
				Depth = DEPTH24STENCIL8,
			};
			enum class Usage
			{
				None = 0,
				Texture,
				Attachment,
				Storage
			};
			struct Specification
			{
				Image::Format Format = Image::Format::RGBA;
				Image::Usage Usage = Image::Usage::None;

				std::uint32_t MipLevels = 1;
				std::uint32_t Layers = 1;
				std::uint32_t Width = 1;
				std::uint32_t Height = 1;
				// TIP: TEST !!
				bool IsCubeMap = false;
			};
			struct ImageData
			{
				std::uint8_t* Data = nullptr;
				std::uint16_t  Width = 0;
				std::uint16_t  Height = 0;
				std::uint8_t   MipMapCount = 1;
				std::uint32_t  Size = 0;
				enum class DXTCompression : std::uint32_t
				{
					Undefined = 0,
					DXT1 = 827611204,
					DXT3 = 861165636,
					DXT5 = 894720068,
					DXT10 = 808540228,
					// Linear unsigned 
					BC5LU = 1429553986,
					// Linear unsigned
					BC5LS = 1395999554
				} Compression = DXTCompression::Undefined;

				void Delete()
				{
					if (Data != nullptr)
					{
						delete[] Data;
						Data = nullptr;
					}
				}
			};
			virtual ~Image();
			void GenerateDiffuseTexture();
			ImageData& GetImageData();
		private:
			struct Header
			{
				std::uint32_t Magic;
				std::uint32_t Size;
				std::uint32_t Flags;
				std::uint32_t Height;
				std::uint32_t Width;
				std::uint32_t PitchOrLinearSize;
				std::uint32_t Depth;
				std::uint32_t MipMapCount;
				std::uint32_t Reserved1[11];
				struct DdsPixelFormat
				{
					std::uint32_t dwSize;
					std::uint32_t dwFlags;
					std::uint32_t dwFourCC;
					std::uint32_t dwRGBBitCount;
					std::uint32_t dwRBitMask;
					std::uint32_t dwGBitMask;
					std::uint32_t dwBBitMask;
					std::uint32_t dwABitMask;
				} Dspf;
				std::uint32_t Caps;
				std::uint32_t Caps2;
				std::uint32_t Caps3;
				std::uint32_t Caps4;
				std::uint32_t Reserved2;
			};
		private:
			ImageData m_ImageData;
		private:
			std::size_t Serialize(std::ostream& stream) const;
			std::size_t Deserialize(std::istream& stream);
			static void ReadHeader(std::istream& stream, Header& header);
		private:
			friend class Serializer;
		};
		class SHADE_API Image2D
		{
		public:
			virtual ~Image2D() = default;
			static SharedPointer<Image2D> Create(Image& source);
			static SharedPointer<Image2D> Create(const Image::Specification& spec, const void* source);
			static SharedPointer<Image2D> Create(const Image::Specification& spec);
			virtual void Resize(std::uint32_t width, std::uint32_t height, std::uint32_t mipCount = UINT32_MAX) = 0;

			Image::Specification& GetSpecification();
			const Image::Specification& GetSpecification() const;

			template<typename T>
			T& As();
		protected:
			Image::Specification m_Specification;
			Image::ImageData m_ImageData;
		};
		template<typename T>
		inline T& Image2D::As()
		{
			static_assert(std::is_base_of<Image2D, T>::value, "");
			return static_cast<T&>(*this);
		}
	}

	// This function serializes an image on a given output stream
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const render::Image& image, std::size_t)
	{
		return image.Serialize(stream);
	}
	// This function deserializes an image from a given input stream
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, render::Image& image, std::size_t)
	{
		return image.Deserialize(stream);
	}
}
