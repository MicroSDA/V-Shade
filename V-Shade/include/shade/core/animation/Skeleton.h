#pragma once
#include <shade/core/math/Math.h>
#include <shade/core/memory/Memory.h>

namespace shade
{
	class SHADE_API Skeleton
	{
	public:

		// TIP: maybe we need to refactor this and pack all in one vector, mby exclude parent ids, mby.
		struct BonesData
		{
			std::vector<std::string>	Names;
			std::vector<std::uint32_t>	ParentIDs;
			// a.k.a LocalTransform
			std::vector<glm::vec3>		Translations;
			std::vector<glm::quat>		Rotations;
			std::vector<glm::vec3>		Scales;
		};

		struct BoneData
		{
			const std::uint32_t&	ParentID;
			const glm::vec3&		Translation;
			const glm::quat&		Rotation;
			const glm::vec3&		Scale;
		};

		struct BoneNode
		{
			std::string Name;
			glm::vec3	Translation;
			glm::quat   Rotation;
			glm::vec3	Scale;
			std::vector<SharedPointer<BoneNode>> Children;
		};

	public:
		static constexpr std::uint32_t NULL_BONE_ID = ~0; // std::numeric_limits<std::uint32_t>::max()
	public:
		Skeleton() = default;
		~Skeleton() = default;
	public:

		std::uint32_t AddBone(const std::string& name, std::uint32_t parentID, const glm::mat4& transform);

		shade::SharedPointer<shade::Skeleton::BoneNode>& AddBone(const std::string& name, const glm::mat4& transform);

		std::uint32_t GetBoneID(const std::string& name) const;
		const std::string& GetBoneName(std::size_t id) const;

		const Skeleton::BonesData& GetSkeletonBones() const;
		Skeleton::BonesData& GetSkeletonBones();
		std::size_t GetBonesCount() const;

		BoneData GetBoneData(const std::string& name) const;
		BoneData GetBoneData(std::size_t id) const;

	public:
		// Where string (Bone name) - > size_t bone index
		BonesData m_BonesData;
		std::unordered_map<std::string, SharedPointer<BoneNode>> m_BoneNodes;
		SharedPointer<BoneNode> m_RootBone;
	};
}