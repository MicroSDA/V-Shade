#pragma once
#include <shade/core/math/Math.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/asset/Asset.h>

namespace shade
{
	/**
	 * @brief The Skeleton class represents a basic skeleton structure.
	 *
	 * This class encapsulates the essential attributes and functionality of a skeleton,
	 * typically used in computer graphics, animation, or game development. A skeleton
	 * is composed of bones, and each bone may have a position, orientation 
	 */
	class SHADE_API Skeleton : public BaseAsset, public Asset<Skeleton>
	{
	public:
		struct BoneNode
		{
			// ID of the bone node
			std::uint32_t ID = Skeleton::BONE_NULL_ID;
			// Name of the bone node
			std::string Name;
			// Translation of the bone node
			glm::vec3 Translation = glm::vec3(1.f);
			// Rotation of the bone node
			glm::quat Rotation = glm::vec3(1.f);
			// Scale of the bone node
			glm::vec3 Scale = glm::vec3(1.f);
			// Inverse bind pose matrix of the bone node
			glm::mat4 InverseBindPose = glm::mat4(1.f);
			// Children bone nodes
			std::vector<SharedPointer<BoneNode>> Children;
		};
		struct BoneArmature
		{
			// Initialize a transformation matrix with identity matrix
			glm::mat4 Transform = glm::mat4(1.f);
		};

		using BoneNodes = std::unordered_map<std::string, SharedPointer<BoneNode>>;
	public:
		// The value of BONE_BONE_ID is the bitwise complement of 0.
		static constexpr std::uint32_t BONE_NULL_ID = ~0;
	public:
		// Destructor
		virtual ~Skeleton() = default;
		// Static method to get the static type of Skeleton asset
		static AssetMeta::Type GetAssetStaticType();
		// Virtual method to get the type of the Skeleton asset
		virtual AssetMeta::Type GetAssetType() const override;
		// Default constructor
		Skeleton() = default;
		// Static method to create a shared pointer to a Skeleton object
		static SharedPointer<Skeleton> CreateEXP();
	public:
		// Add a Bone to the Skeleton
		// Parameters:
		// - name: the name of the bone
		// - transform: the transformation matrix of the bone
		// - inverseBindPose: the inverse bind pose matrix of the bone
		// Return:
		// - a reference to the added bone
		shade::SharedPointer<shade::Skeleton::BoneNode>& AddBone(const std::string& name, const glm::mat4& transform, const glm::mat4& inverseBindPose);
		// Add a BoneNode to the Skeleton
		// Parameters:
		// - node: a shared pointer to the BoneNode to add
		void AddNode(const shade::SharedPointer<shade::Skeleton::BoneNode>& node);
		// Add an Armature to the Skeleton
		// Parameters:
		// - transform: the transformation matrix of the armature
		// Return:
		// - a reference to the added armature
		shade::SharedPointer<shade::Skeleton::BoneArmature>& AddArmature(const glm::mat4& transform);
		// Get a Bone from the Skeleton by name
		// Parameters:
		// - name: the name of the bone to retrieve
		// Return:
		// - a const reference to the BoneNode with the specified name
		const shade::SharedPointer<shade::Skeleton::BoneNode>& GetBone(const std::string& name) const;
		// Get a Bone from the Skeleton by id
		// Parameters:
		// - id: the id of the bone to retrieve
		// Return:
		// - a const reference to the BoneNode with the specified id
		const shade::SharedPointer<shade::Skeleton::BoneNode>& GetBone(std::size_t id) const;
		// Get the Armature from the Skeleton
		// Return:
		// - a const reference to the BoneArmature in the Skeleton
		const shade::SharedPointer<shade::Skeleton::BoneArmature>& GetArmature() const;
		// Get the root BoneNode of the Skeleton
		// Return:
		// - a const reference to the root BoneNode of the Skeleton
		const shade::SharedPointer<shade::Skeleton::BoneNode>& GetRootNode() const;
		// Get a map of all the Bones in the Skeleton
		// Return:
		// - a const reference to the map of bone nodes in the Skeleton
		const BoneNodes& GetBones() const;
	private:
		// Create a skeleton object with the given asset data, lifetime, and instantiation behaviour
		Skeleton(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		// Create a new skeleton object with the given asset data, lifetime, and instantiation behaviour
		static Skeleton* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		// Serialize the skeleton object and write the serialized data to the given output stream
		std::size_t Serialize(std::ostream& stream) const;
		// Deserialize the skeleton object from the given input stream and return the number of bytes read
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

	// Serialize function for vector of shared pointers to Skeleton::BoneNode
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
	// Deserialize function for vector of shared pointers to Skeleton::BoneNode
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

	// Serialize function for shared pointer to Skeleton::BoneNode
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

	// Deserialize function for shared pointer to Skeleton::BoneNode
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