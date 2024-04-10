#include "shade_pch.h"
#include "IntEqualsNode.h"
#include <shade/core/event/Input.h>

void shade::graphs::IntEqualsNode::Evaluate(const FrameTimer& deltaTime)
{

	const std::int32_t input   = GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Int>(0);
	const std::int32_t compare = GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Int>(1);

	if (input == compare)
	{
		GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Bool>(0, true);
	}
	else
	{
		GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Bool>(0, false);
	}
	/*if (input == compare)
	{
		GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Bool>(0, true);
	}
	else
	{
		GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Bool>(0, false);
	}*/
}
