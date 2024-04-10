#include "shade_pch.h"
#include "BaseNode.h"

shade::graphs::BaseNode::BaseNode(GraphContext* context, NodeIdentifier identifier) :
	m_pGraphContext(context), m_NodeIdentifier(identifier)
{
}

shade::graphs::BaseNode::~BaseNode()
{
	assert(m_pParrentNode == nullptr);
}

void shade::graphs::BaseNode::Initialize(BaseNode* pParentNode, BaseNode* pRootParrentNode)
{
	m_pParrentNode = pParentNode;
	m_pRootParrentNode = pRootParrentNode;
}

void shade::graphs::BaseNode::Shutdown()
{
	for (const auto node : m_Nodes)
	{
		node->Shutdown();
		SDELETE node;
	}

	m_Nodes.clear();
	m_pParrentNode = nullptr;
}

bool shade::graphs::BaseNode::ConnectNodes(BaseNode* inputNode, EndpointIdentifier inputEndpoint, BaseNode* outputNode, EndpointIdentifier outputEndpoint)
{
	/*auto pInput  = FindNode(inputNode);
	auto pOutput = FindNode(outputNode);*/

	if (!inputNode || !outputNode || inputNode == outputNode) return false;

	auto inputValue = inputNode->__GET_ENDPOINT<Connection::Input>(inputEndpoint);
	auto outputValue = outputNode->__GET_ENDPOINT<Connection::Output>(outputEndpoint);

	if (!inputValue || !outputValue) return false;

	if (inputValue->get()->GetType() != outputValue->get()->GetType()) return false;


	if (m_pGraphContext->AddConnection(inputNode, inputEndpoint, outputNode, outputEndpoint, Connection::Input))
	{
		if (ConnectValues(inputValue, outputValue))
		{
			inputNode->OnConnect(Connection::Type::Input, inputValue->get()->GetType(), inputEndpoint);
			outputNode->OnConnect(Connection::Type::Output, outputValue->get()->GetType(), outputEndpoint);

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

bool shade::graphs::BaseNode::DisconnectNodes(NodeIdentifier inputNode, EndpointIdentifier inputEndpoint, NodeIdentifier outputNode, EndpointIdentifier outputEndpoint)
{
	auto pInput = FindNode(inputNode);
	auto pOutput = FindNode(outputNode);

	if (!pInput) return false;

	auto connection = m_pGraphContext->FindConnection(pInput, inputEndpoint, pOutput, outputEndpoint);

	if (connection.InputEndpoint != INVALID_NODE_IDENTIFIER)
	{
		auto inputValue = pInput->__GET_ENDPOINT<Connection::Input>(inputEndpoint);
		auto outputValue = pOutput->__GET_ENDPOINT<Connection::Output>(connection.OutputEndpoint);

		if (!inputValue || !outputValue) return false;

		//TODO: We need to garanty that there is only one same child connected once !!!!!!

		if (m_pGraphContext->RemoveConnection(pInput, inputEndpoint, pOutput, outputEndpoint))
		{
			pInput->OnDisconnect(Connection::Type::Input, inputValue->get()->GetType(), inputEndpoint);
			pOutput->OnDisconnect(Connection::Type::Output, outputValue->get()->GetType(), connection.OutputEndpoint);

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

shade::graphs::BaseNode* shade::graphs::BaseNode::FindNode(NodeIdentifier identifier)
{
	auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(), [identifier](const BaseNode* node)
		{
			return node->GetNodeIdentifier() == identifier;
		});

	if (it != m_Nodes.end()) return *it;

	return nullptr;
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

	auto connections = m_pGraphContext->Connections.find(this);
	if (connections != m_pGraphContext->Connections.end())
	{
		for (auto& connection : connections->second)
			connection.OutputNode->ProcessBranch(deltaTime);
	}

	Evaluate(deltaTime);
}
