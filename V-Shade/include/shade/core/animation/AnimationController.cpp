#include "shade_pch.h"
#include "AnimationController.h"
#include <glm/glm/gtx/common.hpp>

shade::AnimationController::AnimationController() :
    m_DefaultPose(SharedPointer<animation::Pose>::Create(nullptr))
{

}

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

shade::SharedPointer<shade::AnimationController> shade::AnimationController::Create()
{
    return SharedPointer<AnimationController>::Create();
}

void shade::AnimationController::CalculateBoneTransforms(SharedPointer<animation::Pose>& pose, const Asset<Animation>& animation, const SharedPointer<Skeleton::BoneNode>& bone, const glm::mat4& parentTransform, const SharedPointer<Skeleton::BoneArmature>& armature)
{
    glm::mat4 localMatrix  = glm::identity<glm::mat4>(), globalMatrix = parentTransform;

    const auto animationChannel = animation->GetAnimationCahnnels().find(bone->Name);

    if (animationChannel != animation->GetAnimationCahnnels().end())
    {
        glm::mat4 translation = animation->InterpolatePosition(animationChannel->second, pose->GetCurrentPlayTime());
        glm::mat4 rotation = animation->InterpolateRotation(animationChannel->second, pose->GetCurrentPlayTime());
        glm::mat4 scale = animation->InterpolateScale(animationChannel->second, pose->GetCurrentPlayTime());
        glm::mat4 interpolatedTransfom = translation * rotation * scale;

        localMatrix *= interpolatedTransfom;
        globalMatrix *= interpolatedTransfom;
      
        pose->GetBoneLocalTransform(bone->ID)  = localMatrix;
        pose->GetBoneGlobalTransform(bone->ID) = globalMatrix * bone->InverseBindPose * armature->Transform;
    }
    else
    {
        globalMatrix *= parentTransform * glm::translate(glm::mat4(1.0f), bone->Translation) * glm::toMat4(bone->Rotation) * glm::scale(glm::mat4(1.0f), bone->Scale);
    }

    for (const auto& child : bone->Children)
        CalculateBoneTransforms(pose, animation, child, globalMatrix, armature);
}

void shade::AnimationController::CalculateBoneTransformsBlend(const shade::SharedPointer<shade::Skeleton::BoneNode>& bone, shade::SharedPointer<shade::animation::Pose>& targetPose, const shade::SharedPointer<shade::animation::Pose>& first, const shade::SharedPointer<shade::animation::Pose>& second, const glm::mat4& parrentTransform, float blendFactor)
{
    const glm::mat4& firstPose = first->GetBoneLocalTransform(bone->ID);
    const glm::mat4& secondPose = second->GetBoneLocalTransform(bone->ID);

    glm::mat4 combined = glm::mat4_cast(glm::slerp(glm::quat_cast(firstPose), glm::quat_cast(secondPose), blendFactor));

    combined[3] = (1.0f - blendFactor) * firstPose[3] + secondPose[3] * blendFactor;

    glm::mat4 locallMatrix = combined, globalMatrix = parrentTransform * combined;

    targetPose->GetBoneLocalTransform(bone->ID) = locallMatrix;
    targetPose->GetBoneGlobalTransform(bone->ID) = globalMatrix * bone->InverseBindPose * targetPose->GetSkeleton()->GetArmature()->Transform;

    for (const auto& child : bone->Children)
        CalculateBoneTransformsBlend(child, targetPose, first, second, globalMatrix, blendFactor);
}

