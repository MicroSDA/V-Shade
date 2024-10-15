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
			void BasePoseBlendRecursevly(Pose* p0, const Pose* p1, const Pose* p2, const Skeleton::BoneNode* bone, const glm::mat4& parrentTransform, float blendFactor, const BoneMask& boneMask, std::uint32_t parentId)
			{
				const float blendFactorWithBoneMask = blendFactor * boneMask.GetWeight(bone->ID);

				Pose::LocalTransform& localP0 = p0->GetBoneLocalTransform(bone->ID);

				const Pose::LocalTransform& localP1 = p1->GetBoneLocalTransform(bone->ID);
				const Pose::LocalTransform& localP2 = p2->GetBoneLocalTransform(bone->ID);

				localP0.Translation = BlendF::Translation(localP1.Translation, localP2.Translation, blendFactorWithBoneMask);
				localP0.Rotation	= BlendF::Rotation(localP1.Rotation, localP2.Rotation, blendFactorWithBoneMask);
				localP0.Scale		= BlendF::Scale(localP1.Scale, localP2.Scale, blendFactorWithBoneMask);

				const glm::mat4 globalMatrix = parrentTransform * glm::translate(glm::identity<glm::mat4>(), localP0.Translation) * glm::toMat4(localP0.Rotation) * glm::scale(glm::identity<glm::mat4>(), localP0.Scale);

				p0->GetBoneGlobalTransform(bone->ID) = { globalMatrix , parentId };

				for (const auto& child : bone->Children)
					BasePoseBlendRecursevly<BlendF>(p0, p1, p2, child, globalMatrix, blendFactor, boneMask, bone->ID);
			}
			template<typename BlendF>
			void BasePoseBlend(Pose* p0, const Pose* p1, const Pose* p2, const Asset<Skeleton>& skeleton, float blendFactor, const BoneMask& boneMask)
			{
				p0->MarkHasInverseBindPose(false);

				const float blendFactorWithBoneMask = blendFactor; // TODO : Armature blend weight

				Pose::LocalTransform& localP0		= p0->GetArmatureLocalTransform();

				const Pose::LocalTransform& localP1 = p1->GetArmatureLocalTransform();
				const Pose::LocalTransform& localP2 = p2->GetArmatureLocalTransform();

				localP0.Translation = BlendF::Translation(localP1.Translation, localP2.Translation, blendFactorWithBoneMask);
				localP0.Rotation = BlendF::Rotation(localP1.Rotation, localP2.Rotation, blendFactorWithBoneMask);
				localP0.Scale = BlendF::Scale(localP1.Scale, localP2.Scale, blendFactorWithBoneMask);

				BasePoseBlendRecursevly<BlendF>(p0, p1, p2, skeleton->GetRootNode(), glm::translate(glm::identity<glm::mat4>(), localP0.Translation) * glm::toMat4(localP0.Rotation) * glm::scale(glm::identity<glm::mat4>(), localP0.Scale), 
					blendFactorWithBoneMask, boneMask, ~0);
			}

			template<typename BlendF>
			void RootMotionBlend(Pose::RootMotion* r0, const Pose::RootMotion* r1, const Pose::RootMotion* r2, float weight)
			{
				// Translation TODO
				// When you blend root motion translation, you will lose velocity if you just do a simple lerp.
				// lerp + then scale the lerped result so that it's magnitude is the velocity you want
				// (this will only have a small effect compared to getting the foot phase sync sorted out)

				/*r0->Translation.Delta = BlendF::Translation(r1->Translation.Delta, r2->Translation.Delta, weight);
				r0->Translation.Current = BlendF::Translation(r1->Translation.Current, r2->Translation.Current, weight);
				r0->Translation.Difference = BlendF::Translation(r1->Translation.Difference, r2->Translation.Difference, weight);*/

				r0->Translation.Delta		= BlendF::Translation(r1->Translation.Delta, r2->Translation.Delta, weight);
				r0->Translation.Current		= BlendF::Translation(r1->Translation.Current, r2->Translation.Current, weight);
				r0->Translation.Difference	= BlendF::Translation(r1->Translation.Difference, r2->Translation.Difference, weight);

				r0->Rotation.Delta			= r0->Rotation.Current;
				r0->Rotation.Current		= BlendF::Rotation(r1->Rotation.Current, r2->Rotation.Current, weight);

				r0->Rotation.Difference		= glm::normalize(glm::conjugate(r0->Rotation.Delta) * r0->Rotation.Current);

				/*r0->Scale.Current			= BlendF::Scale(r1->Scale.Current, r2->Scale.Current, weight);
				r0->Scale.Delta				= BlendF::Scale(r1->Scale.Delta, r2->Scale.Delta, weight);*/
			}


			void ComputePoseRecursevly(animation::Pose* pose, const AnimationController::AnimationControlData& animationData, const Skeleton::BoneNode* bone, const glm::mat4& parentTransform, std::uint32_t parentId)
			{
				glm::mat4 globalMatrix = parentTransform;

				if (const Animation::Channel* channel = animationData.Animation->GetAnimationCahnnel(bone->Name))
				{
					Pose::LocalTransform& local = pose->GetBoneLocalTransform(bone->ID);
					
					local.Translation = animationData.Animation->InterpolatePosition(*channel, animationData.CurrentPlayTime);
					local.Rotation = animationData.Animation->InterpolateRotation(*channel, animationData.CurrentPlayTime);
					local.Scale = animationData.Animation->InterpolateScale(*channel, animationData.CurrentPlayTime);

					globalMatrix *= glm::translate(glm::identity<glm::mat4>(), local.Translation) * glm::toMat4(local.Rotation) * glm::scale(glm::identity<glm::mat4>(), local.Scale);

					pose->GetBoneGlobalTransform(bone->ID) = { globalMatrix, parentId };
				}
				else
				{
					globalMatrix *= glm::translate(glm::identity<glm::mat4>(), bone->Translation) * glm::toMat4(bone->Rotation) * glm::scale(glm::identity<glm::mat4>(), bone->Scale);
					
					pose->GetBoneGlobalTransform(bone->ID) = { globalMatrix, parentId };
				}

				for (const auto& child : bone->Children)
					ComputePoseRecursevly(pose, animationData, child, globalMatrix, bone->ID);
			}

			void ComputePose(animation::Pose* pose, const AnimationController::AnimationControlData& animationData, const Asset<Skeleton>& skeleton)
			{
				glm::mat4 armatureMatrix = glm::identity<glm::mat4>();

				pose->MarkHasInverseBindPose(false);

				Pose::LocalTransform& local = pose->GetArmatureLocalTransform();

				if (const Animation::Channel* channel = animationData.Animation->GetAnimationCahnnel(skeleton->GetArmature()->Name))
				{
					local.Translation = animationData.Animation->InterpolatePosition(*channel, animationData.CurrentPlayTime);
					local.Rotation = animationData.Animation->InterpolateRotation(*channel, animationData.CurrentPlayTime);
					local.Scale = animationData.Animation->InterpolateScale(*channel, animationData.CurrentPlayTime);

					armatureMatrix = glm::translate(glm::identity<glm::mat4>(), local.Translation) * glm::toMat4(local.Rotation) * glm::scale(glm::identity<glm::mat4>(), local.Scale);
				}
				else
				{
					local.Translation	= skeleton->GetArmature()->Translation;
					local.Rotation		= skeleton->GetArmature()->Rotation;
					local.Scale			= skeleton->GetArmature()->Scale;

					armatureMatrix = glm::translate(glm::identity<glm::mat4>(), local.Translation) * glm::toMat4(local.Rotation) * glm::scale(glm::identity<glm::mat4>(), local.Scale);
				}

				ComputePoseRecursevly(pose, animationData, skeleton->GetRootNode(), armatureMatrix, ~0);
			}
		}
	}
}

