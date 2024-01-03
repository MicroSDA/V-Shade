#include "shade_pch.h"
#include "GraphNode.h"

shade::animation::GraphNode::GraphNode(NodeIDX idx, const GraphContext& context): 
	m_NodeIdx(idx), m_rGraphContext(context)
{

}

shade::animation::GraphNode::NodeIDX shade::animation::GraphNode::GetNodeIndex() const
{
	return m_NodeIdx;
}

void shade::animation::GraphNode::ProcessBranch(const FrameTimer& delatTime)
{
	//1. If there no children we need to Evaluate node and go back to parrent
	//2. If there some child we need go till end of branch

	for (auto& [hash, child] : m_Children)
	{
		child->ProcessBranch(delatTime);
	}

	Evaluate(delatTime);
}

void shade::animation::GraphNode::AddChild(const SharedPointer<GraphNode>& node)
{
	m_Children.insert({ node, node });
}

std::vector<std::shared_ptr<shade::animation::NodeValue>>& shade::animation::GraphNode::GetInputEndpoints()
{
	return INPUT;
}

const std::vector<std::shared_ptr<shade::animation::NodeValue>>& shade::animation::GraphNode::GetInputEndpoints() const
{
	return OUTPUT;
}

void shade::animation::GraphNode::RemoveChild(const SharedPointer<GraphNode>& node)
{
	if (m_Children.find(node) != m_Children.end())
		m_Children.erase(node);
}

const shade::animation::GraphContext& shade::animation::GraphNode::GetGraphContext() const
{
	return m_rGraphContext;
}
