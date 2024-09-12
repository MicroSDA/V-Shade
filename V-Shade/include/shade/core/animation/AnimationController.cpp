#include "shade_pch.h"
#include "AnimationController.h"
#include <shade/core/asset/AssetManager.h>
#include <glm/glm/gtx/common.hpp>

shade::animation::Pose* shade::animation::AnimationController::ReceiveAnimationPose(const Asset<Skeleton>& skeleton, std::size_t hash)
{
	return CreatePose(skeleton, hash);
}

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
		glm::mat4 interpolatedTransform = translation * rotation * scale;

		localMatrix *= interpolatedTransform;
		globalMatrix *= interpolatedTransform;

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

	glm::mat4 combined = glm::mat4_cast(glm::slerp(glm::quat_cast(firstPose), glm::quat_cast(secondPose), blendFactorWithBoneMask));
	
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
	return (skeleton) ? CalculatePose(ReceiveAnimationPose(skeleton, animationData.Animation), animationData, deltaTime, timeMultiplier) : nullptr;
}

shade::animation::Pose* shade::animation::AnimationController::Blend(const Asset<Skeleton>& skeleton, const animation::Pose* first, const animation::Pose* second, float blendFactor, const animation::BoneMask& boneMask)
{
	auto targetPose = ReceiveAnimationPose(skeleton, first->GetAnimationHash(), second->GetAnimationHash());
	CalculateBoneTransformsBlend(targetPose->GetSkeleton()->GetRootNode(), targetPose, first, second, glm::identity<glm::mat4>(), blendFactor, boneMask);
	return targetPose;
}

shade::animation::Pose* shade::animation::AnimationController::BlendTriangular(const Asset<Skeleton>& skeleton, const animation::Pose* aPose, const animation::Pose* bPose, const animation::Pose* cPose, float aBlend, float bBlend, float cBlend, const animation::BoneMask& boneMask)
{
	Pose* AB =	ReceiveAnimationPose(skeleton, aPose->GetAnimationHash(), bPose->GetAnimationHash());
	CalculateBoneTransformsBlend(skeleton->GetRootNode(), AB, aPose, bPose, glm::identity<glm::mat4>(), aBlend, boneMask);

	Pose* BC = ReceiveAnimationPose(skeleton, bPose->GetAnimationHash(), cPose->GetAnimationHash());
	CalculateBoneTransformsBlend(skeleton->GetRootNode(), BC, bPose, cPose, glm::identity<glm::mat4>(), bBlend, boneMask);

	Pose* AB_BC = ReceiveAnimationPose(skeleton, AB->GetAnimationHash(), BC->GetAnimationHash());
	CalculateBoneTransformsBlend(skeleton->GetRootNode(), AB_BC, AB, BC, glm::identity<glm::mat4>(), cBlend, boneMask);

	return AB_BC;
}

shade::animation::Pose* shade::animation::AnimationController::CreatePose(const Asset<Skeleton>& skeleton, std::size_t hash)
{
	if (m_Poses.find(hash) == m_Poses.end()) m_Poses.emplace(std::piecewise_construct, std::forward_as_tuple(hash), std::forward_as_tuple(skeleton, hash));

	return &m_Poses.at(hash);
}

