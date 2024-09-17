#include "shade_pch.h"
#include "AdditivePose.h"

shade::animation::AdditivePose::AdditivePose(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode) :
	PoseNode(context, identifier, pParentNode)
{
	m_CanBeOpen = false;

	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(nullptr); // Reference Pose
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(nullptr); // Base pose, t pose

	//REGISTER_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(nullptr);
}

void shade::animation::AdditivePose::Evaluate(const FrameTimer& delatTime)
{
	auto& controller = GetGraphContext()->As<AnimationGraphContext>().Controller;
	auto& skeleton = GetGraphContext()->As<AnimationGraphContext>().Skeleton;

	const Pose* rPose = GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(0);
	const Pose* bPose = (GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(1)) ? GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(1) : controller->GetZeroPose(skeleton);

	if (rPose && bPose)
	{
		GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->GenerateAdditivePose(skeleton, rPose, bPose));
	}
}