shade::animation::Pose* shade::animation::AnimationController::ReceiveAnimationPose(const Asset<Skeleton>& skeleton, Pose::Type type, std::size_t hash)
{
	return CreatePose(skeleton, type, hash);
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
		(pose->GetType() == Pose::Type::ZeroPose || animationData.RootMotion != &pose->GetRootMotion()) ? pose->GetRootMotion().Initialize(skeleton, animationData.Animation, animationData.Start, animationData.End) : void();

		animationData.RootMotion = &pose->GetRootMotion();
		pose->SetRootMotion(); // MBY rename to mark as use root motion or set use root motion
	}
	else
	{
		animationData.RootMotion = nullptr;
		pose->UnsetRootMotion();
	}

	return (skeleton) ? CalculatePose(pose, animationData, deltaTime, timeMultiplier) : nullptr;
}

shade::animation::Pose* shade::animation::AnimationController::Blend(const Asset<Skeleton>& skeleton, const animation::Pose* p1, const animation::Pose* p2, float blendFactor, const animation::BoneMask& boneMask)
{
	Pose* p0 = ReceiveAnimationPose(skeleton, Pose::Type::Pose, p1->GetAnimationHash(), p2->GetAnimationHash());

	if (p1->GetType() == Pose::Type::AdditivePose)
	{
		
	}
	else if (p2->GetType() == Pose::Type::AdditivePose)
	{
		
	}
	else
	{
		utils::BasePoseBlend<utils::BaseBlend>(p0, p1, p2, skeleton, blendFactor, boneMask);
	}

	if (p1->HasRootMotion() && p2->HasRootMotion())
	{
		utils::RootMotionBlend<utils::BaseBlend>(&p0->GetRootMotion(), &p1->GetRootMotion(), &p2->GetRootMotion(), blendFactor);
		p0->SetRootMotion();
	}

	return p0;
}

