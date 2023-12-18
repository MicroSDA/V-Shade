#include "shade_pch.h"
#include "AnimationController.h"
#include <glm/glm/gtx/common.hpp>

void shade::AnimationController::AddAnimation(const Asset<Animation>& animation)
{
    if (IsAnimationExists(animation))
    {
        SHADE_CORE_ERROR("Animation '{}' already exists in Animation controller!", animation->GetAssetData()->GetId());
        assert(false);
    }
    else
    {
        m_Animations.insert({ animation,
                   {
                       .Animation = animation,
                       .Duration = animation->GetDuration(),
                       .TiksPerSecond = animation->GetTiksPerSecond(),
                       .BoneTransforms = SharedPointer<std::vector<glm::mat4>>::Create(RenderAPI::MAX_BONES_PER_INSTANCE, glm::mat4(1.f))
                   }
            });

        m_CurrentAnimation = &m_Animations.at(animation);
    }  
}

void shade::AnimationController::SetCurrentAnimation(const Asset<Animation>& animation, Animation::State state)
{
    if (!IsAnimationExists(animation))
    {
        SHADE_CORE_WARNING("Animation '{0}' doesn't exist in Animation controller!", animation->GetAssetData()->GetId());
        assert(false); 
    }
    else
    {
        m_CurrentAnimation = &m_Animations.at(animation);
        m_CurrentAnimation->State = state;

        if (state == Animation::State::Stop)
            m_CurrentAnimation->CurrentPlayTime = 0.f;
    }
}

void shade::AnimationController::SetCurrentAnimation(const std::string& name, Animation::State state)
{
    for (const auto& [animation, data] : m_Animations)
    {
        if (animation->GetAssetData()->GetId() == name)
        {
            m_CurrentAnimation = &m_Animations.at(animation); m_CurrentAnimation->State = state; 
            
            if (state == Animation::State::Stop)
                m_CurrentAnimation->CurrentPlayTime = 0.f;

            return;
        }
    }
   
    SHADE_CORE_ERROR("Animation '{0}' doesn't exist in Animation controller!", name);
}

void shade::AnimationController::SetAnimationDuration(const Asset<Animation>& animation, float duration)
{
    if (!IsAnimationExists(animation))
    {
        SHADE_CORE_WARNING("Animation '{0}' doesn't exist in Animation controller!", animation->GetAssetData()->GetId());
        assert(false);
    }
    else
    {
        m_Animations.at(animation).Duration = duration;
    }
}

void shade::AnimationController::SetAnimationDuration(const std::string& name, float duration)
{
    for (auto& [animation, data] : m_Animations)
    {
        if (animation->GetAssetData()->GetId() == name)
        {
            data.Duration = duration; return;
        }
    }

    SHADE_CORE_ERROR("Animation '{0}' doesn't exist in Animation controller!", name);
}

float& shade::AnimationController::GetAnimationDuration(const Asset<Animation>& animation)
{
    if (!IsAnimationExists(animation))
    {
        SHADE_CORE_WARNING("Animation '{0}' doesn't exist in Animation controller!", animation->GetAssetData()->GetId());
        assert(false); // you cannot return default value by reference
    }
    else 
    {
        return m_Animations.at(animation).Duration;
    }
}

float& shade::AnimationController::GetAnimationDuration(const std::string& name)
{
    for (auto& [animation, data] : m_Animations)
    {
        if (animation->GetAssetData()->GetId() == name)
        {
            return data.Duration;
        }
    }

    SHADE_CORE_ERROR("Animation '{0}' doesn't exist in Animation controller!", name);
}

void shade::AnimationController::SetAnimationTiks(const Asset<Animation>& animation, float tiksPerSekond)
{
    if (!IsAnimationExists(animation))
    {
        SHADE_CORE_WARNING("Animation '{0}' doesn't exist in Animation controller!", animation->GetAssetData()->GetId());
        assert(false);
    }
    else
    {
        m_Animations.at(animation).TiksPerSecond = tiksPerSekond;
    }
}

