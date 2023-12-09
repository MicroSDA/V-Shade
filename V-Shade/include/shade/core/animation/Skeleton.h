#pragma once
#include <shade/core/math/Math.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/asset/Asset.h>

namespace shade
{
	class SHADE_API Skeleton : public BaseAsset, public Asset<Skeleton>
	{
	public:
		struct BoneNode
		{
			std::uint32_t	ID;
			std::string		Name;
			glm::vec3		Translation		= glm::vec3(1.f);
			glm::quat		Rotation		= glm::vec3(1.f);
			glm::vec3		Scale			= glm::vec3(1.f);
			glm::mat4       InverseBindPose = glm::mat4(1.f);
			std::vector<SharedPointer<BoneNode>> Children;
		};
		struct BoneArmature
		{
			glm::mat4		Transform = glm::mat4(1.f);
		};
		using BoneNodes = std::unordered_map<std::string, SharedPointer<BoneNode>>;

	public:
		static constexpr std::uint32_t NULL_BONE_ID = ~0; // std::numeric_limits<std::uint32_t>::max()
	public:
		virtual ~Skeleton() = default;
		static AssetMeta::Type GetAssetStaticType();
		virtual AssetMeta::Type GetAssetType() const override;

		Skeleton() = default;
		static SharedPointer<Skeleton> CreateEXP();
	public:
		shade::SharedPointer<shade::Skeleton::BoneNode>& AddBone(const std::string& name, const glm::mat4& transform, const glm::mat4& inverseBindPose);
		shade::SharedPointer<shade::Skeleton::BoneArmature>& AddArmature(const glm::mat4& transform);

		const shade::SharedPointer<shade::Skeleton::BoneNode>& GetBone(const std::string& name) const;
		const shade::SharedPointer<shade::Skeleton::BoneNode>& GetBone(std::size_t id) const;

		const shade::SharedPointer<shade::Skeleton::BoneArmature>& GetArmature() const;
		const shade::SharedPointer<shade::Skeleton::BoneNode>& GetRootNode() const;

		const BoneNodes& GetBones() const;

	private:
		Skeleton(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		static Skeleton* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		std::size_t Serialize(std::ostream& stream) const;
		std::size_t Deserialize(std::istream& stream);
	private:
		friend class Serializer;
		friend class Asset<Skeleton>;
	private:
		BoneNodes m_BoneNodes;
		SharedPointer<BoneNode> m_RootNode;
		SharedPointer<BoneArmature> m_Armature;
	};

	/* Serialize Skeleton.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Skeleton& skeleton, std::size_t)
	{
		return skeleton.Serialize(stream);
	}
	/* Deserialize Skeleton.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Skeleton& skeleton, std::size_t)
	{
		return skeleton.Deserialize(stream);
	}
	/* Serialize Asset<Skeleton>.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Asset<Skeleton>& skeleton, std::size_t)
	{
		return skeleton->Serialize(stream);
	}
	/* Deserialize Asset<Skeleton>.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Asset<Skeleton>& skeleton, std::size_t)
	{
		return skeleton->Deserialize(stream);
	}
}