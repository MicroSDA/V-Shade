#include "shade_pch.h"
#include "AnimationController.h"
#include <glm/glm/gtx/common.hpp>

void shade::animation::AnimationController::CalculateBoneTransforms(
	animation::Pose* pose,
	const  AnimationControlData& animationData,
	const Skeleton::BoneNode* bone,
	const glm::mat4& parentTransform,
	const Skeleton::BoneArmature& armature)
{
	glm::mat4 localMatrix = glm::identity<glm::mat4>(), globalMatrix = parentTransform;

	const auto animationChannel = animationData.Animation->GetAnimationCahnnels().find(bone->Name);

	if (animationChannel != animationData.Animation->GetAnimationCahnnels().end())
	{
		glm::mat4 translation = animationData.Animation->InterpolatePosition(animationChannel->second, animationData.CurrentPlayTime);
		glm::mat4 rotation = animationData.Animation->InterpolateRotation(animationChannel->second, animationData.CurrentPlayTime);
		glm::mat4 scale = animationData.Animation->InterpolateScale(animationChannel->second, animationData.CurrentPlayTime);
		glm::mat4 interpolatedTransfom = translation * rotation * scale;

		localMatrix *= interpolatedTransfom;
		globalMatrix *= interpolatedTransfom;

		pose->GetBoneLocalTransform(bone->ID) = localMatrix;
		pose->GetBoneGlobalTransform(bone->ID) = globalMatrix * bone->InverseBindPose * armature.Transform;
	}
	else
	{
		globalMatrix *= parentTransform * glm::translate(glm::mat4(1.0f), bone->Translation) * glm::toMat4(bone->Rotation) * glm::scale(glm::mat4(1.0f), bone->Scale);
	}

	for (const auto& child : bone->Children)
		CalculateBoneTransforms(pose, animationData, child, globalMatrix, armature);
}

void shade::animation::AnimationController::CalculateBoneTransformsBlend(
	const shade::Skeleton::BoneNode* bone, 
	animation::Pose* targetPose,
	const animation::Pose* first,
	const animation::Pose* second, 
	const glm::mat4& parrentTransform, 
	float blendFactor, 
	const animation::BoneMask& boneMask)
{
	const glm::mat4& firstPose = first->GetBoneLocalTransform(bone->ID);
	const glm::mat4& secondPose = second->GetBoneLocalTransform(bone->ID);

	float blendFactorWithBoneMask = blendFactor * boneMask.GetWeight(bone->ID);

	glm::mat4 combined	    = glm::mat4_cast(glm::slerp(glm::quat_cast(firstPose), glm::quat_cast(secondPose), blendFactorWithBoneMask));
	//glm::vec3 translate	    = glm::mix(firstPose[3], secondPose[3], blendFactor);
	//glm::vec3 scale		= glm::mix(firstPose[3], secondPose[3], blendFactor);

	//result->RootMotion.Translation = glm::mix(poseA->RootMotion.Translation, poseB->RootMotion.Translation, w);


	combined[3] = (1.0f - blendFactorWithBoneMask) * firstPose[3] + secondPose[3] * blendFactorWithBoneMask;

	glm::mat4 locallMatrix = combined, globalMatrix = parrentTransform * combined;

	targetPose->GetBoneLocalTransform(bone->ID) = locallMatrix;
	targetPose->GetBoneGlobalTransform(bone->ID) = globalMatrix * bone->InverseBindPose * targetPose->GetSkeleton()->GetArmature().Transform;

	for (const auto& child : bone->Children)
		CalculateBoneTransformsBlend(child, targetPose, first, second, globalMatrix, blendFactor, boneMask);
}

std::pair<float, float> shade::animation::AnimationController::GetTimeMultiplier(float firstDuration, float secondDuration, float blendFactor) const
{
	float difference = firstDuration / secondDuration;
	const float up = (1.0f - blendFactor) * 1.0f + difference * blendFactor;

	difference = secondDuration / firstDuration;
	const float down = (1.0f - blendFactor) * difference + 1.0f * blendFactor;

	return { up, down };
}

shade::SharedPointer<shade::animation::AnimationController> shade::animation::AnimationController::Create()
{
	return SharedPointer<shade::animation::AnimationController>::Create();
}

shade::animation::Pose* shade::animation::AnimationController::ProcessPose(const Asset<Skeleton>& skeleton, AnimationControlData& animationData, const FrameTimer& deltaTime, float timeMultiplier)
{
	return CalculatePose(ReceiveAnimationPose(skeleton, animationData.Animation), animationData, deltaTime, timeMultiplier);
}

shade::animation::Pose* shade::animation::AnimationController::Blend(const Asset<Skeleton>& skeleton, const animation::Pose* first, const animation::Pose* second, float blendFactor, const animation::BoneMask& boneMask)
{
	auto targetPose = ReceiveAnimationPose(skeleton, first->GetAnimationHash(), second->GetAnimationHash());
	CalculateBoneTransformsBlend(targetPose->GetSkeleton()->GetRootNode(), targetPose, first, second, glm::identity<glm::mat4>(), blendFactor, boneMask);
	return targetPose;
}

shade::animation::Pose* shade::animation::AnimationController::CreatePose(const Asset<Skeleton>& skeleton, std::size_t hash)
{
	if (m_Poses.find(hash) == m_Poses.end()) m_Poses.emplace(hash, animation::Pose(skeleton, hash));

	return &m_Poses.at(hash);
}

shade::animation::Pose* shade::animation::AnimationController::CalculatePose(animation::Pose* targetPose, AnimationControlData& animationData, const FrameTimer& deltaTime, float timeMultiplier)
{	
	animationData.CurrentPlayTime += animationData.TicksPerSecond * deltaTime.GetInSeconds<float>() * timeMultiplier;
	animationData.CurrentPlayTime  = glm::fmod(animationData.CurrentPlayTime, animationData.End);

	animationData.CurrentPlayTime  = glm::clamp(animationData.CurrentPlayTime, animationData.Start, animationData.End);

	targetPose->SetCurrentPlayTime(animationData.CurrentPlayTime);
	targetPose->SetDuration(animationData.Duration);

	CalculateBoneTransforms(targetPose, animationData, targetPose->GetSkeleton()->GetRootNode(), glm::mat4(1.0), targetPose->GetSkeleton()->GetArmature());

	return targetPose;
}

shade::animation::Pose* shade::animation::AnimationController::ReceiveAnimationPose(const Asset<Skeleton>& skeleton, std::size_t hash)
{
	return CreatePose(skeleton, hash);
}

shade::animation::Pose::Pose(const Asset<Skeleton>& skeleton, std::size_t animationHash, Type initial) :
	m_Skeleton(skeleton),
	m_AnimationCombinationHash(animationHash),
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
	m_AnimationCombinationHash(0),
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