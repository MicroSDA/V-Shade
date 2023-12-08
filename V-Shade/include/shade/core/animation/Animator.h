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
        Animator(Animation* Animation, Skeleton* skeleton)
        {
            m_CurrentTime = 0.0;
            m_CurrentAnimation = Animation;
            m_CurrentSkeleton = skeleton;

            m_FinalBoneMatrices.reserve(250);

            for (int i = 0; i < 250; i++)
                m_FinalBoneMatrices.push_back(glm::mat4(1.0f));

            PlayAnimation(m_CurrentAnimation);
        }

        void UpdateAnimation(float dt, const Asset<shade::Mesh>& mesh);
       
        void CalculateBoneTransform(const Asset<shade::Mesh>& mesh, const SharedPointer<Skeleton::BoneNode>& bone, const glm::mat4& parentTransform);
        
        void PlayAnimation(Animation* pAnimation)
        {
            m_CurrentAnimation = pAnimation;
            m_CurrentTime = 0.0f;
        }

		std::vector<glm::mat4>& GetFinalBoneMatrices()
		{
			return m_FinalBoneMatrices;
		}
	private:
		std::vector<glm::mat4> m_FinalBoneMatrices;
		Animation* m_CurrentAnimation;
		Skeleton* m_CurrentSkeleton;
		float m_CurrentTime;
		float m_DeltaTime;
	};
}
