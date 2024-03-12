#include "shade_pch.h"
#include "BoneMaskNode.h"

void shade::animation::BoneMaskNode::Evaluate(const FrameTimer& delatTime)
{
}

shade::animation::BoneMask& shade::animation::BoneMaskNode::GetBoneMask()
{
	return GET_ENDPOINT<graphs::Connection::Output, NodeValueType::BoneMask>(0);
}
