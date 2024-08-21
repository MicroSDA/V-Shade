#include "shade_pch.h"
#include "Pose.h"
#include <shade/core/render/RenderAPI.h>


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
	m_GlobalTransforms->clear(); // mby dont need to clear, resize will do this ??
	m_GlobalTransforms->resize(RenderAPI::MAX_BONES_PER_INSTANCE, glm::identity<glm::mat4>());

	m_LocalTransforms->clear();
	m_LocalTransforms->resize(RenderAPI::MAX_BONES_PER_INSTANCE, glm::identity<glm::mat4>());

	m_State = State::ZeroPose;
}