shade::animation::Pose* shade::animation::AnimationController::BlendTriangular(const Asset<Skeleton>& skeleton, const animation::Pose* aPose, const animation::Pose* bPose, const animation::Pose* cPose, float aBlend, float bBlend, float cBlend, const animation::BoneMask& boneMask)
{
	Pose* AB	= ReceiveAnimationPose(skeleton, Pose::Type::Pose, aPose->GetAnimationHash(), bPose->GetAnimationHash());
	Pose* BC	= ReceiveAnimationPose(skeleton, Pose::Type::Pose, bPose->GetAnimationHash(), cPose->GetAnimationHash());
	Pose* AB_BC = ReceiveAnimationPose(skeleton, Pose::Type::Pose, AB->GetAnimationHash(), BC->GetAnimationHash());

	utils::BasePoseBlend<utils::BaseBlend>(AB, aPose, bPose, skeleton, aBlend, boneMask);
	utils::BasePoseBlend<utils::BaseBlend>(BC, bPose, cPose, skeleton, bBlend, boneMask);
	utils::BasePoseBlend<utils::BaseBlend>(AB_BC, AB, BC, skeleton, cBlend, boneMask);

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
	if (m_Poses.find(hash) == m_Poses.end()) m_Poses.emplace(std::piecewise_construct, std::forward_as_tuple(hash), std::forward_as_tuple(skeleton, hash));
	return &m_Poses.at(hash);
}

