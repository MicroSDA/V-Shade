#pragma once
#include <shade/core/math/Math.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/asset/Asset.h>
#include <shade/core/serializing/File.h>

namespace shade
{
	/**
	 * @brief The Skeleton class represents a basic skeleton structure.
	 *
	 * This class encapsulates the essential attributes and functionality of a skeleton,
	 * typically used in computer graphics, animation, or game development. A skeleton
	 * is composed of bones, and each bone may have a position, orientation 
	 */
	class SHADE_API Skeleton : ASSET_INHERITANCE(Skeleton)
	{
		ASSET_DEFINITION_HELPER(Skeleton)

	public:
		// The value of BONE_BONE_ID is the bitwise complement of 0.
		using BoneID = std::uint32_t;
		static constexpr BoneID BONE_NULL_ID = ~0;

		struct BoneNode
		{
			// ID of the bone node
			BoneID ID = Skeleton::BONE_NULL_ID;
			// Name of the bone node
			std::string Name;
			// Translation of the bone node
			glm::vec3 Translation = glm::vec3(1.f);
			// Rotation of the bone node
			glm::quat Rotation = glm::identity<glm::quat>();
			// Scale of the bone node
			glm::vec3 Scale = glm::vec3(1.f);
			// Inverse bind pose matrix of the bone node
			glm::mat4 InverseBindPose = glm::identity<glm::mat4>();
			// Children bone nodes
			std::vector<BoneNode*> Children;
		};
		struct BoneArmature
		{
			std::string Name;
			// Translation of the bone node
			glm::vec3 Translation = glm::vec3(1.f);
			// Rotation of the bone node
			glm::quat Rotation = glm::identity<glm::quat>();
			// Scale of the bone node
			glm::vec3 Scale = glm::vec3(1.f);
		};
		using BoneNodes = std::unordered_map<std::string, BoneNode>;
	public:
		
	public:
		// Destructor
		virtual ~Skeleton() = default;
	public:
		// Add a Bone to the Skeleton
		// Parameters:
		// - name: the name of the bone
		// - transform: the transformation matrix of the bone
		// - inverseBindPose: the inverse bind pose matrix of the bone
		// Return:
		// - a reference to the added bone
		shade::Skeleton::BoneNode* AddBone(const std::string& name, const glm::mat4& transform, const glm::mat4& inverseBindPose);
		// Add a BoneNode to the Skeleton
		// Parameters:
		// - node: a shared pointer to the BoneNode to add
		shade::Skeleton::BoneNode* AddNode(const shade::Skeleton::BoneNode& node);
		// Get a Bone from the Skeleton by name
		// Parameters:
		// - name: the name of the bone to retrieve
		// Return:
		// - a const reference to the BoneNode with the specified name
		const shade::Skeleton::BoneNode* GetBone(const std::string& name) const;
		// Get a Bone from the Skeleton by id
		// Parameters:
		// - id: the id of the bone to retrieve
		// Return:
		// - a const reference to the BoneNode with the specified id
		const shade::Skeleton::BoneNode* GetBone(std::size_t id) const;

		shade::Skeleton::BoneArmature* AddArmature(const std::string& name, const glm::mat4& transform);
		// Get the Armature from the Skeleton
		// Return:
		// - a const reference to the BoneArmature in the Skeleton
		const shade::Skeleton::BoneArmature* GetArmature() const;
		// Get the Armature from the Skeleton
		// Return:
		// - a reference to the BoneArmature in the Skeleton
		shade::Skeleton::BoneArmature* GetArmature();
		// Get the root BoneNode of the Skeleton
		// Return:
		// - a const reference to the root BoneNode of the Skeleton
		const shade::Skeleton::BoneNode* GetRootNode() const;
		// Get the root BoneNode of the Skeleton
		// Return:
		// - a reference to the root BoneNode of the Skeleton
		shade::Skeleton::BoneNode* GetRootNode();
		// Get a map of all the Bones in the Skeleton
		// Return:
		// - a const reference to the map of bone nodes in the Skeleton
		const BoneNodes& GetBones() const;
	private:
		// Create a skeleton object with the given asset data, lifetime, and instantiation behaviour
		Skeleton(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		// Serialize the skeleton object and write the serialized data to the given output stream
		void Serialize(std::ostream& stream) const;
		// Deserialize the skeleton object from the given input stream and return the number of bytes read
		void Deserialize(std::istream& stream);
	private:
		friend class serialize::Serializer;
	private:
		BoneNodes		m_BoneNodes;
		BoneNode*		m_RootNode = nullptr;
		BoneArmature	m_Armature;
	};

	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const Skeleton& skeleton)
	{
		return skeleton.Serialize(stream);
	}
	/* Deserialize Skeleton.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, Skeleton& skeleton)
	{
		return skeleton.Deserialize(stream);
	}
	/* Serialize Asset<Skeleton>.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const Asset<Skeleton>& skeleton)
	{
		return skeleton->Serialize(stream);
	}
	/* Deserialize Asset<Skeleton>.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, Asset<Skeleton>& skeleton)
	{
		return skeleton->Deserialize(stream);
	}

	namespace animation
	{
		struct BoneMask
		{
			BoneMask(const Asset<Skeleton>& skeleton)
			{
				if (skeleton)
				{
					for (auto& [name, bone] : skeleton->GetBones())
						Weights.emplace(bone.ID, std::pair<std::string, float>{ name, 1.0f });
				}
			}
			~BoneMask() = default;
			void Reset()
			{
				for (auto& [id, mask] : Weights)
					mask.second = 1.0f;
			}
			float GetWeight(std::size_t id) const
			{
				auto search = Weights.find(id);
				return ((search != Weights.end()) ? search->second.second : 1.f);
			}
			std::unordered_map<std::size_t, std::pair<std::string, float>> Weights;
		};
	}

}