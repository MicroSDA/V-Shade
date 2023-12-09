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
        Animator(const Asset<Animation>& animation)
        {
            m_CurrentTime = 0.0;
            m_CurrentAnimation = animation;

            m_FinalBoneMatrices.reserve(250);

            for (int i = 0; i < 250; i++)
                m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
        }

        void UpdateAnimation(float deltaTime, const Asset<Skeleton>& skeleton);
        void CalculateBoneTransform(const SharedPointer<Skeleton::BoneNode>& bone, const glm::mat4& parentTransform, const SharedPointer<Skeleton::BoneArmature>& armature);

      
		std::vector<glm::mat4>& GetFinalBoneMatrices()
		{
			return m_FinalBoneMatrices;
		}
	private:
		std::vector<glm::mat4> m_FinalBoneMatrices;
        Asset<Animation> m_CurrentAnimation;
		float m_CurrentTime;
		float m_DeltaTime;
	};
}
