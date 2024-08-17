#include "shade_pch.h"
#include "BaseNode.h"

shade::graphs::BaseNode::BaseNode(GraphContext* context, NodeIdentifier identifier, BaseNode* pParentNode, const std::string& name) :
	m_pGraphContext(context), m_NodeIdentifier(identifier), m_Name(name), m_pParrentNode(pParentNode),
	m_pRootParrentNode(pParentNode ? (pParentNode->GetParrentRootGraph() ? nullptr : pParentNode) : nullptr)
{
}

shade::graphs::BaseNode::~BaseNode()
{
	assert(m_pParrentNode == nullptr);
	assert(m_pRootParrentNode == nullptr);
}

std::uint32_t shade::graphs::BaseNode::GetNodeStaticType()
{
	return GetNodeTypeId<BaseNode>();
}

std::uint32_t shade::graphs::BaseNode::GetNodeType() const
{
	return GetNodeStaticType();
}

void shade::graphs::BaseNode::Shutdown()
{
	m_pParrentNode = nullptr;
	m_pRootParrentNode = nullptr;
}

bool shade::graphs::BaseNode::ConnectNodes(EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint)
{
	return m_pGraphContext->ConnectNodes(this, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint);
}

bool shade::graphs::BaseNode::DisconnectNodes(EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint)
{
	return m_pGraphContext->DisconnectNodes(this, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint);
}

void shade::graphs::BaseNode::SetRootNode(BaseNode* node)
{
	m_pRootNode = node;
}

shade::graphs::BaseNode* shade::graphs::BaseNode::GetRootNode()
{
	return m_pRootNode;
}

const shade::graphs::BaseNode* shade::graphs::BaseNode::GetRootNode() const
{
	return m_pRootNode;
}

bool shade::graphs::BaseNode::RemoveNode(BaseNode* pNode)
{
	return m_pGraphContext->RemoveNode(pNode);
}

void shade::graphs::BaseNode::OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint)
{
	if (connectionType == Connection::Type::Input)
		m_Endpoints[Connection::Type::Input].Reset(endpoint);
}

void shade::graphs::BaseNode::ProcessBranch(const FrameTimer& deltaTime)
{
	//1. If there no connections we need to Evaluate node and go back to parrent
	//2. If there some connections we need go till end of branch
	assert(m_pGraphContext != nullptr);

	auto connections = m_pGraphContext->GetConnections(this);

	if (connections)
	{
		for (auto& connection : *connections)
			connection.PConnectedFrom->ProcessBranch(deltaTime);
	}

	Evaluate(deltaTime);
}
