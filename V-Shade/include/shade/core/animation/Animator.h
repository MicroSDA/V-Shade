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

            m_FinalBoneMatrices.reserve(100);

            for (int i = 0; i < 100; i++)
                m_FinalBoneMatrices.push_back(glm::mat4(1.0f));

            PlayAnimation(m_CurrentAnimation);
        }

        void UpdateAnimation(float dt, const Asset<shade::Mesh>& mesh)
        {
            m_DeltaTime = dt;

            for (auto& mat : m_FinalBoneMatrices)
            {
                mat = glm::mat4(1.0f);
            }
           
            if (m_CurrentAnimation)
            {
                m_CurrentTime += static_cast<float>(m_CurrentAnimation->m_TicksPerSecond) * dt;
                m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->m_Duration);

                CalculateBoneTransform(mesh, m_CurrentSkeleton->m_RootBone, glm::mat4(1.0f));
            }

        }
        void CalculateBoneTransform(const Asset<shade::Mesh>& mesh, const SharedPointer<Skeleton::BoneNode>& bone, const glm::mat4& parentTransform)
        {
            std::string name = bone->Name;
            glm::mat4 nodeTranform = glm::translate(glm::mat4(1.0f), bone->Translation) * glm::toMat4(bone->Rotation) * glm::scale(glm::mat4(1.0f), bone->Scale);

            auto chanel = m_CurrentAnimation->m_AnimationChanels.find(name);

            if (chanel != m_CurrentAnimation->m_AnimationChanels.end())
            {
                glm::mat4 translation = m_CurrentAnimation->InterpolatePosition(name, m_CurrentTime);
                glm::mat4 rotation = m_CurrentAnimation->InterpolateRotation(name, m_CurrentTime);
                glm::mat4 scale = m_CurrentAnimation->InterpolateScale(name, m_CurrentTime);
                nodeTranform = translation * rotation * scale;
            }

            glm::mat4 globalTransformation = parentTransform * nodeTranform;

            auto& boneInfoMap = mesh->m_BoneInfoMap;
            if (boneInfoMap.find(name) != boneInfoMap.end())
            {
                int index = boneInfoMap.at(name).ID;
                glm::mat4 offset = boneInfoMap.at(name).Offset;
                m_FinalBoneMatrices[index] = globalTransformation * offset;
            }
            else
            {
                SHADE_CORE_INFO("Missing {0}", name);
            }
            
            for (const auto& child: bone->Children)
                CalculateBoneTransform(mesh, child, globalTransformation);
        }
        glm::mat4 CalculateBoneTransform(const Asset<shade::Mesh>& mesh, const std::string& chanelName, const Animation::Chanel& chanel, const Skeleton& skeleton, const glm::mat4& parentTransform)
        {
            // Without parent or child righ now !
            auto boneId  = skeleton.GetBoneID(chanelName);
            if (boneId != Skeleton::NULL_BONE_ID)
            {
                const auto& data = skeleton.GetBoneData(boneId);

                glm::mat4 nodeTranform = glm::translate(glm::mat4(1.f), data.Translation) * glm::toMat4(glm::quat((data.Rotation))) * glm::scale(glm::mat4(1.f), data.Scale);

                if (boneId != Skeleton::NULL_BONE_ID)
                {
                    //1.Interpolate position;
                    //2.Interpolate rotation;
                    //3.Interpolate scale;

                    glm::mat4 translation = m_CurrentAnimation->InterpolatePosition(chanelName, m_CurrentTime);
                    glm::mat4 rotation = m_CurrentAnimation->InterpolateRotation(chanelName, m_CurrentTime);
                    glm::mat4 scale  = m_CurrentAnimation->InterpolateScale(chanelName, m_CurrentTime);
                    nodeTranform = translation * rotation * scale;
                }

                glm::mat4 globalTransformation = parentTransform * nodeTranform;
                m_FinalBoneMatrices[boneId] = globalTransformation;

                auto& boneInfoMap = mesh->m_BoneInfoMap;
                if (boneInfoMap.find(chanelName) != boneInfoMap.end())
                {
                    int index = boneInfoMap.at(chanelName).ID;
                    glm::mat4 offset = boneInfoMap.at(chanelName).Offset;
                    m_FinalBoneMatrices[index] = globalTransformation * offset;
                }

                return nodeTranform;
            }
            return glm::mat4(1.0);
        }
       
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
