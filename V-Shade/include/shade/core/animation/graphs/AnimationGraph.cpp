#include "shade_pch.h"
#include "AnimationGraph.h"

shade::animation::AnimationGraph::AnimationGraph(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour, graphs::GraphContext* context) : BaseAsset(assetData, lifeTime, behaviour),
BaseNode(context, 0u, nullptr)
{

}

shade::animation::AnimationGraph::AnimationGraph(graphs::GraphContext* context, const std::string& name) : BaseNode(context, 0u, nullptr, name)
{
	context->GetNodes().emplace(this, graphs::NodesPack{});
	SetRootNode(CreateNode<OutputPoseNode>());
}

void shade::animation::AnimationGraph::ProcessGraph(const shade::FrameTimer& deltaTime)
{
	ProcessBranch(deltaTime);
}

const shade::animation::Pose* shade::animation::AnimationGraph::GetOutPutPose() const
{
	return GetRootNode()->As<OutputPoseNode>().GetFinalPose();
}

void shade::animation::AnimationGraph::Evaluate(const FrameTimer& deltaTime)
{
	GetRootNode()->ProcessBranch(deltaTime);
}

bool shade::animation::AnimationGraph::RemoveNode(BaseNode* pNode)
{
	RemoveInputNode(pNode);
	return GetGraphContext()->RemoveNode(pNode);
}

shade::graphs::BaseNode* shade::animation::AnimationGraph::GetInputNode(const std::string& name)
{
	auto node = m_InputNodes.find(name);
	if (node != m_InputNodes.end())
		return node->second;

	SHADE_CORE_WARNING("Input node has not been found : {0}", name);
	return nullptr;
}

const shade::graphs::BaseNode* shade::animation::AnimationGraph::GetInputNode(const std::string& name) const
{
	auto node = m_InputNodes.find(name);
	assert(node != m_InputNodes.end() && "Input node has not been found!");
	return node->second;
}

template<typename... Args>
shade::graphs::BaseNode* DeserializeNodeBodyByType(std::istream& stream, shade::graphs::BaseNode* pParrentNode, std::uint32_t type)
{
	if (shade::graphs::GetNodeTypeId<shade::graphs::BoolNode>() == type) return pParrentNode->CreateNode<shade::graphs::BoolNode>();
	if (shade::graphs::GetNodeTypeId<shade::graphs::FloatEqualsNode>() == type) return pParrentNode->CreateNode<shade::graphs::FloatEqualsNode>();
	if (shade::graphs::GetNodeTypeId<shade::graphs::FloatNode>() == type) return pParrentNode->CreateNode<shade::graphs::FloatNode>();
	if (shade::graphs::GetNodeTypeId<shade::graphs::IntEqualsNode>() == type) return pParrentNode->CreateNode<shade::graphs::IntEqualsNode>();
	if (shade::graphs::GetNodeTypeId<shade::graphs::IntNode>() == type) return pParrentNode->CreateNode<shade::graphs::IntNode>();
	if (shade::graphs::GetNodeTypeId<shade::graphs::StringNode>() == type) return pParrentNode->CreateNode<shade::graphs::StringNode>();

	//if (shade::graphs::GetNodeTypeId<shade::animation::AnimationGraph>() == type) return pParrentNode->CreateNode<shade::animation::AnimationGraph>();
	if (shade::graphs::GetNodeTypeId<shade::animation::BlendNode2D>() == type) return pParrentNode->CreateNode<shade::animation::BlendNode2D>();
	if (shade::graphs::GetNodeTypeId<shade::animation::BoneMaskNode>() == type) return pParrentNode->CreateNode<shade::animation::BoneMaskNode>();
	if (shade::graphs::GetNodeTypeId<shade::animation::OutputPoseNode>() == type)
	{
		return &pParrentNode->GetRootNode()->As<shade::animation::OutputPoseNode>();
	}
	if (shade::graphs::GetNodeTypeId<shade::animation::PoseNode>() == type)
	{
		shade::animation::PoseNode* node = pParrentNode->CreateNode<shade::animation::PoseNode>(); shade::Serializer::Deserialize(stream, node->GetAnimationData());
		return node;
	}
	if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::OutputTransitionNode>() == type)return pParrentNode->CreateNode<shade::animation::state_machine::OutputTransitionNode>();
	if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::TransitionNode>() == type)
	{
		shade::animation::state_machine::TransitionNode::Data transitionData;
		shade::graphs::NodeIdentifier srcIdentifier, dstIdentifier;

		shade::Serializer::Deserialize(stream, srcIdentifier);
		shade::Serializer::Deserialize(stream, dstIdentifier);

		shade::graphs::BaseNode* pSourceNode = pParrentNode->GetGraphContext()->FindInternalNode(pParrentNode, srcIdentifier);
		shade::graphs::BaseNode* pDestinationNode = pParrentNode->GetGraphContext()->FindInternalNode(pParrentNode, dstIdentifier);

		if (pSourceNode && pDestinationNode)
		{
			transitionData.SourceState = &pSourceNode->As<shade::animation::state_machine::StateNode>();
			transitionData.DestinationState = &pDestinationNode->As<shade::animation::state_machine::StateNode>();
		}
	
		return pParrentNode->As<shade::animation::state_machine::StateNode>().AddTransition(&pDestinationNode->As<shade::animation::state_machine::StateNode>());
	}
	if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::StateNode>() == type) return pParrentNode->CreateNode<shade::animation::state_machine::StateNode>();
	if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::StateMachineNode>() == type) return pParrentNode->CreateNode<shade::animation::state_machine::StateMachineNode>();
	
	return nullptr;
}

