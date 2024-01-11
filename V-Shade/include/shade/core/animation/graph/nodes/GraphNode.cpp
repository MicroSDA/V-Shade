#include "shade_pch.h"
#include "GraphNode.h"

shade::animation::GraphNode::GraphNode(NodeIDX idx, const GraphContext& context) :
	m_NodeIdx(idx), m_rGraphContext(context)
{

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


void shade::animation::GraphNode::RemoveChild(const SharedPointer<GraphNode>& node)
{
	if (m_Children.find(node) != m_Children.end())
		m_Children.erase(node);
}

void shade::animation::GraphNode::OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint)
{
	if (connectionType == Connection::Type::Input)
	{
		m_Endpoints[Connection::Type::Input].Reset(endpoint);
	}
}
