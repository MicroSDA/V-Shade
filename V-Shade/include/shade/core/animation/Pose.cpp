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

void shade::animation::Pose::RootMotion::Initialize(const Asset<Skeleton>& skeleton, const Asset<Animation>& aniamtion, float start, float end)
{
	if (const Animation::Channel* channel = aniamtion->GetAnimationCahnnel(skeleton->GetArmature()->Name))
	{
		//// Возможно нужно будет использовать парент мультипликацию 

		Translation.Start = aniamtion->InterpolatePosition(*channel, start);
		Translation.End = aniamtion->InterpolatePosition(*channel, end);

		Translation.Current = Translation.Start;
		Translation.Delta	= glm::vec3(0.f);

		Rotation.Start = aniamtion->InterpolateRotation(*channel, start);
		Rotation.End = aniamtion->InterpolateRotation(*channel, end);

		// Начинать анимацию не со страта а со нуля !!!!!, возможно только для ротейшена, попробуй !!

	/*	Rotation.Current	= glm::identity<glm::quat>();
		Rotation.Delta		= glm::identity<glm::quat>();*/

		Rotation.Current	= Rotation.Start;
		Rotation.Delta		= glm::identity<glm::quat>();

	}
	else
	{
		// We couldnt find bone 
	}
}
