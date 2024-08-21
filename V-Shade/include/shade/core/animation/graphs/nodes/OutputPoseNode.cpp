#include "shade_pch.h"
#include "OutputPoseNode.h"

void shade::animation::OutputPoseNode::Evaluate(const FrameTimer& delatTime)
{

}

const shade::animation::Pose* shade::animation::OutputPoseNode::GetFinalPose() const
{
	return GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(0);
}