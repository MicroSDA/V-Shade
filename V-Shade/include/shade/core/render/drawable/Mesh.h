#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/asset/Asset.h>
#include <shade/core/render/drawable/Drawable.h>
#include <shade/core/serializing/Serializer.h>
#include <shade/utils/Logger.h>
#include <shade/core/render/drawable/Material.h>
#include <shade/core/animation/Skeleton.h>
#include <shade/core/serializing/File.h>

namespace shade
{
	class SHADE_API Mesh : public Drawable, ASSET_INHERITANCE(Mesh)
	{
		ASSET_DEFINITION_HELPER(Mesh)

	public:
		virtual ~Mesh() = default;
	private:
		Mesh(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		void Serialize(std::ostream& stream) const;
		void Deserialize(std::istream& stream);
	private:
		friend class serialize::Serializer;
	};

	/* Serialize Mesh.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const Mesh& mesh)
	{
		mesh.Serialize(stream);
	}
	/* Deserialize Mesh.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, Mesh& mesh)
	{
		mesh.Deserialize(stream);
	}
	/* Serialize Asset<Mesh>.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const Asset<Mesh>& mesh)
	{
		mesh->Serialize(stream);
	}
	/* Deserialize Asset<Mesh>.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, Asset<Mesh>& mesh)
	{
		mesh->Deserialize(stream);
	}
}