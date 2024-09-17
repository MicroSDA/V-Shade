#include "shade_pch.h"
#include "Pose.h"
#include <shade/core/render/RenderAPI.h>


shade::animation::Pose::Pose(const Asset<Skeleton>& skeleton, std::size_t animationHash, Type type) :
	m_Skeleton(skeleton),
	m_AnimationCombinationHash(animationHash),
	m_GlobalTransforms(SharedPointer<std::vector<glm::mat4>>::Create()),
	m_LocalTransforms(SharedPointer<std::vector<glm::mat4>>::Create()),
	m_Type(type)
{
	Reset();
}

void shade::animation::Pose::Reset()
{
	//m_GlobalTransforms->clear(); // mby dont need to clear, resize will do this ??
	m_GlobalTransforms->resize(RenderAPI::MAX_BONES_PER_INSTANCE, glm::identity<glm::mat4>());

	//m_LocalTransforms->clear();
	m_LocalTransforms->resize(RenderAPI::MAX_BONES_PER_INSTANCE, glm::identity<glm::mat4>());
}

void shade::animation::Pose::UpdateRootMotion(float tickPerSecond)
{
	m_RootMotionTranslationDelta	= m_RootMotionTranslation;
	m_RootMotionRotationDelta		= m_RootMotionRotation;

	glm::vec3 s;
	math::DecomposeMatrix(m_LocalTransforms->at(0), m_RootMotionTranslation, m_RootMotionRotation, s);
}

glm::vec3 shade::animation::Pose::GetRootMotionTranslationDifference() const
{
	return m_RootMotionTranslation - m_RootMotionTranslationDelta;
}

glm::vec3 shade::animation::Pose::GetRootMotionTranslation() const
{
	return m_RootMotionTranslation; 
}

glm::quat shade::animation::Pose::GetRootMotionRotationDifference() const
{
	return glm::inverse(m_RootMotionRotationDelta) * m_RootMotionRotation;
}

glm::quat shade::animation::Pose::GetRootMotionRotation() const
{
	return m_RootMotionRotation;
}
