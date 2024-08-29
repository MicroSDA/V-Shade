#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/asset/Asset.h>
#include <shade/core/render/drawable/Drawable.h>
#include <shade/core/serializing/Serializer.h>
#include <shade/utils/Logger.h>
#include <shade/core/render/drawable/Material.h>
#include <shade/core/animation/Skeleton.h>

namespace shade
{
	class SHADE_API Mesh : public Drawable, ASSET_INHERITANCE(Mesh)
	{
		ASSET_DEFINITION_HELPER(Mesh)

	public:
		virtual ~Mesh() = default;
	private:
		Mesh(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		std::size_t Serialize(std::ostream& stream) const;
		std::size_t Deserialize(std::istream& stream);
	private:
		friend class Serializer;
	};

	/* Serialize Mesh.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Mesh& mesh, std::size_t)
	{
		return mesh.Serialize(stream);
	}
	/* Deserialize Mesh.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Mesh& mesh, std::size_t)
	{
		return mesh.Deserialize(stream);
	}
	/* Serialize Asset<Mesh>.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Asset<Mesh>& mesh, std::size_t)
	{
		return mesh->Serialize(stream);
	}
	/* Deserialize Asset<Mesh>.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Asset<Mesh>& mesh, std::size_t)
	{
		return mesh->Deserialize(stream);
	}
}