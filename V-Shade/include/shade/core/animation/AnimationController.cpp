#include "shade_pch.h"
#include "AnimationController.h"
#include <shade/core/asset/AssetManager.h>
#include <glm/glm/gtx/common.hpp>

namespace shade
{
	namespace animation
	{
		namespace utils
		{
			struct BaseBlend
			{
				static glm::vec3 Translation(const glm::vec3& t1, const glm::vec3& t2, float weight)
				{
					return glm::mix(t1, t2, weight);
				}

				static glm::quat Rotation(const glm::quat& r1, const glm::quat& r2, float weight)
				{
					return glm::slerp(r1, r2, weight);
				}

				static glm::vec3 Scale(const glm::vec3& s1, const glm::vec3& s2, float weight)
				{
					return Translation(s1, s2, weight);
				}
			};

			template<typename BlendF>
			void BasePoseBlend(const Skeleton::BoneNode* bone, Pose* p0, const Pose* p1, const Pose* p2, const glm::mat4& parrentTransform, float blendFactor, const BoneMask& boneMask)
			{
				const float blendFactorWithBoneMask = blendFactor * boneMask.GetWeight(bone->ID);

				Pose::LocalTransform& localP0 = p0->GetBoneLocalTransform(bone->ID);

				const Pose::LocalTransform& localP1 = p1->GetBoneLocalTransform(bone->ID);
				const Pose::LocalTransform& localP2 = p2->GetBoneLocalTransform(bone->ID);


				localP0.Translation = BlendF::Translation(localP1.Translation, localP2.Translation, blendFactorWithBoneMask);
				localP0.Rotation = BlendF::Rotation(localP1.Rotation, localP2.Rotation, blendFactorWithBoneMask);
				localP0.Scale = BlendF::Scale(localP1.Scale, localP2.Scale, blendFactorWithBoneMask);


				const glm::mat4 globalMatrix = parrentTransform * glm::translate(glm::identity<glm::mat4>(), localP0.Translation) * glm::toMat4(localP0.Rotation) * glm::scale(glm::identity<glm::mat4>(), localP0.Scale);

				p0->GetBoneGlobalTransform(bone->ID) = globalMatrix * bone->InverseBindPose * p0->GetSkeleton()->GetArmature().Transform;

				for (const auto& child : bone->Children)
					BasePoseBlend<BlendF>(child, p0, p1, p2, globalMatrix, blendFactor, boneMask);
			}

			template<typename BlendF>
			void RootMotionBlend(Pose::RootMotion* r0, const Pose::RootMotion* r1, const Pose::RootMotion* r2, float weight)
			{
				// Translation TODO
				// When you blend root motion translation, you will lose velocity if you just do a simple lerp.
				// lerp + then scale the lerped result so that it's magnitude is the velocity you want
				// (this will only have a small effect compared to getting the foot phase sync sorted out)

				r0->Translation.Delta		= BlendF::Translation(r1->Translation.Delta, r2->Translation.Delta, weight);
				r0->Translation.Current		= BlendF::Translation(r1->Translation.Current, r2->Translation.Current, weight);

				r0->Rotation.Delta			= r0->Rotation.Current;
				r0->Rotation.Current		= BlendF::Rotation(r1->Rotation.Current, r2->Rotation.Current, weight);

				/*r0->Scale.Current			= BlendF::Scale(r1->Scale.Current, r2->Scale.Current, weight);
				r0->Scale.Delta				= BlendF::Scale(r1->Scale.Delta, r2->Scale.Delta, weight);*/
			}
		}
	}
}

shade::animation::Pose* shade::animation::AnimationController::ReceiveAnimationPose(const Asset<Skeleton>& skeleton, Pose::Type type, std::size_t hash)
{
	return CreatePose(skeleton, type, hash);
}

