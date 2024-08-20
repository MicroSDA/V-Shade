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
						m_AnimationData.State = Animation::State::Play;
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

void shade::animation::PoseNode::ResetAnimationData(const Asset<Animation>& animation)
{
	m_AnimationData = AnimationController::AnimationControlData(animation);
}

const shade::animation::AnimationController::AnimationControlData& shade::animation::PoseNode::GetAnimationData() const
{
	return m_AnimationData;
}

shade::animation::AnimationController::AnimationControlData& shade::animation::PoseNode::GetAnimationData()
{
	return m_AnimationData;
}