template<typename... Args>
void SerializeNodeBodyByType(std::ostream& stream, const shade::graphs::BaseNode* pNode, std::uint32_t type)
{
	/*if (shade::graphs::GetNodeTypeId<shade::graphs::BoolNode>() == type) 
	if (shade::graphs::GetNodeTypeId<shade::graphs::FloatEqualsNode>() == type) 
	if (shade::graphs::GetNodeTypeId<shade::graphs::FloatNode>() == type)
	if (shade::graphs::GetNodeTypeId<shade::graphs::IntEqualsNode>() == type)
	if (shade::graphs::GetNodeTypeId<shade::graphs::IntNode>() == type)
	if (shade::graphs::GetNodeTypeId<shade::graphs::StringNode>() == type)*/

	if (shade::graphs::GetNodeTypeId<shade::animation::AnimationGraph>() == type)
	{
		SHADE_CORE_INFO("Serialize body section of {0}", "AnimationGraph");
	}
	//if (shade::graphs::GetNodeTypeId<shade::animation::BlendNode2D>() == type)
	
	//if (shade::graphs::GetNodeTypeId<shade::animation::BoneMaskNode>() == type) return pParrentNode->CreateNode<shade::animation::BoneMaskNode>();
	//if (shade::graphs::GetNodeTypeId<shade::animation::OutputPoseNode>() == type) return pParrentNode->CreateNode<shade::animation::OutputPoseNode>();
	if (shade::graphs::GetNodeTypeId<shade::animation::PoseNode>() == type) 
	{
		shade::Serializer::Serialize(stream, pNode->As<shade::animation::PoseNode>().GetAnimationData());
	}
	if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::OutputTransitionNode>() == type)
	{
		SHADE_CORE_INFO("Serialize body section of {0}", "OutputTransitionNode");
	}
	if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::TransitionNode>() == type)
	{
		// Если мы попытаемся десериализовать то дестинатион стейт может быть еще не десиарилизована поэтому там будет не верный нод id, его еше не будет в конетексте
		const shade::animation::state_machine::TransitionNode::Data& transitionData = pNode->As<shade::animation::state_machine::TransitionNode>().GetTransitionData();

		shade::Serializer::Serialize(stream, transitionData.SourceState->GetNodeIdentifier());
		shade::Serializer::Serialize(stream, transitionData.DestinationState->GetNodeIdentifier());
	}
	if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::StateNode>() == type)
	{
		SHADE_CORE_INFO("Serialize body section of {0}", "StateNode");
	}
	//if (shade::graphs::GetNodeTypeId<shade::animation::state_machine::StateMachineNode>() == type)
}


