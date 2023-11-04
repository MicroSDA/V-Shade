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

	class SHADE_API Texture2D : public Texture, public BaseAsset, public Asset<Texture2D>
	{
	public:
		// To create through AssetManager
		Texture2D(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		// To create directly.
		Texture2D(const SharedPointer<render::Image2D>& image);
		Texture2D(const render::Image::Specification& specification);

		// To create directly.
		static SharedPointer<Texture2D> CreateEXP(const SharedPointer<render::Image2D>& image);
		static SharedPointer<Texture2D> CreateEXP(const render::Image::Specification& specification);

		SharedPointer<render::Image2D>& GetImage();

		static AssetMeta::Type GetAssetStaticType();
		virtual AssetMeta::Type GetAssetType() const override;

		template<typename T>
		T& As();
	private:
		// To create through AssetManager
		static Texture2D* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
	protected:
		SharedPointer<render::Image2D> m_Image;

	private:
		friend class Asset<Texture2D>;
	};

	template<typename T>
	inline T& Texture2D::As()
	{
		static_assert(std::is_base_of<Texture2D, T>::value, "");
		return static_cast<T&>(*this);
	}
}
