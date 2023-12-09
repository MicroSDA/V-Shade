#include "shade_pch.h"
#include "Animator.h"

void shade::Animator::UpdateAnimation(float deltaTime, const Asset<Skeleton>& skeleton)
{
    m_DeltaTime = deltaTime;

    if (m_CurrentAnimation)
    {
        m_CurrentTime += m_CurrentAnimation->GetTiksPerSecond() * deltaTime;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDureation());

        CalculateBoneTransform(skeleton->GetRootNode(), glm::mat4(1.0), skeleton->GetArmature()); // Where root node is firt Bone
    }

}
void shade::Animator::CalculateBoneTransform(const SharedPointer<Skeleton::BoneNode>& bone, const glm::mat4& parentTransform, const SharedPointer<Skeleton::BoneArmature>& armature)
{
    /*  The offsetMatrix of a bone is the inverse of that bone's global transform at bind pose. In other words, 
        if you traverse the bone/node hierarchy, applying the local transform (mTransformation in Assimp) of each bone/node hierarchically to its children,
        we get the global transform for each bone/node. The inverse of this matrix for a particular bone equals its offsetMatrix. 
        As implied here, it can be computed manually - regardless, 
        it is constant and should not be computed per frame.*/

    glm::mat4 globalMatrix = parentTransform;
    const auto animationChannel = m_CurrentAnimation->GetAnimationCahnnels().find(bone->Name);
    if (animationChannel != m_CurrentAnimation->GetAnimationCahnnels().end())
    {
        glm::mat4 translation           = m_CurrentAnimation->InterpolatePosition(animationChannel->second, m_CurrentTime);
        glm::mat4 rotation              = m_CurrentAnimation->InterpolateRotation(animationChannel->second, m_CurrentTime);
        glm::mat4 scale                 = m_CurrentAnimation->InterpolateScale(animationChannel->second, m_CurrentTime);
        glm::mat4 interpolatedTransfom  = translation * rotation * scale;

        globalMatrix                    *= interpolatedTransfom;

        m_FinalBoneMatrices[bone->ID] = globalMatrix * bone->InverseBindPose * armature->Transform;
    }
    else
    {
        globalMatrix *= glm::translate(glm::mat4(1.0f), bone->Translation) * glm::toMat4(bone->Rotation) * glm::scale(glm::mat4(1.0f), bone->Scale);
    }

    for (const auto& child : bone->Children)
        CalculateBoneTransform(child, globalMatrix, armature);
}