std::pair<float, float> shade::AnimationController::GetTimeMultiplier(float firstDuration, float secondDuration, float blendFactor) const
{
    float difference = firstDuration / secondDuration;
    const float up = (1.0f - blendFactor) * 1.0f + difference * blendFactor;

    difference = secondDuration / firstDuration;
    const float down = (1.0f - blendFactor) * difference + 1.0f * blendFactor;

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

shade::SharedPointer<shade::animation::Pose>& shade::AnimationController::CalculatePose(SharedPointer<animation::Pose>& targetPose, const Asset<Animation>& animation, float from, float till, const FrameTimer& deltaTime, float timeMultiplier)
{
    float currentPlayTime = targetPose->GetCurrentPlayTime();

    currentPlayTime += animation->GetTiksPerSecond() * deltaTime.GetInSeconds<float>() * timeMultiplier;
    currentPlayTime = glm::fmod(currentPlayTime, animation->GetDuration());

    targetPose->SetCurrentPlayTime(currentPlayTime);
  
    CalculateBoneTransforms(targetPose, animation, targetPose->GetSkeleton()->GetRootNode(), glm::mat4(1.0), targetPose->GetSkeleton()->GetArmature());

    return targetPose;
}

shade::SharedPointer<shade::animation::Pose>& shade::AnimationController::Blend(SharedPointer<animation::Pose>& targetPose, SharedPointer<animation::Pose>& first, SharedPointer<animation::Pose>& second, float blendFactor)
{
    CalculateBoneTransformsBlend(targetPose->GetSkeleton()->GetRootNode(), targetPose, first, second, glm::identity<glm::mat4>(), blendFactor);
    return targetPose;
}

shade::SharedPointer<shade::animation::Pose>& shade::AnimationController::CreatePose(std::size_t hash, const Asset<Skeleton>& skeleton)
{
    if (m_Poses[hash] == nullptr) m_Poses.at(hash) = SharedPointer<animation::Pose>::Create(skeleton);

    return m_Poses.at(hash);
}

void shade::AnimationController::ProcessPose(const Asset<Animation>& animation, float from, float till)
{
    assert(animation != nullptr);

    m_QuerryPose = [=](const Asset<Skeleton>& skeleton, const FrameTimer& deltaTime) mutable
        {
            return CalculatePose(ReceiveAnimationPose(skeleton, animation), animation, from, till, deltaTime);
        };
}

void shade::AnimationController::ProcessPose(const Asset<Animation>& first, float firstFrom, float firstTill, const Asset<Animation>& second, float secondFrom, float secondTill, float blendFactor, bool isSync)
{
    assert(first != nullptr || second != nullptr);

    m_QuerryPose = [=](const Asset<Skeleton>& skeleton, const FrameTimer& deltaTime) mutable
        {
            auto& firstPose  = ReceiveAnimationPose(skeleton, first);
            auto& secondPose = ReceiveAnimationPose(skeleton, second);

            if (isSync)
            {
                // TODO : Use firstDuration = firstTill - firstFrom,  secondDuration = secondTill - secondFrom for multiplier
                auto [up, down] = GetTimeMultiplier(first->GetDuration(), second->GetDuration(), blendFactor);

                CalculatePose(firstPose, first, firstFrom, firstTill, deltaTime, up);
                CalculatePose(secondPose, second, secondFrom, secondTill, deltaTime, down);
            }
            else
            {
                CalculatePose(firstPose, first, firstFrom, firstTill, deltaTime);
                CalculatePose(secondPose, second, secondFrom, secondTill, deltaTime);
            }
           
            return Blend(ReceiveAnimationPose(skeleton, first, second), firstPose, secondPose, blendFactor);
        };
}

shade::SharedPointer<shade::animation::Pose> shade::AnimationController::QuerryPose(const Asset<Skeleton>& skeleton, const FrameTimer& deltaTime)
{
    if (m_QuerryPose != nullptr)
    {
        auto pose = m_QuerryPose(skeleton, deltaTime); //m_QuerryPose = nullptr;
        return pose;
    }
    else
        return m_DefaultPose;
}

shade::SharedPointer<shade::animation::Pose>& shade::AnimationController::ReceiveAnimationPose(const Asset<Skeleton>& skeleton, const Asset<Animation>& animation)
{
    return CreatePose(animation::PointerHashCombine(animation), skeleton);
}

shade::animation::Pose::Pose(const Asset<Skeleton>& skeleton, Type initial) :
    m_Skeleton(skeleton), 
    m_GlobalTransforms(SharedPointer<std::vector<glm::mat4>>::Create()),
    m_LocalTransforms(SharedPointer<std::vector<glm::mat4>>::Create()),
    m_State(State::Unset)
{
    switch (initial)
    {
        case Type::DontKnow1:
        {
            Reset();
        }
    }  
}

shade::animation::Pose::Pose(Type initial) :
    m_GlobalTransforms(SharedPointer<std::vector<glm::mat4>>::Create()),
    m_LocalTransforms(SharedPointer<std::vector<glm::mat4>>::Create()),
    m_State(State::ZeroPose)
{
    switch (initial)
    {
        case Type::DontKnow1:
        {
            Reset();
        }
    }
}

void shade::animation::Pose::Reset()
{
    m_GlobalTransforms->clear();
    m_GlobalTransforms->resize(RenderAPI::MAX_BONES_PER_INSTANCE, glm::identity<glm::mat4>());

    m_LocalTransforms->clear();
    m_LocalTransforms->resize(RenderAPI::MAX_BONES_PER_INSTANCE, glm::identity<glm::mat4>());

    m_State = State::ZeroPose;
}