shade::animation::Pose* shade::animation::AnimationController::CalculatePose(animation::Pose* targetPose, AnimationControlData& animationData, const FrameTimer& deltaTime, float timeMultiplier)
{
	bool newFrame = false;

#if 1
	switch (animationData.State)
	{
	case Animation::State::Play:
		if (animationData.CurrentPlayTime >= animationData.End && !animationData.IsLoop)
		{
			animationData.CurrentPlayTime = animationData.Start;
		}

		animationData.CurrentPlayTime += animationData.TicksPerSecond * deltaTime.GetInSeconds<float>() * timeMultiplier;
		//animationData.CurrentPlayTime += animationData.TicksPerSecond / 1000.f * deltaTime.GetInMilliseconds<float>() * timeMultiplier;

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


	utils::ComputePose(targetPose, animationData, targetPose->GetSkeleton());

	targetPose->SetDuration(animationData.Duration); targetPose->SetCurrentPlayTime(animationData.CurrentPlayTime);

	(animationData.HasRootMotion) ? animationData.UpdateRootMotion(targetPose, newFrame) : void();

	targetPose->SetType(Pose::Type::Pose); return targetPose;
}

void shade::animation::AnimationController::AnimationControlData::UpdateRootMotion(Pose* pose, bool newFrame)
{
	// Ќачинать анимацию не со страта а со нул€ !!!!!, пр присваевании ее

	const Pose::LocalTransform& local = pose->GetArmatureLocalTransform(); // ƒа, нужно добавить арматуру в позу !!

	if (State == Animation::State::Play || State == Animation::State::Pause) 
	{

		RootMotion->Translation.Delta	= RootMotion->Translation.Current;
		RootMotion->Rotation.Delta		= RootMotion->Rotation.Current;

		RootMotion->Translation.Current = local.Translation;
		RootMotion->Rotation.Current	= local.Rotation;

		//std::swap(RootMotion->Rotation.Current.y, RootMotion->Rotation.Current.x);

		if (newFrame)
		{
			RootMotion->Translation.Difference	= RootMotion->Translation.Current - (RootMotion->Translation.Delta - RootMotion->Translation.End + RootMotion->Translation.Start); // Not Sure if we need to + start
			//RootMotion->Rotation.Difference		= glm::normalize(glm::conjugate(RootMotion->Rotation.Delta) * RootMotion->Rotation.Current * glm::conjugate(RootMotion->Rotation.End) * RootMotion->Rotation.Start);
			RootMotion->Rotation.Difference		= glm::normalize(glm::conjugate(RootMotion->Rotation.Delta) * RootMotion->Rotation.Current * glm::conjugate(RootMotion->Rotation.End) * RootMotion->Rotation.Start);


			// Another wariant
			//RootMotion->Rotation.Difference		= glm::conjugate(RootMotion->Rotation.Delta) * (RootMotion->Rotation.End * glm::conjugate(RootMotion->Rotation.Current) * RootMotion->Rotation.Start);
		}
		else
		{
			RootMotion->Translation.Difference = RootMotion->Translation.Current - RootMotion->Translation.Delta;
			RootMotion->Rotation.Difference = glm::normalize(glm::conjugate(RootMotion->Rotation.Delta) * RootMotion->Rotation.Current);
		}


		
		//// Try to keep delta difference
		//// and multiply current diff by curmagnitude / delta magnitude
		//// Translation.Current - Translation.Delta * (glm::length(DeltaDifference) / glm::length(Translation.Current - Translation.Delta));
		//RootMotion->Translation.Difference	= RootMotion->Translation.Current - RootMotion->Translation.Delta;
		//RootMotion->Rotation.Difference		= glm::conjugate(RootMotion->Rotation.Delta) * RootMotion->Rotation.Current;

	}
	else if (State == Animation::State::Stop)
	{
		RootMotion->Translation.Current		= RootMotion->Translation.Start; // Should be zero if we will use normal root bone, not pelvis
		RootMotion->Translation.Delta		= glm::vec3(0.f);

		RootMotion->Rotation.Current		= RootMotion->Rotation.Start;
		RootMotion->Rotation.Delta			= glm::identity<glm::quat>();

		RootMotion->Translation.Difference	= glm::vec3(0.f);
		RootMotion->Rotation.Difference		= glm::identity<glm::quat>();
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
