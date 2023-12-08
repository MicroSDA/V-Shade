#include "shade_pch.h"
#include "Animator.h"

void shade::Animator::UpdateAnimation(float dt, const Asset<shade::Mesh>& mesh)
{
    m_DeltaTime = dt;

    if (m_CurrentAnimation)
    {
        m_CurrentTime += static_cast<float>(m_CurrentAnimation->m_TicksPerSecond) * dt;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->m_Duration);

        CalculateBoneTransform(mesh, m_CurrentSkeleton->GetRootNode(), glm::mat4(1.0)); // Where root node is firt Bone
    }

}
void shade::Animator::CalculateBoneTransform(const Asset<shade::Mesh>& mesh, const SharedPointer<Skeleton::BoneNode>& bone, const glm::mat4& parentTransform)
{
    /*  The offsetMatrix of a bone is the inverse of that bone's global transform at bind pose. In other words, 
        if you traverse the bone/node hierarchy, applying the local transform (mTransformation in Assimp) of each bone/node hierarchically to its children,
        we get the global transform for each bone/node. The inverse of this matrix for a particular bone equals its offsetMatrix. 
        As implied here, it can be computed manually - regardless, 
        it is constant and should not be computed per frame.*/
    const auto animationChannel = m_CurrentAnimation->m_AnimationChanels.find(bone->Name);

    glm::mat4 globalMatrix = parentTransform;

    if (animationChannel != m_CurrentAnimation->m_AnimationChanels.end())
    {
        glm::mat4 translation           = m_CurrentAnimation->InterpolatePosition(bone->Name, m_CurrentTime);
        glm::mat4 rotation              = m_CurrentAnimation->InterpolateRotation(bone->Name, m_CurrentTime);
        glm::mat4 scale                 = m_CurrentAnimation->InterpolateScale(bone->Name, m_CurrentTime);
        glm::mat4 interpolatedTransfom  = translation * rotation * scale;

        globalMatrix                    *= interpolatedTransfom;

        auto& boneInfoMap = mesh->m_BoneInfoMap;
        if (boneInfoMap.find(bone->Name) != boneInfoMap.end())
        {
            int index = boneInfoMap.at(bone->Name).ID;
            glm::mat4 offset = boneInfoMap.at(bone->Name).Offset;
            m_FinalBoneMatrices[index] = globalMatrix * offset * m_CurrentSkeleton->GetArmature()->Transform;
        }

        // offset matrix calculating - ofsetMatrix for current node = inverse(ParentNodeTransform * NodeTransfor) recursivly 
    }
    else
    {
        globalMatrix *= glm::translate(glm::mat4(1.0f), bone->Translation) * glm::toMat4(bone->Rotation) * glm::scale(glm::mat4(1.0f), bone->Scale);
    }

    for (const auto& child : bone->Children)
        CalculateBoneTransform(mesh, child, globalMatrix);

}
