#include "shade_pch.h"
#include "PoseNode.h"
#include <shade/core/animation/AnimationController.h>

void shade::animation::PoseNode::Evaluate(const FrameTimer& deltaTime)
{
	auto& controller	= GetGraphContext().Controller;
	auto& skeleton		= GetGraphContext().Skeleton;

	if (AnimationData.Animation) GET_OUTPUT_ENDPOINT(POSE, 0) = controller->ProcessPose(skeleton, AnimationData.Animation, 0, 0, deltaTime);
}