std::size_t SerializeNodeRecursively(std::ostream& stream, const shade::graphs::BaseNode* pNode)
{
	// SerialzieSerialzie type
	std::size_t size = shade::Serializer::Serialize(stream, pNode->GetNodeType());
	// Serialzie Identifier
	size += shade::Serializer::Serialize(stream, pNode->GetNodeIdentifier());
	// Serialzie Name
	size += shade::Serializer::Serialize(stream, pNode->GetName());
	
	// Serialzie screen position
	size += shade::Serializer::Serialize(stream, pNode->GetScreenPosition());
	// Serialzie count of internal nodes
	size += shade::Serializer::Serialize(stream, std::uint32_t(pNode->GetInternalNodes().size()));

	//------------------------------------------------------------------------
	// Body section
	//------------------------------------------------------------------------
		SerializeNodeBodyByType(stream, pNode, pNode->GetNodeType());
	//------------------------------------------------------------------------
	// !Body section
	//------------------------------------------------------------------------

	const auto& inputs = pNode->GetEndpoints().at(shade::graphs::Connection::Type::Input);
	const auto& outputs = pNode->GetEndpoints().at(shade::graphs::Connection::Type::Output);

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
	for (const auto node : pNode->GetInternalNodes())
	{
		size += SerializeNodeRecursively(stream, node);
	}

	// Serialize root node id 
	size += shade::Serializer::Serialize(stream, (pNode->GetRootNode()) ? pNode->GetRootNode()->GetNodeIdentifier() : shade::graphs::INVALID_NODE_IDENTIFIER);

	return size;
}

std::size_t shade::animation::AnimationGraph::Serialize(std::ostream& stream) const
{
	//3. Сначала серриалайз чилдрен, потом сам нод, затем коннекшены
	//5. Проблема состоит в серриализации инпут нодов и их конекшенов к остальным, видимо надо создавать пометку что это инпут

	std::size_t size = SerializeNodeRecursively(stream, this);
	// Serialize Graph context
	size += Serializer::Serialize(stream, *GetGraphContext());

	return size;
}

