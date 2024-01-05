#include "shade_pch.h"
#include "PoseNode.h"
#include <shade/core/animation/AnimationController.h>

void shade::animation::PoseNode::Evaluate(const FrameTimer& deltaTime)
{
	auto& controller	= GetGraphContext().Controller;
	auto& skeleton		= GetGraphContext().Skeleton;

	if (AnimationData.Animation) 
		GET_ENDPOINT<Connection::Output, NodeValueType::Pose>(0,controller->ProcessPose(skeleton, AnimationData, deltaTime));
}

void shade::animation::PoseNode::ResetAnimationData(const Asset<Animation>& animation)
{
	AnimationData = AnimationController::AnimationControllData(animation);
}

shade::animation::AnimationController::AnimationControllData& shade::animation::PoseNode::GetAnimationData()
{
	return AnimationData;
}