void shade::animation::AnimationController::CalculateBoneTransforms(
	animation::Pose* pose,
	const  AnimationControlData& animationData,
	const Skeleton::BoneNode* bone,
	const glm::mat4& parentTransform,
	const Skeleton::BoneArmature& armature)
{
	glm::mat4 globalMatrix = parentTransform;

	if (const Animation::Channel* channel = animationData.Animation->GetAnimationCahnnel(bone->Name))
	{
		const Pose::LocalTransform local
		{
			.Translation = animationData.Animation->InterpolatePosition(*channel, animationData.CurrentPlayTime),
			.Rotation = animationData.Animation->InterpolateRotation(*channel, animationData.CurrentPlayTime),
			.Scale = animationData.Animation->InterpolateScale(*channel, animationData.CurrentPlayTime)
		};

		pose->GetBoneLocalTransform(bone->ID) = local;

		glm::mat4 interpolatedTransform = glm::translate(glm::identity<glm::mat4>(), local.Translation) * glm::toMat4(local.Rotation) * glm::scale(glm::identity<glm::mat4>(), local.Scale);

		globalMatrix *= interpolatedTransform;

		pose->GetBoneGlobalTransform(bone->ID) = globalMatrix * bone->InverseBindPose * armature.Transform;
	}
	else
	{
		globalMatrix *= parentTransform * glm::translate(glm::identity<glm::mat4>(), bone->Translation) * glm::toMat4(bone->Rotation) * glm::scale(glm::identity<glm::mat4>(), bone->Scale);
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
	glm::mat4 globalMatrix = parrentTransform;

	const Pose::LocalTransform& firstPose = first->GetBoneLocalTransform(bone->ID);
	const Pose::LocalTransform& secondPose = second->GetBoneLocalTransform(bone->ID);

	const float blendFactorWithBoneMask = blendFactor * boneMask.GetWeight(bone->ID);

	const Pose::LocalTransform combined
	{
		.Translation = utils::BaseBlend::Translation(firstPose.Translation, secondPose.Translation, blendFactorWithBoneMask),
		.Rotation = glm::slerp(firstPose.Rotation, secondPose.Rotation, blendFactorWithBoneMask),
		.Scale = glm::mix(firstPose.Scale, secondPose.Scale, blendFactorWithBoneMask)
	};

	globalMatrix *= glm::translate(glm::identity<glm::mat4>(), combined.Translation) * glm::toMat4(combined.Rotation) * glm::scale(glm::identity<glm::mat4>(), combined.Scale);

	targetPose->GetBoneLocalTransform(bone->ID) = combined;
	targetPose->GetBoneGlobalTransform(bone->ID) = globalMatrix * bone->InverseBindPose * targetPose->GetSkeleton()->GetArmature().Transform;

	for (const auto& child : bone->Children)
		CalculateBoneTransformsBlend(child, targetPose, first, second, globalMatrix, blendFactor, boneMask);
}

void shade::animation::AnimationController::CalculateBoneTransformsAdditive(const shade::Skeleton::BoneNode* bone, animation::Pose* targetPose, const animation::Pose* referencePose, const animation::Pose* additivePose, const glm::mat4& parrentTransform, float blendFactor, const animation::BoneMask& boneMask)
{
	// DOESN'T WORK
	//const glm::mat4& rMat	= referencePose->GetBoneLocalTransform(bone->ID);
	//const glm::mat4& aMat	= additivePose->GetBoneLocalTransform(bone->ID);

	//glm::quat baseRotation, additiveRotation, combinedRotation;
	//glm::vec3 baseTranslation, additiveTranslation, combinedTranslation;
	//glm::vec3 baseScale, additiveScale, combinedScale;

	//math::DecomposeMatrix(rMat, baseTranslation, baseRotation, baseScale);
	//math::DecomposeMatrix(aMat, additiveTranslation, additiveRotation, additiveScale);

	//combinedTranslation = baseTranslation + blendFactor * additiveTranslation;
	//combinedRotation = baseRotation * glm::slerp(glm::identity<glm::quat>(), additiveRotation, blendFactor) ;
	//
	//combinedScale = baseScale + blendFactor * (additiveScale - glm::one<glm::vec3>());

	//
	////glm::mat4 combinedLocal = aMat;
	//glm::mat4 combinedLocal = glm::translate(glm::mat4(1.f), combinedTranslation) * glm::toMat4(combinedRotation) * glm::scale(glm::mat4(1.f), combinedScale);

	//glm::mat4 globalMatrix = parrentTransform * combinedLocal;

	//targetPose->GetBoneLocalTransform(bone->ID) = combinedLocal;
	//targetPose->GetBoneGlobalTransform(bone->ID) = globalMatrix * bone->InverseBindPose * targetPose->GetSkeleton()->GetArmature().Transform;

	//for (const auto& child : bone->Children)
	//	CalculateBoneTransformsAdditive(child, targetPose, referencePose, additivePose, globalMatrix, blendFactor, boneMask);
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
	Pose* pose = ReceiveAnimationPose(skeleton, Pose::Type::Pose, animationData.Animation);

	if (animationData.HasRootMotion)
	{
		animationData.RootMotion.FinalizeRootMotion(skeleton, animationData.Animation, animationData.Start, animationData.End);
		pose->SetRootMotion(&animationData.RootMotion);

	}
	else
	{
		pose->SetRootMotion(nullptr);
	}

	return (skeleton) ? CalculatePose(pose, animationData, deltaTime, timeMultiplier) : nullptr;
}

shade::animation::Pose* shade::animation::AnimationController::Blend(const Asset<Skeleton>& skeleton, const animation::Pose* first, const animation::Pose* second, float blendFactor, const animation::BoneMask& boneMask)
{
	auto targetPose = ReceiveAnimationPose(skeleton, Pose::Type::Pose, first->GetAnimationHash(), second->GetAnimationHash());

	if (first->GetType() == Pose::Type::AdditivePose)
	{
		CalculateBoneTransformsAdditive(skeleton->GetRootNode(), targetPose, second, first, glm::identity<glm::mat4>(), blendFactor, boneMask);
	}
	else if (second->GetType() == Pose::Type::AdditivePose)
	{
		CalculateBoneTransformsAdditive(skeleton->GetRootNode(), targetPose, first, second, glm::identity<glm::mat4>(), blendFactor, boneMask);
	}
	else
	{
		utils::BasePoseBlend<utils::BaseBlend>(targetPose->GetSkeleton()->GetRootNode(), targetPose, first, second, glm::identity<glm::mat4>(), blendFactor, boneMask);
	}

	static Pose::RootMotion motion;

	utils::RootMotionBlend<utils::BaseBlend>(&motion, first->GetRootMotion(), second->GetRootMotion(), blendFactor);

	targetPose->SetRootMotion(&motion);

	return targetPose;
}

shade::animation::Pose* shade::animation::AnimationController::BlendTriangular(const Asset<Skeleton>& skeleton, const animation::Pose* aPose, const animation::Pose* bPose, const animation::Pose* cPose, float aBlend, float bBlend, float cBlend, const animation::BoneMask& boneMask)
{
	Pose* AB = ReceiveAnimationPose(skeleton, Pose::Type::Pose, aPose->GetAnimationHash(), bPose->GetAnimationHash());
	CalculateBoneTransformsBlend(skeleton->GetRootNode(), AB, aPose, bPose, glm::identity<glm::mat4>(), aBlend, boneMask);

	Pose* BC = ReceiveAnimationPose(skeleton, Pose::Type::Pose, bPose->GetAnimationHash(), cPose->GetAnimationHash());
	CalculateBoneTransformsBlend(skeleton->GetRootNode(), BC, bPose, cPose, glm::identity<glm::mat4>(), bBlend, boneMask);

	Pose* AB_BC = ReceiveAnimationPose(skeleton, Pose::Type::Pose, AB->GetAnimationHash(), BC->GetAnimationHash());
	CalculateBoneTransformsBlend(skeleton->GetRootNode(), AB_BC, AB, BC, glm::identity<glm::mat4>(), cBlend, boneMask);

	return AB_BC;
}

shade::animation::Pose* shade::animation::AnimationController::GetZeroPose(const Asset<Skeleton>& skeleton)
{
	return ReceiveAnimationPose(skeleton, Pose::Type::ZeroPose, static_cast<std::size_t>(Pose::Type::ZeroPose));
}

shade::animation::Pose* shade::animation::AnimationController::GenerateAdditivePose(const Asset<Skeleton>& skeleton, const animation::Pose* referencePose, const animation::Pose* basePose)
{
	Pose* additivePose = ReceiveAnimationPose(skeleton, Pose::Type::AdditivePose, referencePose->GetAnimationHash(), basePose->GetAnimationHash());

	for (std::size_t i = 0; i < additivePose->GetBoneLocalTransforms()->size(); ++i)
	{
		/*const glm::mat4& rMat = referencePose->GetBoneLocalTransform(i);
		const glm::mat4& bMat = basePose->GetBoneLocalTransform(i);
		additivePose->GetBoneLocalTransform(i) = rMat * glm::inverse(bMat);*/
	}

	return additivePose;
}

shade::animation::Pose* shade::animation::AnimationController::CreatePose(const Asset<Skeleton>& skeleton, Pose::Type type, std::size_t hash)
{
	if (m_Poses.find(hash) == m_Poses.end()) m_Poses.emplace(std::piecewise_construct, std::forward_as_tuple(hash), std::forward_as_tuple(skeleton, hash, type));
	return &m_Poses.at(hash);
}

shade::animation::Pose* shade::animation::AnimationController::CalculatePose(animation::Pose* targetPose, AnimationControlData& animationData, const FrameTimer& deltaTime, float timeMultiplier)
{
	bool newFrame = false;

#if 1
	switch (animationData.State)
	{
	case Animation::State::Play:
		if (animationData.CurrentPlayTime == animationData.End && !animationData.IsLoop)
		{
			animationData.CurrentPlayTime = animationData.Start;
		}

		animationData.CurrentPlayTime += animationData.TicksPerSecond * deltaTime.GetInSeconds<float>() * timeMultiplier;

		// TODO: Add reverse animation plying !!
		newFrame = animationData.CurrentPlayTime >= animationData.End;

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

	CalculateBoneTransforms(targetPose, animationData, targetPose->GetSkeleton()->GetRootNode(), glm::identity<glm::mat4>(), targetPose->GetSkeleton()->GetArmature());

	targetPose->SetDuration(animationData.Duration); targetPose->SetCurrentPlayTime(animationData.CurrentPlayTime);

	if (animationData.HasRootMotion) animationData.UpdateRootMotion(targetPose, newFrame);


	//if(animationData.IsUseRootMotion && animationData.State == Animation::State::Play || animationData.State == Animation::State::Pause) targetPose->UpdateRootMotion(isNextFrameNewLoop);

	return targetPose;
}

void shade::animation::AnimationController::AnimationControlData::UpdateRootMotion(Pose* pose, bool newFrame)
{
	const Pose::LocalTransform& local = pose->GetBoneLocalTransform(RootMotion.RootBone);

	if (State == Animation::State::Play)
	{
		if (newFrame)
		{
			//RootMotion.Translation.Delta = RootMotion.Translation.Start;
			RootMotion.Translation.Delta	= (RootMotion.Translation.Current - RootMotion.Translation.End + RootMotion.Translation.Start);
			RootMotion.Rotation.Delta		= glm::conjugate(RootMotion.Rotation.End) * RootMotion.Rotation.Current * glm::conjugate(RootMotion.Rotation.Start);

			
			//RootMotion.Scale.Delta = RootMotion.Scale.Start;
		}
		else
		{
			RootMotion.Translation.Delta = RootMotion.Translation.Current;
			RootMotion.Rotation.Delta = RootMotion.Rotation.Current;
			RootMotion.Scale.Delta = RootMotion.Scale.Current;
		}

		RootMotion.Translation.Current = local.Translation;
		RootMotion.Rotation.Current = local.Rotation;
		RootMotion.Scale.Current = local.Scale;

	}
	else if (State == Animation::State::Stop)
	{
		RootMotion.Reset();
	}
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