void shade::AnimationController::SetAnimationTiks(const std::string& name, float tiksPerSekond)
{
    for (auto& [animation, data] : m_Animations)
    {
        if (animation->GetAssetData()->GetId() == name)
        {
            data.Duration = tiksPerSekond; return;
        }
    }

    SHADE_CORE_ERROR("Animation '{0}' doesn't exist in Animation controller!", name);
}

float& shade::AnimationController::GetAnimationTiks(const Asset<Animation>& animation)
{
    if (!IsAnimationExists(animation))
    {
        SHADE_CORE_WARNING("Animation '{0}' doesn't exist in Animation controller!", animation->GetAssetData()->GetId());
        assert(false); // you cannot return default value by reference
    }
    else 
    {
        return m_Animations.at(animation).TiksPerSecond;
    }
}

float& shade::AnimationController::GetAnimationTiks(const std::string& name)
{
    for (auto& [animation, data] : m_Animations)
    {
        if (animation->GetAssetData()->GetId() == name)
        {
            return data.TiksPerSecond;
        }
    }

    SHADE_CORE_ERROR("Animation '{0}' doesn't exist in Animation controller!", name);
}

void shade::AnimationController::SetAnimationState(const Asset<Animation>& animation, Animation::State state)
{
    if (!IsAnimationExists(animation))
    {
        SHADE_CORE_WARNING("Animation '{0}' doesn't exist in Animation controller!", animation->GetAssetData()->GetId());
        assert(false);
    }
    else
    {
        m_Animations.at(animation).State = state;

        if (state == Animation::State::Stop)
        {
            m_Animations.at(animation).CurrentPlayTime = 0.f;

            for (auto& transform : *m_Animations.at(animation).BoneTransforms)
                transform = glm::mat4(1.f);
        }
    }
}

void shade::AnimationController::SetAnimationState(const std::string& name, Animation::State state)
{
    for (auto& [animation, data] : m_Animations)
    {
        if (animation->GetAssetData()->GetId() == name)
        {
           data.State = state; 
           if (state == Animation::State::Stop)
           {
               data.CurrentPlayTime = 0.f;

               for (auto& transform : *m_Animations.at(animation).BoneTransforms)
                   transform = glm::mat4(1.f);
           }
           return;
        }
    }

    SHADE_CORE_ERROR("Animation '{0}' doesn't exist in Animation controller!", name);
}

shade::Animation::State shade::AnimationController::GetAnimationState(const Asset<Animation>& animation) const
{
    if (!IsAnimationExists(animation))
    {
        SHADE_CORE_WARNING("Animation '{0}' doesn't exist in Animation controller!", animation->GetAssetData()->GetId());
        assert(false); return shade::Animation::State::Stop;
    }
    else 
    {
        return m_Animations.at(animation).State;
    }
}

shade::Animation::State shade::AnimationController::GetAnimationState(const std::string& name) const
{
    for (auto& [animation, data] : m_Animations)
    {
        if (animation->GetAssetData()->GetId() == name)
        {
            return data.State;
        }
    }

    SHADE_CORE_ERROR("Animation '{0}' doesn't exist in Animation controller!", name);
}

shade::Asset<shade::Animation> shade::AnimationController::GetCurentAnimation()
{
    return (m_CurrentAnimation) ? m_CurrentAnimation->Animation : nullptr;
}

std::unordered_map<shade::Asset<shade::Animation>, shade::AnimationController::AnimationControllData>& shade::AnimationController::GetAnimations()
{
    return m_Animations;
}

void shade::AnimationController::UpdateCurrentAnimation(const Asset<Skeleton>& skeleton, const FrameTimer& deltaTime)
{
    assert(skeleton != nullptr && "Skeleton is nullptr!");

    if (m_CurrentAnimation)
    {
        if (m_CurrentAnimation->State == Animation::State::Play)
        {
            m_CurrentAnimation->CurrentPlayTime += m_CurrentAnimation->TiksPerSecond * deltaTime.GetInSeconds<float>();
            m_CurrentAnimation->CurrentPlayTime = glm::fmod(m_CurrentAnimation->CurrentPlayTime, m_CurrentAnimation->Duration);
        }
        if (m_CurrentAnimation->State != Animation::State::Stop)
        {
            CalculateBoneTransform(skeleton->GetRootNode(), glm::mat4(1.0), skeleton->GetArmature());
        }
    }
}

