#include "shade_pch.h"
#include "BlendNode.h"

void shade::animation::BlendNode::Evaluate(const FrameTimer& deltaTime)
{
	auto& controller = GetGraphContext()->As<AnimationGraphContext>().Controller;
	auto& skeleton   = GetGraphContext()->As<AnimationGraphContext>().Skeleton;

	const Pose* source		= GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(2);
	const Pose* destination	= GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(3);

	if (source && destination)
	{
		GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0,
			controller->Blend(skeleton, source, destination,
				GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Float>(0),
				GET_ENDPOINT<graphs::Connection::Input, NodeValueType::BoneMask>(1)));
	}
}