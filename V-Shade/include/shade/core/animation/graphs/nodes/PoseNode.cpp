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
			const state_machine::StateNode::TransitionSyncData syncData = GetParrentGraph()->As<state_machine::StateNode>().GetTransitionSyncData();

			if (syncData.PStateAnimationDuration > 0.f)
			{
				auto multiplier = controller->GetTimeMultiplier(AnimationData.Duration, syncData.PStateAnimationDuration, syncData.BlendFactor);

				if (!syncData.CurrentTransitionTime)
				{
					AnimationData.CurrentPlayTime = 0.f;
				}
				if (syncData.Offset)
				{
					AnimationData.CurrentPlayTime  = syncData.Offset;
				}

				GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, AnimationData, deltaTime, multiplier.first));
			}
			else
			{
				GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, AnimationData, deltaTime));
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
