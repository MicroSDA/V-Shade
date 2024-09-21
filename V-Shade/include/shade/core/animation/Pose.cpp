#include "shade_pch.h"
#include "Pose.h"
#include <shade/core/render/RenderAPI.h>
#include <glm/glm/gtx/common.hpp>

shade::animation::Pose::Pose(const Asset<Skeleton>& skeleton, std::size_t animationHash, Type type) :
	m_Skeleton(skeleton),
	m_AnimationCombinationHash(animationHash),
	m_GlobalTransforms(SharedPointer<std::vector<glm::mat4>>::Create()),
	m_LocalTransforms(SharedPointer<std::vector<LocalTransform>>::Create()),
	m_Type(type)
{
	Reset();
}

void shade::animation::Pose::Reset()
{
	//m_GlobalTransforms->clear(); // mby dont need to clear, resize will do this ??
	m_GlobalTransforms->resize(RenderAPI::MAX_BONES_PER_INSTANCE, glm::identity<glm::mat4>());

	//m_LocalTransforms->clear();
	m_LocalTransforms->resize(RenderAPI::MAX_BONES_PER_INSTANCE);
}

glm::vec3 shade::animation::Pose::RootMotion::GetTranlsationDifference() const
{
	// Try to keep delta difference
	// and multiply current diff by curmagnitude / delta magnitude
	// return = Translation.Current - Translation.Delta * (glm::length(DeltaDifference) / glm::length(Translation.Current - Translation.Delta));
	return Translation.Current - Translation.Delta;
}

void shade::animation::Pose::RootMotion::FinalizeRootMotion(const Asset<Skeleton>& skeleton, const Asset<Animation>& aniamtion, float start, float end)
{
	if (const Animation::Channel* channel = aniamtion->GetAnimationCahnnel(skeleton->GetBone(RootBone)->Name))
	{
		//// Возможно нужно будет использовать парент мультипликацию 

		Translation.Start = aniamtion->InterpolatePosition(*channel, start);
		Translation.End = aniamtion->InterpolatePosition(*channel, end);

		Rotation.Start = aniamtion->InterpolateRotation(*channel, start);
		Rotation.End = aniamtion->InterpolateRotation(*channel, end);

		Scale.Start = aniamtion->InterpolateScale(*channel, start);
		Scale.End = aniamtion->InterpolateScale(*channel, end);
	}
	else
	{
		// We couldnt find bone 
	}
}
