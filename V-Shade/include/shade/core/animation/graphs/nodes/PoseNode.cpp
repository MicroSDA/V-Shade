#include "shade_pch.h"
#include "PoseNode.h"
#include <shade/core/animation/AnimationController.h>
#include <shade/core/animation/graphs/nodes/StateMachineNode.h>

void shade::animation::PoseNode::Evaluate(const FrameTimer& deltaTime)
{
	auto& controller	= GetGraphContext()->As<AnimationGraphContext>().Controller;
	auto& skeleton		= GetGraphContext()->As<AnimationGraphContext>().Skeleton;

	if (m_AnimationData.Animation)
	{
		if (graphs::GetNodeTypeId<state_machine::StateNode>() == GetParrentGraph()->GetNodeType())
		{
			const state_machine::TransitionSyncData syncData = GetParrentGraph()->As<state_machine::StateNode>().GetTransitionSyncData();

			switch (syncData.Status)
			{
				case state_machine::TransitionStatus::Start:
				{
					if (syncData.Preferences.ResetFromStart)
						m_AnimationData.CurrentPlayTime = m_AnimationData.Start + syncData.Preferences.Offset;
				}
				case state_machine::TransitionStatus::InProcess:
				{
					switch (syncData.Preferences.Style)
					{
					case state_machine::SyncStyle::SourceFrozen:

						m_AnimationData.State = Animation::State::Pause;
						GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, m_AnimationData, deltaTime, syncData.TimeMultiplier));
						break;

					case state_machine::SyncStyle::SourceToDestinationTimeSync:

					case state_machine::SyncStyle::DestinationToSourceTimeSync:

					case state_machine::SyncStyle::DestinationAndSourceTimeSync:

					case state_machine::SyncStyle::KeyFrameSync: break;

					default:
						//m_AnimationData.State = Animation::State::Play;
						GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, m_AnimationData, deltaTime, syncData.TimeMultiplier));
						break;
					}
					break;
				}
				case state_machine::TransitionStatus::End: // When transition end or non transition occurs
				{
					GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, m_AnimationData, deltaTime, syncData.TimeMultiplier));
					break;
				}
			}
		}
		else
		{
			GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, m_AnimationData, deltaTime));
		}
	}
	
}
// TODO: При добавлении анимации нужно хотябы раз ее процеснуть на 0 делта времени что бы поза генерировалась 
void shade::animation::PoseNode::ResetAnimationData(const Asset<Animation>& animation)
{
	m_AnimationData = AnimationController::AnimationControlData(animation);
	
	/*auto oldState = m_AnimationData.State;

	m_AnimationData.State = Animation::State::Play;
	Evaluate({ 0 });
	m_AnimationData.State = oldState;*/
}

void shade::animation::PoseNode::ResetAnimationData(const AnimationController::AnimationControlData& data)
{
	m_AnimationData = data;

	/*auto oldState = m_AnimationData.State;

	m_AnimationData.State = Animation::State::Play;
	Evaluate({ 0 });
	m_AnimationData.State = oldState;*/
}

const shade::animation::AnimationController::AnimationControlData& shade::animation::PoseNode::GetAnimationData() const
{
	return m_AnimationData;
}

shade::animation::AnimationController::AnimationControlData& shade::animation::PoseNode::GetAnimationData()
{
	return m_AnimationData;
}

void shade::animation::PoseNode::SerializeBody(std::ostream& stream) const
{
	SHADE_CORE_INFO("Serialize '{0}' body section...", GetName());
	serialize::Serializer::Serialize(stream, m_AnimationData);
}

void shade::animation::PoseNode::DeserializeBody(std::istream& stream)
{
	SHADE_CORE_INFO("Deserialize '{0}' body section...", GetName());
	AnimationController::AnimationControlData animationData;
	serialize::Serializer::Deserialize(stream, animationData); ResetAnimationData(animationData);
}
