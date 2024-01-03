#include "shade_pch.h"
#include "BlendNode2D.h"


void shade::animation::BlendNode2D::Evaluate(const FrameTimer& deltaTime)
{
	auto& controller = GetGraphContext().Controller;
	auto& skeleton = GetGraphContext().Skeleton;

	const Pose* source			= GET_INPUT_ENDPOINT(POSE, 1);
	const Pose* destination		= GET_INPUT_ENDPOINT(POSE, 2);

	GET_OUTPUT_ENDPOINT(POSE, 0) = controller->Blend(skeleton, source, destination, GET_INPUT_ENDPOINT(FLOAT, 0), Mask);
}

void shade::animation::BlendNode2D::OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint)
{
	if (connectionType == Connection::Type::Input)
	{
		switch (type)
		{
		case shade::animation::NodeValueType::FLOAT: GET_INPUT_ENDPOINT(FLOAT, endpoint) = DefaultWeightValue; break;
		case shade::animation::NodeValueType::POSE: GET_INPUT_ENDPOINT(POSE, endpoint) = nullptr; break;
		}
	}
	if (connectionType == Connection::Type::Output)
	{
		switch (type)
		{
		case shade::animation::NodeValueType::POSE: GET_OUTPUT_ENDPOINT(POSE, endpoint) = nullptr; break;
		}
	}
}
