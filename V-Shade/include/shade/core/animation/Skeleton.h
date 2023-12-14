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
		static constexpr std::uint32_t NULL_BONE_ID = ~0;
	public:
		virtual ~Skeleton() = default;
		static AssetMeta::Type GetAssetStaticType();
		virtual AssetMeta::Type GetAssetType() const override;

		Skeleton() = default;
		static SharedPointer<Skeleton> CreateEXP();
	public:
		shade::SharedPointer<shade::Skeleton::BoneNode>& AddBone(const std::string& name, const glm::mat4& transform, const glm::mat4& inverseBindPose);
		void AddNode(const shade::SharedPointer<shade::Skeleton::BoneNode>& node);

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

	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const SharedPointer<Skeleton::BoneNode>& node, std::size_t);

	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, SharedPointer<Skeleton::BoneNode>& node, std::size_t);

	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const std::vector<SharedPointer<Skeleton::BoneNode>>& children, std::size_t)
	{
		std::uint32_t count = children.size();
		if (count == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", count));

		std::uint32_t totalSize = Serialize<std::uint32_t>(stream, count);

		for (const auto& node : children)
			totalSize += Serializer::Serialize<SharedPointer<Skeleton::BoneNode>>(stream, node);
		return totalSize;
	}
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, std::vector<SharedPointer<Skeleton::BoneNode>>& children, std::size_t)
	{
		std::uint32_t count = 0;
		// Read size first.
		std::size_t totalSize = Deserialize<std::uint32_t>(stream, count);
		if (count == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", count));

		children.resize(count);

		for (auto& child : children)
		{
			child = SharedPointer<Skeleton::BoneNode>::Create();
			totalSize += Serializer::Deserialize<SharedPointer<Skeleton::BoneNode>>(stream, child);
		}
		
		return totalSize;
	}
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const SharedPointer<Skeleton::BoneNode>& node, std::size_t)
	{
		std::size_t size = Serialize<std::uint32_t>(stream, node->ID);
		size += Serialize<std::string>(stream, node->Name);
		size += Serialize<glm::vec3>(stream, node->Translation);
		size += Serialize<glm::quat>(stream, node->Rotation);
		size += Serialize<glm::vec3>(stream, node->Scale);
		size += Serialize<glm::mat4>(stream, node->InverseBindPose);
		size += Serialize<std::vector<SharedPointer<Skeleton::BoneNode>>>(stream, node->Children);
		return size;
	}
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, SharedPointer<Skeleton::BoneNode>& node, std::size_t)
	{
		std::size_t size = Serializer::Deserialize<std::uint32_t>(stream, node->ID);
		size += Serializer::Deserialize<std::string>(stream, node->Name);
		size += Serializer::Deserialize<glm::vec3>(stream, node->Translation);
		size += Serializer::Deserialize<glm::quat>(stream, node->Rotation);
		size += Serializer::Deserialize<glm::vec3>(stream, node->Scale);
		size += Serializer::Deserialize<glm::mat4>(stream, node->InverseBindPose);
		size += Serializer::Deserialize<std::vector<SharedPointer<Skeleton::BoneNode>>>(stream, node->Children);
		return size;
	}
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Skeleton::BoneNodes& nodes, std::size_t)
	{
		std::uint32_t count = static_cast<std::uint32_t>(nodes.size());
		if (count == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", count));

		std::size_t totalSize = Serialize<std::uint32_t>(stream, count);

		for (auto& [str, value] : nodes)
		{
			totalSize += Serialize<std::string>(stream, str);
			totalSize += Serialize<SharedPointer<Skeleton::BoneNode>>(stream, value);
		}

		return totalSize;
	}
	//template<>
	//inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Skeleton::BoneNodes& nodes, std::size_t)
	//{
	//	std::uint32_t size = 0;
	//	// Read size first.
	//	std::size_t totalSize = Deserialize<std::uint32_t>(stream, size);
	//	if (size == UINT32_MAX)
	//		throw std::out_of_range(std::format("Incorrect array size = {}", size));

	//	for (std::uint32_t i = 0; i < size; i++)
	//	{
	//		std::string key; SharedPointer<Skeleton::BoneNode> value = SharedPointer<Skeleton::BoneNode>::Create();

	//		totalSize += Deserialize<std::string>(stream, key);
	//		totalSize += Deserialize<SharedPointer<Skeleton::BoneNode>>(stream, value);
	//		
	//		nodes.insert({ key, value });
	//	}
	//	return totalSize;
	//}
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