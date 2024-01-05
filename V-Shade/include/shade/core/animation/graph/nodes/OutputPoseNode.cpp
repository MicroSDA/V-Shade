#include "shade_pch.h"
#include "OutputPoseNode.h"

const shade::animation::Pose* shade::animation::OutputPoseNode::GetFinalPose() const
{
	return GET_ENDPOINT<Connection::Input, NodeValueType::Pose>(0);
}