#pragma once
#include <shade/core/render/drawable/Mesh.h>
#include <shade/core/animation/Animation.h>

namespace shade
{
	class SHADE_API		Model : public BaseAsset, public Asset<Model>
	{
	public:
		virtual ~Model();
		static AssetMeta::Type GetAssetStaticType();
		virtual AssetMeta::Type GetAssetType() const override;

		Model() = default;
		static SharedPointer<Model> CreateEXP();
	public:
		std::vector<Asset<Mesh>>::iterator begin() noexcept { return m_Meshes.begin(); };
		std::vector<Asset<Mesh>>::iterator end() noexcept { return m_Meshes.end(); };
		std::vector<Asset<Mesh>>::const_iterator cbegin() const noexcept { return m_Meshes.begin(); };
		std::vector<Asset<Mesh>>::const_iterator cend() const noexcept { return m_Meshes.end(); };
	public:
		void AddMesh(const Asset<Mesh>& mesh);
		const std::vector<Asset<Mesh>>& GetMeshes() const;
		std::vector<Asset<Mesh>>& GetMeshes();
	private:
		std::vector<Asset<Mesh>> m_Meshes;
	private:
		static Model* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		Model(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);

		// TODO: mby rework it
	public:
		SharedPointer<Skeleton> m_Skeleton;

		friend class Asset<Model>;
		friend class Serializer;
	private:
		friend class SceneComponentSerializer;
		std::size_t SerializeAsComponent(std::ostream& stream) const;
		std::size_t DeserializeAsComponent(std::istream& stream);
	};
}

namespace shade
{
	template<>
	inline std::size_t shade::SceneComponentSerializer::Serialize(std::ostream& stream, const Model& model)
	{
		return shade::Serializer::Serialize(stream, model.GetAssetData()->GetId());
	}

	template<>
	inline std::size_t shade::SceneComponentSerializer::Serialize(std::ostream& stream, const Asset<Model>& model)
	{
		return shade::Serializer::Serialize(stream, model->GetAssetData()->GetId());
	}

	template<>
	inline std::size_t shade::SceneComponentSerializer::Serialize(std::ostream& stream, const SharedPointer<Model>& model)
	{
		return shade::Serializer::Serialize(stream, model->GetAssetData()->GetId());
	}

	template<>
	inline std::size_t shade::SceneComponentSerializer::Deserialize(std::istream& stream, Model& model, std::size_t)
	{
		return model.DeserializeAsComponent(stream);
	}

	template<>
	inline std::size_t shade::SceneComponentSerializer::Deserialize(std::istream& stream, Asset<Model>& model, std::size_t)
	{
		return model->DeserializeAsComponent(stream);
	}

	template<>
	inline std::size_t shade::SceneComponentSerializer::Deserialize(std::istream& stream, SharedPointer<Model>& model, std::size_t)
	{
		return model->DeserializeAsComponent(stream);
	}
}