#include "shade_pch.h"
#include "PoseNode.h"
#include <shade/core/animation/AnimationController.h>
#include <shade/core/animation/graphs/nodes/StateMachineNode.h>

void shade::animation::PoseNode::Evaluate(const FrameTimer& deltaTime)
{
	auto& controller	= GetGraphContext()->As<AnimationGraphContext>().Controller;
	auto& skeleton		= GetGraphContext()->As<AnimationGraphContext>().Skeleton;

	if (AnimationData.Animation)
	{
		if (dynamic_cast<state_machine::StateNode*>(GetParrentGraph())) // Need to fix thix dynamic cast
		{
			const state_machine::TransitionSyncData syncData = GetParrentGraph()->As<state_machine::StateNode>().GetTransitionSyncData();

			switch (syncData.Status)
			{
				case state_machine::TransitionStatus::Start:
				{
					if (syncData.Preferences.ResetFromStart)
						AnimationData.CurrentPlayTime = AnimationData.Start;
				}
				case state_machine::TransitionStatus::InProcess:
				{
					switch (syncData.Preferences.Style)
					{
					case state_machine::SyncStylePreferences::SourceFrozen:

						AnimationData.State = Animation::State::Pause;
						GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, AnimationData, deltaTime, syncData.TimeMultiplier));
						break;

					case state_machine::SyncStylePreferences::SourceToDestinationTimeSync:

					case state_machine::SyncStylePreferences::DestinationToSourceTimeSync:

					case state_machine::SyncStylePreferences::DestinationAndSourceTimeSync:

					case state_machine::SyncStylePreferences::KeyFrameSync: break;

					default:
						AnimationData.State = Animation::State::Play;
						GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, AnimationData, deltaTime, syncData.TimeMultiplier));
						break;
					}
					break;
				}
				case state_machine::TransitionStatus::End: // When transition end or non transition occurs
				{
					GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, AnimationData, deltaTime, syncData.TimeMultiplier));
					break;
				}
			}
		}
		else
		{
			GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, AnimationData, deltaTime));
		}
	}
	
}

void shade::animation::PoseNode::ResetAnimationData(const Asset<Animation>& animation)
{
	AnimationData = AnimationController::AnimationControlData(animation);
}

shade::animation::AnimationController::AnimationControlData& shade::animation::PoseNode::GetAnimationData()
{
	return AnimationData;
}
