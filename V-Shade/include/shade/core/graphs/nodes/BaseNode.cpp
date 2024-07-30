#include "shade_pch.h"
#include "BaseNode.h"

shade::graphs::BaseNode::BaseNode(GraphContext* context, NodeIdentifier identifier, BaseNode* pParentNode, const std::string& name) :
	m_pGraphContext(context), m_NodeIdentifier(identifier), m_Name(name), m_pParrentNode(pParentNode),
	m_pRootParrentNode(pParentNode ? (pParentNode->GetParrentRootGraph() ? nullptr : pParentNode) : nullptr)
{
}

shade::graphs::BaseNode::~BaseNode()
{
	SHADE_CORE_DEBUG("~BaseNode : {0}, {1}", GetName(), (int)this);

	assert(m_pParrentNode == nullptr);
	assert(m_pRootParrentNode == nullptr);
}

void shade::graphs::BaseNode::Shutdown()
{
	m_pParrentNode = nullptr;
	m_pRootParrentNode = nullptr;
}

bool shade::graphs::BaseNode::ConnectNodes(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint)
{
	// TODO: Add assert 
	if (pConnectedTo == pConnectedFrom) return false;

	auto inputValue		= pConnectedTo->__GET_ENDPOINT<Connection::Input>(connectedToEndpoint);
	auto outputValue	= pConnectedFrom->__GET_ENDPOINT<Connection::Output>(connectedFromEndpoint);

	if (!inputValue || !outputValue) return false;

	if (inputValue->get()->GetType() != outputValue->get()->GetType()) return false;


	if (m_pGraphContext->CreateConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint))
	{
		if (ConnectValues(inputValue, outputValue))
		{
			pConnectedTo->OnConnect(Connection::Type::Input, inputValue->get()->GetType(), connectedToEndpoint);
			pConnectedFrom->OnConnect(Connection::Type::Output, outputValue->get()->GetType(), connectedFromEndpoint);

			return true;
		}
	}
	return false;
}


bool shade::graphs::BaseNode::ConnectValues(std::shared_ptr<NodeValue>* inputEndpoint, const std::shared_ptr<NodeValue>* outputEndpoint)
{
	if (!inputEndpoint || !outputEndpoint)
		return false;

	*inputEndpoint = *outputEndpoint;

	return true;
}

bool shade::graphs::BaseNode::DisconnectNodes(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint)
{
	auto connection = m_pGraphContext->FindConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint);

	if (connection != nullptr && connection->ConnectedToEndpoint != INVALID_NODE_IDENTIFIER)
	{
		NodeValues::Value* connectedToValue		= pConnectedTo->__GET_ENDPOINT<Connection::Input>(connectedToEndpoint);
		NodeValues::Value* connectedFromValue	= pConnectedFrom->__GET_ENDPOINT<Connection::Output>(connectedFromEndpoint);

		if (!connectedToValue || !connectedFromValue) return false;

		if (m_pGraphContext->RemoveConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint))
		{
			pConnectedTo->OnDisconnect(Connection::Type::Input, connectedToValue->get()->GetType(), connectedToEndpoint);
			pConnectedFrom->OnDisconnect(Connection::Type::Output, connectedFromValue->get()->GetType(), connectedFromEndpoint);

			return true;
		}
	}

	return false;
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
	SHADE_CORE_INFO("Node {0}", (int)this);
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