shade::animation::Pose* shade::animation::AnimationController::CalculatePose(animation::Pose* targetPose, AnimationControlData& animationData, const FrameTimer& deltaTime, float timeMultiplier)
{
#if 1
	switch (animationData.State)
	{
	case Animation::State::Play:
		if (animationData.CurrentPlayTime == animationData.End && !animationData.IsLoop)
		{
			animationData.CurrentPlayTime = animationData.Start;
		}

		animationData.CurrentPlayTime += animationData.TicksPerSecond * deltaTime.GetInSeconds<float>() * timeMultiplier;

		if (!animationData.IsLoop && animationData.CurrentPlayTime > animationData.End)
		{
			animationData.State = Animation::State::Pause;
			animationData.CurrentPlayTime = animationData.End;
		}
		else
		{
			if (animationData.TicksPerSecond)
			{
				animationData.CurrentPlayTime = glm::fmod(animationData.CurrentPlayTime, animationData.End);
				animationData.CurrentPlayTime = glm::clamp(animationData.CurrentPlayTime, animationData.Start, animationData.End);
			}
		}
		break;

	case Animation::State::Stop:
		animationData.CurrentPlayTime = 0.f;
		break;

	case Animation::State::Pause:
		// Do nothing
		break;
	}
#else

	const float epsilon = 1e-5f; // Small value for float comparison

	switch (animationData.State)
	{
	case Animation::State::Play:
		// Check if CurrentPlayTime is approximately equal to End
		if (fabs(animationData.CurrentPlayTime - animationData.End) < epsilon && !animationData.IsLoop)
		{
			animationData.CurrentPlayTime = animationData.Start;
		}

		animationData.CurrentPlayTime += animationData.TicksPerSecond * deltaTime.GetInSeconds<float>() * timeMultiplier;

		if (!animationData.IsLoop && animationData.CurrentPlayTime > animationData.End)
		{
			animationData.State = Animation::State::Pause;
			animationData.CurrentPlayTime = animationData.End;
		}
		else
		{
			animationData.CurrentPlayTime = glm::fmod(animationData.CurrentPlayTime, animationData.End);
			animationData.CurrentPlayTime = glm::clamp(animationData.CurrentPlayTime, animationData.Start, animationData.End);
		}
		break;

	case Animation::State::Stop:
		animationData.CurrentPlayTime = animationData.Start;
		break;

	case Animation::State::Pause:
		// Do nothing
		break;
	}

#endif // 0

	targetPose->SetDuration(animationData.Duration); targetPose->SetCurrentPlayTime(animationData.CurrentPlayTime);
	CalculateBoneTransforms(targetPose, animationData, targetPose->GetSkeleton()->GetRootNode(), glm::identity<glm::mat4>(), targetPose->GetSkeleton()->GetArmature());
	targetPose->UpdatePreviousRootTransform();
	return targetPose;
}

void shade::animation::AnimationController::AnimationControlData::Serialize(std::ostream& stream) const
{
	serialize::Serializer::Serialize(stream, (Animation && Animation->GetAssetData()) ? Animation->GetAssetData()->GetId() : "");
	serialize::Serializer::Serialize(stream, std::uint8_t(State));
	serialize::Serializer::Serialize(stream, Start);
	serialize::Serializer::Serialize(stream, End);
	serialize::Serializer::Serialize(stream, Duration);
	serialize::Serializer::Serialize(stream, CurrentPlayTime);
	serialize::Serializer::Serialize(stream, TicksPerSecond);
	serialize::Serializer::Serialize(stream, IsLoop);
}

void shade::animation::AnimationController::AnimationControlData::Deserialize(std::istream& stream)
{
	std::string assetId; serialize::Serializer::Deserialize(stream, assetId);
	serialize::Serializer::Deserialize(stream, static_cast<Animation::State&>(State));
	serialize::Serializer::Deserialize(stream, Start);
	serialize::Serializer::Deserialize(stream, End);
	serialize::Serializer::Deserialize(stream, Duration);
	serialize::Serializer::Deserialize(stream, CurrentPlayTime);
	serialize::Serializer::Deserialize(stream, TicksPerSecond);
	serialize::Serializer::Deserialize(stream, IsLoop);

	shade::AssetManager::GetAsset<shade::Animation,
		shade::BaseAsset::InstantiationBehaviour::Synchronous>(assetId,
			shade::AssetMeta::Category::Secondary,
			shade::BaseAsset::LifeTime::KeepAlive,
			[&](auto& animation) mutable
			{
				Animation = animation;
			});
}