void shade::AnimationController::Blend(const Asset<Skeleton>& skeleton, const Asset<Animation>& sourceAnimation, const Asset<Animation>& targetAnimation, const FrameTimer& deltaTime)
{
    assert(sourceAnimation != nullptr && targetAnimation != nullptr);
    auto sourceData = m_Animations.find(sourceAnimation), targetData = m_Animations.find(targetAnimation);
  
    if (sourceData != m_Animations.end() && targetData != m_Animations.end())
    {
        if (sourceData->second.State == Animation::State::Play)
        {
            auto [source, target] = GetTimeMultiplier(sourceData->second, targetData->second, BlendFactor);

            sourceData->second.CurrentPlayTime += sourceData->second.TiksPerSecond * deltaTime.GetInSeconds<float>() * ((IsSync) ? source : 1.0);
            sourceData->second.CurrentPlayTime = glm::fmod(sourceData->second.CurrentPlayTime, sourceData->second.Duration);

            targetData->second.CurrentPlayTime += targetData->second.TiksPerSecond * deltaTime.GetInSeconds<float>() * ((IsSync) ? target : 1.0);
            targetData->second.CurrentPlayTime = fmod(targetData->second.CurrentPlayTime, targetData->second.Duration);

        }

        if (sourceData->second.State != Animation::State::Stop)
        {
            CalculateBlendedBoneTransform(skeleton->GetRootNode(),
                sourceData->second,
                sourceData->second.CurrentPlayTime,

                targetData->second,
                targetData->second.CurrentPlayTime,

                glm::mat4(1.0),
                skeleton->GetArmature(),
                BlendFactor);
        }
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
        glm::mat4 translation           = m_CurrentAnimation->Animation->InterpolatePosition(animationChannel->second, m_CurrentAnimation->CurrentPlayTime);
        glm::mat4 rotation              = m_CurrentAnimation->Animation->InterpolateRotation(animationChannel->second, m_CurrentAnimation->CurrentPlayTime);
        glm::mat4 scale                 = m_CurrentAnimation->Animation->InterpolateScale(animationChannel->second, m_CurrentAnimation->CurrentPlayTime);
        glm::mat4 interpolatedTransfom  = translation * rotation * scale;

        globalMatrix *= interpolatedTransfom;

        m_CurrentAnimation->BoneTransforms.Get()[bone->ID] = globalMatrix * bone->InverseBindPose * armature->Transform; 
        //m_CurrentAnimation->BoneTransforms.Get()[bone->ID] = globalMatrix * bone->InverseBindPose;
    }
    else
    {
        globalMatrix *= glm::translate(glm::mat4(1.0f), bone->Translation) * glm::toMat4(bone->Rotation) * glm::scale(glm::mat4(1.0f), bone->Scale);
    }

    for (const auto& child : bone->Children)
        CalculateBoneTransform(child, globalMatrix, armature);
}

