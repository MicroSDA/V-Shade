#pragma once
#include <shade/core/image/Image.h>
#include <shade/core/asset/Asset.h>

namespace shade
{
	class SHADE_API Texture 
	{
	public:
		virtual ~Texture() = default;
		virtual std::uint32_t GetWidth() const = 0;
		virtual std::uint32_t GetHeight() const = 0;
		virtual void Resize(std::uint32_t width, std::uint32_t height, std::uint32_t mipCount = UINT32_MAX) = 0;
	private:
	};


	class SHADE_API Texture2D : public Texture, ASSET_INHERITANCE(Texture2D)
	{
		ASSET_STATIC_TYPE_HELPER(Texture)
		SHADE_CAST_HELPER(Texture2D)

	public:
		virtual ~Texture2D() = default;
		Texture2D(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		// To create EXP
		Texture2D(const SharedPointer<render::Image2D>& image);
		Texture2D(const render::Image::Specification& specification);
	public:
		SharedPointer<shade::render::Image2D>& GetImage();
		// To create EXP.
		static SharedPointer<Texture2D> CreateEXP(const SharedPointer<render::Image2D>& image);
		static SharedPointer<Texture2D> CreateEXP(const render::Image::Specification& specification);
	private:
		static Texture2D* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
	protected:
		SharedPointer<render::Image2D> m_Image;
	private:
		friend class Asset<Texture2D>;
		friend class SharedPointer<Texture2D>;
	};
}
