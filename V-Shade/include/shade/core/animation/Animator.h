#pragma once
#include <shade/core/animation/Animation.h>
#include <shade/core/animation/Skeleton.h>

// TEMPORARY 
#include <shade/core/render/drawable/Mesh.h>

namespace shade
{
	class SHADE_API Animator
	{
	public:
		Animator(const Asset<Animation>& animation);
        void UpdateAnimation(float deltaTime, const Asset<Skeleton>& skeleton);
        void CalculateBoneTransform(const SharedPointer<Skeleton::BoneNode>& bone, const glm::mat4& parentTransform, const SharedPointer<Skeleton::BoneArmature>& armature);

      
        const SharedPointer<std::vector<glm::mat4>>& GetFinalBoneMatrices() const;

	private:
        SharedPointer<std::vector<glm::mat4>> m_BoneTransforms;
        Asset<Animation> m_CurrentAnimation;
		float m_CurrentTime;
		float m_DeltaTime;
	};
}
