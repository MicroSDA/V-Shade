#pragma once
#include <shade/core/math/Math.h>
#include <shade/core/memory/Memory.h>

namespace shade
{
	class SHADE_API Skeleton
	{
	public:

		struct BoneNode
		{
			std::uint32_t	ID;
			std::string		Name;
			glm::vec3		Translation;
			glm::quat		Rotation;
			glm::vec3		Scale;
			std::vector<SharedPointer<BoneNode>> Children;
		};
		struct BoneArmature
		{
			glm::mat4		Transform;
		};

	public:
		static constexpr std::uint32_t NULL_BONE_ID = ~0; // std::numeric_limits<std::uint32_t>::max()
	public:
		Skeleton() = default;
		~Skeleton() = default;
	public:
		shade::SharedPointer<shade::Skeleton::BoneNode>& AddBone(const std::string& name, const glm::mat4& transform);
		shade::SharedPointer<shade::Skeleton::BoneArmature>& AddArmature(const glm::mat4& transform);

		const shade::SharedPointer<shade::Skeleton::BoneNode>& GetBone(const std::string& name) const;
		const shade::SharedPointer<shade::Skeleton::BoneNode>& GetBone(std::size_t id) const;

		const shade::SharedPointer<shade::Skeleton::BoneArmature>& GetArmature() const;
		const shade::SharedPointer<shade::Skeleton::BoneNode>& GetRootNode() const;

	public:
		std::unordered_map<std::string, SharedPointer<BoneNode>> m_BoneNodes;
		SharedPointer<BoneNode> m_RootNode;
		SharedPointer<BoneArmature> m_Armature;

		glm::mat4 m_GlobalInverseTransform;
	};
}