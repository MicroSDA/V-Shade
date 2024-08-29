#pragma once
#include <shade/core/render/drawable/Mesh.h>
#include <shade/core/animation/Animation.h>

namespace shade
{
	class SHADE_API	Model : ASSET_INHERITANCE(Model)
	{
		ASSET_DEFINITION_HELPER(Model)
		VECTOR_BASE_ITERATOR_HELPER(Asset<Mesh>, m_Meshes)

	public:
		virtual ~Model() = default;
	public:
		void AddMesh(const Asset<Mesh>& mesh);
		const std::vector<Asset<Mesh>>& GetMeshes() const;
		std::vector<Asset<Mesh>>& GetMeshes();
		void SetSkeleton(const Asset<Skeleton>& skeleton);
		const Asset<Skeleton>& GetSkeleton() const;
	private:
		std::vector<Asset<Mesh>> m_Meshes;
		Asset<Skeleton> m_Skeleton; // Remove from here and add to Animation Graph
	private:
		Model(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		friend class Serializer;
	};
}