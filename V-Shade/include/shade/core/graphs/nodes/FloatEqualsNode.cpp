#include "shade_pch.h"
#include "FloatEqualsNode.h"

void shade::graphs::FloatEqualsNode::Evaluate(const FrameTimer& deltaTime)
{
	const float input   = GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Float>(0);
	const float compare = GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Float>(1);

	if (input == compare)
	{
		GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Bool>(0, true);
	}
	else
	{
		GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Bool>(0, false);
	}
}
