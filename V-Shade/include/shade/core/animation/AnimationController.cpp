#include "shade_pch.h"
#include "AnimationController.h"

void shade::AnimationController::AddAnimation(const Asset<Animation>& animation)
{
    // TODO: Check if exist 
    m_Animation[animation->GetAssetData()->GetId()] =
    {
        .Animation = animation,
        .Duration = animation->GetDuration(),
        .TiksPerSecond = animation->GetTiksPerSecond()
    };
    m_Animation[animation->GetAssetData()->GetId()].BoneTransforms = SharedPointer<std::vector<glm::mat4>>::Create();
    m_Animation[animation->GetAssetData()->GetId()].BoneTransforms->resize(RenderAPI::MAX_BONES_PER_INSTANCE, glm::mat4(1.f));
    m_CurrentAnimation = &m_Animation[animation->GetAssetData()->GetId()];
}

void shade::AnimationController::SetCurrentAnimation(const Asset<Animation>& animation, Animation::State state)
{
    // TODO: Check if exist 
    m_Animation[animation->GetAssetData()->GetId()].State = state;
    // TODO: Need to check if addres changed after resizing or something
    m_CurrentAnimation = &m_Animation[animation->GetAssetData()->GetId()];
}

void shade::AnimationController::SetCurrentAnimation(const std::string& name, Animation::State state)
{
    // TODO: Check if exist 
    m_Animation[name].State = state;
    // TODO: Need to check if addres changed after resizing or something
    m_CurrentAnimation = &m_Animation.at(name);
}

void shade::AnimationController::SetAnimationDuration(const Asset<Animation>& animation, float duration)
{
    m_Animation[animation->GetAssetData()->GetId()].Duration = duration;
}

void shade::AnimationController::SetAnimationDuration(const std::string& name, float duration)
{
    m_Animation[name].Duration = duration;
}

float shade::AnimationController::GetAnimationDuration(const Asset<Animation>& animation) const
{
    return m_Animation.at(animation->GetAssetData()->GetId()).Duration;
}

float shade::AnimationController::GetAnimationDuration(const std::string& name) const
{
    return m_Animation.at(name).Duration;
}

void shade::AnimationController::SetAnimationTiks(const Asset<Animation>& animation, float tiksPerSekond)
{
    m_Animation[animation->GetAssetData()->GetId()].TiksPerSecond = tiksPerSekond;
}

void shade::AnimationController::SetAnimationTiks(const std::string& name, float tiksPerSekond)
{
    m_Animation[name].TiksPerSecond = tiksPerSekond;
}

float shade::AnimationController::GetAnimationTiks(const Asset<Animation>& animation) const
{
    return m_Animation.at(animation->GetAssetData()->GetId()).TiksPerSecond;
}

float shade::AnimationController::GetAnimationTiks(const std::string& name) const
{
    return m_Animation.at(name).TiksPerSecond;
}

shade::AnimationController::AnimationControllData& shade::AnimationController::GetCurentAnimation()
{
    // Danger !!
    return *m_CurrentAnimation;
}

void shade::AnimationController::UpdateCurrentAnimation(const FrameTimer& deltaTime, const Asset<Skeleton>& skeleton)
{
    if (m_CurrentAnimation)
    {
        m_CurrentAnimation->CurrentPlayTime += m_CurrentAnimation->TiksPerSecond * deltaTime.GetInSeconds<float>();
        m_CurrentAnimation->CurrentPlayTime = fmod(m_CurrentAnimation->CurrentPlayTime, m_CurrentAnimation->Duration);

        CalculateBoneTransform(skeleton->GetRootNode(), glm::mat4(1.0), skeleton->GetArmature());
    }
}

const shade::SharedPointer<std::vector<glm::mat4>>& shade::AnimationController::GetBoneTransforms() const
{
   return m_CurrentAnimation->BoneTransforms;
}

shade::SharedPointer<shade::AnimationController> shade::AnimationController::Create()
{
    return SharedPointer<AnimationController>::Create();
}

void shade::AnimationController::CalculateBoneTransform(const SharedPointer<Skeleton::BoneNode>& bone, const glm::mat4& parentTransform, const SharedPointer<Skeleton::BoneArmature>& armature)
{
    glm::mat4 globalMatrix = parentTransform;
    const auto animationChannel = m_CurrentAnimation->Animation->GetAnimationCahnnels().find(bone->Name);
    if (animationChannel != m_CurrentAnimation->Animation->GetAnimationCahnnels().end())
    {
        glm::mat4 translation = m_CurrentAnimation->Animation->InterpolatePosition(animationChannel->second, m_CurrentAnimation->CurrentPlayTime);
        glm::mat4 rotation = m_CurrentAnimation->Animation->InterpolateRotation(animationChannel->second, m_CurrentAnimation->CurrentPlayTime);
        glm::mat4 scale = m_CurrentAnimation->Animation->InterpolateScale(animationChannel->second, m_CurrentAnimation->CurrentPlayTime);
        glm::mat4 interpolatedTransfom = translation * rotation * scale;

        globalMatrix *= interpolatedTransfom;

        m_CurrentAnimation->BoneTransforms.Get()[bone->ID] = globalMatrix * bone->InverseBindPose * armature->Transform;
    }
    else
    {
        globalMatrix *= glm::translate(glm::mat4(1.0f), bone->Translation) * glm::toMat4(bone->Rotation) * glm::scale(glm::mat4(1.0f), bone->Scale);
    }

    for (const auto& child : bone->Children)
        CalculateBoneTransform(child, globalMatrix, armature);
}