void shade::AnimationController::CalculateBlendedBoneTransform(const SharedPointer<Skeleton::BoneNode>& bone,
    const AnimationControllData& sourceAnimation,
    float sourceAnimationTime, 
    const AnimationControllData& targetAnimation,
    float targetAnimationTime,
    const glm::mat4& parentTransform, 
    const SharedPointer<Skeleton::BoneArmature>& armature, 
    float blendFactor)
{
    glm::mat4 sourceGlobalMaxtrix = glm::mat4(1.f), targetGlobalMaxtrix = glm::mat4(1.f);

    const auto sourceChinnels = sourceAnimation.Animation->GetAnimationCahnnels().find(bone->Name);
    const auto targetChannels = targetAnimation.Animation->GetAnimationCahnnels().find(bone->Name);

    if (sourceChinnels != sourceAnimation.Animation->GetAnimationCahnnels().end() && targetChannels != targetAnimation.Animation->GetAnimationCahnnels().end())
    {
        {
            glm::mat4 translation   = sourceAnimation.Animation->InterpolatePosition(sourceChinnels->second, sourceAnimationTime);
            glm::mat4 rotation      = sourceAnimation.Animation->InterpolateRotation(sourceChinnels->second, sourceAnimationTime);
            glm::mat4 scale         = sourceAnimation.Animation->InterpolateScale(sourceChinnels->second, sourceAnimationTime);
            glm::mat4 interpolatedTransfom = translation * rotation * scale;

            sourceGlobalMaxtrix = interpolatedTransfom;
        }
        {
            glm::mat4 translation = targetAnimation.Animation->InterpolatePosition(targetChannels->second, targetAnimationTime);
            glm::mat4 rotation = targetAnimation.Animation->InterpolateRotation(targetChannels->second, targetAnimationTime);
            glm::mat4 scale = targetAnimation.Animation->InterpolateScale(targetChannels->second, targetAnimationTime);
            glm::mat4 interpolatedTransfom = translation * rotation * scale;

            targetGlobalMaxtrix = interpolatedTransfom;
        }


        // Blend two matrices
        const glm::quat sourceRotation = glm::quat_cast(sourceGlobalMaxtrix);
        const glm::quat targetRotation = glm::quat_cast(targetGlobalMaxtrix);
      
        glm::mat4 combined = glm::mat4_cast(glm::slerp(sourceRotation, targetRotation, blendFactor));

        //combined[3] = sourceGlobalMaxtrix[3] * blendFactor + targetGlobalMaxtrix[3] * (1.f - blendFactor);
        combined[3] = (1.0f - blendFactor) * sourceGlobalMaxtrix[3] + targetGlobalMaxtrix[3] * blendFactor;
      
        sourceGlobalMaxtrix = parentTransform * combined;

        m_CurrentAnimation->BoneTransforms.Get()[bone->ID] = sourceGlobalMaxtrix * bone->InverseBindPose * armature->Transform;
    }

    for (const auto& child : bone->Children)
        CalculateBlendedBoneTransform(child, sourceAnimation, sourceAnimationTime, targetAnimation, targetAnimationTime, sourceGlobalMaxtrix, armature, blendFactor);
}

std::pair<float, float> shade::AnimationController::GetTimeMultiplier(const AnimationControllData& first, const AnimationControllData& second, float blendFactor) const
{
    float difference = first.Duration / second.Duration;
    const float up = (1.0f - BlendFactor) * 1.0f + difference * blendFactor;

    difference = second.Duration / first.Duration;
    const float down = (1.0f - BlendFactor) * difference + 1.0f * blendFactor;

    return { up, down }; 
}

bool shade::AnimationController::IsAnimationExists(const Asset<Animation>& animation) const
{
    return (m_Animations.find(animation) != m_Animations.end());
}

float& shade::AnimationController::GetCurrentAnimationTime(const Asset<Animation>& animation)
{
    if (!IsAnimationExists(animation))
    {
        SHADE_CORE_WARNING("Animation '{0}' doesn't exist in Animation controller!", animation->GetAssetData()->GetId());
        assert(false); // you cannot return default value by reference;
    }
    else 
    {
        return m_Animations.at(animation).CurrentPlayTime;
    }
}
float& shade::AnimationController::GetCurrentAnimationTime(const std::string& name)
{
    for (auto& [animation, data] : m_Animations)
    {
        if (animation->GetAssetData()->GetId() == name)
        {
            return data.CurrentPlayTime;
        }
    }

    SHADE_CORE_ERROR("Animation '{0}' doesn't exist in Animation controller!", name);
}

void shade::AnimationController::RemoveAnimation(const Asset<Animation>& animation)
{
    if (!IsAnimationExists(animation))
    {
        SHADE_CORE_WARNING("Animation '{0}' doesn't exist in Animation controller!", animation->GetAssetData()->GetId());
        assert(false);
    }
    else
    {
        if (m_CurrentAnimation->Animation.Raw() == animation.Raw())
            m_CurrentAnimation = nullptr;

        m_Animations.erase(animation);
    }
}

void shade::AnimationController::RemoveAnimation(const std::string& name)
{
    for (auto& [animation, data] : m_Animations)
    {
        if (animation->GetAssetData()->GetId() == name)
        {
            if (m_CurrentAnimation->Animation.Raw() == animation.Raw())
                m_CurrentAnimation = nullptr;

            m_Animations.erase(animation); break;
        }
    }
}
