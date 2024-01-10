#include "shade_pch.h"
#include "BlendNode2D.h"


void shade::animation::BlendNode2D::Evaluate(const FrameTimer& deltaTime)
{
	auto& controller = GetGraphContext().Controller;
	auto& skeleton   = GetGraphContext().Skeleton;

	const Pose* source		= GET_ENDPOINT<Connection::Input, NodeValueType::Pose>(1);
	const Pose* destination	= GET_ENDPOINT<Connection::Input, NodeValueType::Pose>(2);

	auto v = GET_ENDPOINT<Connection::Input, NodeValueType::Float>(0);

	if(source && destination) GET_ENDPOINT<Connection::Output, NodeValueType::Pose>(0, controller->Blend(skeleton, source, destination, GET_ENDPOINT<Connection::Input, NodeValueType::Float>(0), Mask));
}

void shade::animation::BlendNode2D::OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint)
{
	if (connectionType == Connection::Type::Input)
	{
		__GET_ENDPOINT<Connection::Input>(endpoint)->reset();
	}
	/*if (connectionType == Connection::Type::Output)
	{
		switch (type)
		{
		case shade::animation::NodeValueType::Pose:		__GET_ENDPOINT<Connection::Output>(endpoint)->reset(); break;
		}
	}*/
}
