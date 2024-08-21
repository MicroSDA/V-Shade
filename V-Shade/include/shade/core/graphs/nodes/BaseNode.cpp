#include "shade_pch.h"
#include "BaseNode.h"
#include <shade/core/animation/graphs/AnimationGraph.h>

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

std::size_t shade::graphs::BaseNode::SerializeBody(std::ostream& stream) const
{
	SHADE_CORE_INFO("Serialize '{0}' body section...", GetName());
	return 0u;
}

std::size_t shade::graphs::BaseNode::DeserializeBody(std::istream& stream)
{
	SHADE_CORE_INFO("Deserialize '{0}' body section...", GetName());
	return 0u;
}

shade::graphs::BaseNode* shade::graphs::BaseNode::CreateNodeByType(NodeType type)
{
	if (shade::graphs::GetNodeTypeId<shade::animation::AnimationGraph>() == type) return this;

	if (shade::graphs::GetNodeTypeId<shade::graphs::BoolNode>() == type) return CreateNode<shade::graphs::BoolNode>();
	if (shade::graphs::GetNodeTypeId<shade::graphs::FloatEqualsNode>() == type) return CreateNode<shade::graphs::FloatEqualsNode>();
	if (shade::graphs::GetNodeTypeId<shade::graphs::FloatNode>() == type) return CreateNode<shade::graphs::FloatNode>();
	if (shade::graphs::GetNodeTypeId<shade::graphs::IntEqualsNode>() == type) return CreateNode<shade::graphs::IntEqualsNode>();
	if (shade::graphs::GetNodeTypeId<shade::graphs::IntNode>() == type) return CreateNode<shade::graphs::IntNode>();
	if (shade::graphs::GetNodeTypeId<shade::graphs::StringNode>() == type) return CreateNode<shade::graphs::StringNode>();

	if (shade::graphs::GetNodeTypeId<shade::animation::BlendNode2D>() == type) return CreateNode<shade::animation::BlendNode2D>();
	if (shade::graphs::GetNodeTypeId<shade::animation::BoneMaskNode>() == type) return CreateNode<shade::animation::BoneMaskNode>();
	if (shade::graphs::GetNodeTypeId<shade::animation::OutputPoseNode>() == type) return &GetRootNode()->As<shade::animation::OutputPoseNode>();
	if (shade::graphs::GetNodeTypeId<shade::animation::PoseNode>() == type) return CreateNode<shade::animation::PoseNode>();
	
	if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::OutputTransitionNode>() == type) return &GetRootNode()->As<shade::animation::state_machine::OutputTransitionNode>();
	if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::TransitionNode>() == type) return CreateNode<shade::animation::state_machine::TransitionNode>();
	if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::StateNode>() == type) return CreateNode<shade::animation::state_machine::StateNode>();
	if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::StateMachineNode>() == type) return CreateNode<shade::animation::state_machine::StateMachineNode>();

	return nullptr;
}

std::size_t shade::graphs::BaseNode::Serialize(std::ostream& stream) const
{
	// Serialzie Identifier
	std::size_t size = shade::Serializer::Serialize(stream, GetNodeIdentifier());
	// Serialzie Name
	size += shade::Serializer::Serialize(stream, GetName());

	// Serialzie screen position
	size += shade::Serializer::Serialize(stream, GetScreenPosition());
	// Serialzie count of internal nodes
	size += shade::Serializer::Serialize(stream, std::uint32_t(GetInternalNodes().size()));

	//------------------------------------------------------------------------
	// Body section
	//------------------------------------------------------------------------
	size += SerializeBody(stream);
	//------------------------------------------------------------------------
	// !Body section
	//------------------------------------------------------------------------

	const auto& inputs	= GetEndpoints().at(shade::graphs::Connection::Type::Input);
	const auto& outputs = GetEndpoints().at(shade::graphs::Connection::Type::Output);

	//------------------------------------------------------------------------
	// Endpoints section
	//------------------------------------------------------------------------

	size += shade::Serializer::Serialize(stream, std::uint32_t(inputs.GetSize()));
	for (const auto& [i, d] : inputs)
	{
		size += shade::Serializer::Serialize(stream, d); // d means default value
	}

	size += shade::Serializer::Serialize(stream, std::uint32_t(outputs.GetSize()));
	for (const auto& [i, d] : outputs)
	{
		size += shade::Serializer::Serialize(stream, d); // d means default value
	}

	//------------------------------------------------------------------------
	// !Endpoints section
	//------------------------------------------------------------------------

	// Serialize internal nodes
	for (const BaseNode* pNode : GetInternalNodes())
	{
		size += shade::Serializer::Serialize(stream, pNode->GetNodeType());
		size += Serializer::Serialize(stream, *pNode);
		
	}

	// Serialize root node id 
	size += shade::Serializer::Serialize(stream, (GetRootNode()) ? GetRootNode()->GetNodeIdentifier() : shade::graphs::INVALID_NODE_IDENTIFIER);

	return size;
}

std::size_t shade::graphs::BaseNode::Deserialize(std::istream& stream)
{
	// Deserialize Identifier
	graphs::NodeIdentifier id; std::size_t size = shade::Serializer::Deserialize(stream, id);
	// Deserialize Name
	std::string name; 					size += shade::Serializer::Deserialize(stream, name);

	// Deserialize Screen position
	glm::vec2 screenPosition;			size += shade::Serializer::Deserialize(stream, screenPosition);
	// Deserialize count of internal nodes
	std::uint32_t internalNodesCount;	size += shade::Serializer::Deserialize(stream, internalNodesCount);

	//------------------------------------------------------------------------
	// Body section
	//------------------------------------------------------------------------
	size += DeserializeBody(stream); SetName(name); SetNodeIdentifier(id); GetScreenPosition() = screenPosition;
	//------------------------------------------------------------------------
	// !Body section
	//------------------------------------------------------------------------

	auto& inputs  = GetEndpoints().at(shade::graphs::Connection::Type::Input);
	auto& outputs = GetEndpoints().at(shade::graphs::Connection::Type::Output);

	//------------------------------------------------------------------------
	// Endpoints section
	//------------------------------------------------------------------------

	std::uint32_t inputEndpointsCount;	size += shade::Serializer::Deserialize(stream, inputEndpointsCount);

	for (std::uint32_t i = 0; i < inputEndpointsCount; ++i)
	{
		size += shade::Serializer::Deserialize(stream, inputs.At(i)); // d means default value
	}

	std::uint32_t outputEndpointsCount;  size += shade::Serializer::Deserialize(stream, outputEndpointsCount); // Не правельно вытягивает количстов оутупотов

	for (std::uint32_t i = 0; i < outputEndpointsCount; ++i)
	{
		size += shade::Serializer::Deserialize(stream, outputs.At(i)); // d means default value
	}

	//------------------------------------------------------------------------
	// !Endpoints section
	//------------------------------------------------------------------------

	for (std::size_t i = 0; i < internalNodesCount; ++i)
	{
		NodeType type;	size += shade::Serializer::Deserialize(stream, type);
		BaseNode*		pNode = CreateNodeByType(type);

		size += Serializer::Deserialize(stream, *pNode);
	}

	// Deserialize root node id  
	shade::graphs::NodeIdentifier rootId;	  size += shade::Serializer::Deserialize(stream, rootId);

	if (rootId != shade::graphs::INVALID_NODE_IDENTIFIER)
		SetRootNode(GetGraphContext()->FindInternalNode(this, rootId));

	return size;
}