std::size_t DeserializeNodeRecursively(std::istream& stream, shade::graphs::BaseNode* pParrentNode, shade::graphs::GraphContext* pGraphContext)
{
	// Deserialize Type
	std::uint32_t type;	std::size_t		size = shade::Serializer::Deserialize(stream, type);
	// Deserialize Identifier
	shade::graphs::NodeIdentifier id;	size += shade::Serializer::Deserialize(stream, id);
	// Deserialize Name
	std::string name; 					size += shade::Serializer::Deserialize(stream, name);
	
	// Deserialize Screen position
	glm::vec2 screenPosition;			size += shade::Serializer::Deserialize(stream, screenPosition);
	// Deserialize count of internal nodes
	std::uint32_t internalNodesCount;	size += shade::Serializer::Deserialize(stream, internalNodesCount);

	//------------------------------------------------------------------------
	// Body section
	//------------------------------------------------------------------------
		shade::graphs::BaseNode* pNode = DeserializeNodeBodyByType(stream, pParrentNode, type); pNode->SetName(name); pNode->SetNodeIdentifier(id); pNode->GetScreenPosition() = screenPosition;
	//------------------------------------------------------------------------
	// !Body section
	//------------------------------------------------------------------------

	auto& inputs	= pNode->GetEndpoints().at(shade::graphs::Connection::Type::Input);
	auto& outputs	= pNode->GetEndpoints().at(shade::graphs::Connection::Type::Output);

	//------------------------------------------------------------------------
	// Endpoints section
	//------------------------------------------------------------------------

	std::uint32_t inputEndpointsCount;	size += shade::Serializer::Deserialize(stream, inputEndpointsCount);

	for (std::uint32_t i = 0; i < inputEndpointsCount; ++i)
	{
		size += shade::Serializer::Deserialize(stream, inputs.At(i)); // d means default value
	}

	std::uint32_t outputEndpointsCount;  size += shade::Serializer::Deserialize(stream, outputEndpointsCount);

	for (std::uint32_t i = 0; i < outputEndpointsCount; ++i)
	{
		size += shade::Serializer::Deserialize(stream, outputs.At(i)); // d means default value
	}

	//------------------------------------------------------------------------
	// !Endpoints section
	//------------------------------------------------------------------------

	for (std::size_t i = 0; i < internalNodesCount; ++i)
	{
		size += DeserializeNodeRecursively(stream, pParrentNode, pGraphContext);
	}

	// Deserialize root node id  
	shade::graphs::NodeIdentifier rootId;	  size += shade::Serializer::Deserialize(stream, rootId);

	if(rootId != shade::graphs::INVALID_NODE_IDENTIFIER)
		pNode->SetRootNode(pNode->GetGraphContext()->FindInternalNode(pNode, rootId));

	return size;
}

std::size_t shade::animation::AnimationGraph::Deserialize(std::istream& stream)
{
	//------------------------------------------------------------------------
	// AnimationGraph sections
	//------------------------------------------------------------------------
	
	// Deserialize Type
	std::uint32_t type; std::size_t	size = shade::Serializer::Deserialize(stream, type);
	// Deserialize Identifier
	size += shade::Serializer::Deserialize(stream, m_NodeIdentifier);
	// Deserialize Name
	size +=  shade::Serializer::Deserialize(stream, m_Name);

	// Deserialize Screen position
	size += shade::Serializer::Deserialize(stream, this->GetScreenPosition());
	// Deserialize count of internal nodes
	std::uint32_t internalNodesCount; size += shade::Serializer::Deserialize(stream, internalNodesCount);

	auto& inputs = this->GetEndpoints().at(shade::graphs::Connection::Type::Input);
	auto& outputs = this->GetEndpoints().at(shade::graphs::Connection::Type::Output);

	//------------------------------------------------------------------------
	// Endpoints section
	//------------------------------------------------------------------------

	std::uint32_t inputEndpointsCount; size += shade::Serializer::Deserialize(stream, inputEndpointsCount);

	for (std::uint32_t i = 0; i < inputEndpointsCount; ++i)
	{
		size += shade::Serializer::Deserialize(stream, inputs.At(i)); // d means default value
	}

	std::uint32_t outputEndpointsCount;  size += shade::Serializer::Deserialize(stream, outputEndpointsCount);

	for (std::uint32_t i = 0; i < outputEndpointsCount; ++i)
	{
		size += shade::Serializer::Deserialize(stream, outputs.At(i)); // d means default value
	}

	//------------------------------------------------------------------------
	// !Endpoints section
	//------------------------------------------------------------------------
	//------------------------------------------------------------------------
	// !AnimationGraph sections
	//------------------------------------------------------------------------

	// Deserialize internal nodes
	for (std::size_t i = 0; i < internalNodesCount; ++i)
	{
		size += DeserializeNodeRecursively(stream, this, GetGraphContext());
	}

	// Deserialize root node id  
	graphs::NodeIdentifier rootId; size += shade::Serializer::Deserialize(stream, rootId);

	if (rootId != shade::graphs::INVALID_NODE_IDENTIFIER)
		SetRootNode(GetGraphContext()->FindInternalNode(this, rootId));
	
	// GraphContext
	size += Serializer::Deserialize(stream, *GetGraphContext());

	return size;
}
