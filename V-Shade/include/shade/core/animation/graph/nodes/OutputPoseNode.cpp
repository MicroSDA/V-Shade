#include "shade_pch.h"
#include "OutputPoseNode.h"

const shade::animation::Pose* shade::animation::OutputPoseNode::GetFinalPose() const
{
	const auto finalPose = GET_INPUT_ENDPOINT(POSE, 0);
	return ((finalPose) ? finalPose : nullptr);
}