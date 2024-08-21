#include "shade_pch.h"
#include "AnimationGraph.h"

shade::animation::AnimationGraph::AnimationGraph(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour, graphs::GraphContext* context) : BaseAsset(assetData, lifeTime, behaviour),
BaseNode(context, 0u, nullptr)
{
	context->GetNodes().emplace(this, graphs::NodesPack{});
	Initialize();
}

shade::animation::AnimationGraph::AnimationGraph(graphs::GraphContext* context, const std::string& name) : BaseNode(context, 0u, nullptr, name)
{
	context->GetNodes().emplace(this, graphs::NodesPack{});
	Initialize();
}

void shade::animation::AnimationGraph::Initialize()
{
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

	//SHADE_CORE_WARNING("Input node has not been found : {0}", name);
	return nullptr;
}

const shade::graphs::BaseNode* shade::animation::AnimationGraph::GetInputNode(const std::string& name) const
{
	auto node = m_InputNodes.find(name);
	assert(node != m_InputNodes.end() && "Input node has not been found!");
	return node->second;
}

std::size_t shade::animation::AnimationGraph::Serialize(std::ostream& stream) const
{
	// Serialzie type
	std::size_t size = shade::Serializer::Serialize(stream, GetNodeType());
	// Serialzie Identifier
	size += shade::Serializer::Serialize(stream, GetNodeIdentifier());
	// Serialzie Name
	size += shade::Serializer::Serialize(stream, GetName());

	// Serialzie screen position
	size += shade::Serializer::Serialize(stream, GetScreenPosition());
	// Serialzie count of internal nodes
	size += shade::Serializer::Serialize(stream, std::uint32_t(GetInternalNodes().size() - GetInputNodes().size()));

	//------------------------------------------------------------------------
	// Body section
	//------------------------------------------------------------------------
	size += SerializeBody(stream);
	//------------------------------------------------------------------------
	// !Body section
	//------------------------------------------------------------------------

	const auto& inputs = GetEndpoints().at(shade::graphs::Connection::Type::Input);
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

	// Serialize internal nodes only
	for (const BaseNode* pNode : GetInternalNodes())
	{
		if (GetInputNodes().end() == std::find_if(GetInputNodes().begin(), GetInputNodes().end(), [pNode](const std::pair<std::string, BaseNode*>& input)
			{
				return input.second == pNode;
			}))
		{
			// Serialize type
			size += shade::Serializer::Serialize(stream, pNode->GetNodeType());
			// Serialize node
			size += shade::Serializer::Serialize(stream, *pNode);
		}
	}

	// Serialize input nodes count
	size += shade::Serializer::Serialize(stream, std::uint32_t(GetInputNodes().size()));

	// Serialize input nodes only
	for (const auto& [name, pNode] : GetInputNodes())
	{
		// Serialize type
		size += shade::Serializer::Serialize(stream, pNode->GetNodeType());
		// Serialize node
		size += shade::Serializer::Serialize(stream, *pNode);
	}
	// Serialize root node id 
	size += shade::Serializer::Serialize(stream, (GetRootNode()) ? GetRootNode()->GetNodeIdentifier() : shade::graphs::INVALID_NODE_IDENTIFIER);

	// Serialize Graph context
	size += Serializer::Serialize(stream, *GetGraphContext());

	return size;
}

std::size_t shade::animation::AnimationGraph::Deserialize(std::istream& stream)
{
	// Deserialize type
	graphs::NodeType type; std::size_t  size = shade::Serializer::Deserialize(stream, type);
	// Deserialize Identifier
	shade::graphs::NodeIdentifier id;   size += shade::Serializer::Deserialize(stream, id);
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

	auto& inputs = GetEndpoints().at(shade::graphs::Connection::Type::Input);
	auto& outputs = GetEndpoints().at(shade::graphs::Connection::Type::Output);

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
	
	// Deserealize internal nodes
	for (std::size_t i = 0; i < internalNodesCount; ++i)
	{
		// Deserialize Type
		graphs::NodeType type;	size += shade::Serializer::Deserialize(stream, type);
		BaseNode*				pNode = CreateNodeByType(type);
		// Deserialize node
		size += Serializer::Deserialize(stream, *pNode);
	}

	// Deserealize input nodes count
	std::uint32_t inputNodesCount; size += Serializer::Deserialize(stream, inputNodesCount);

	// Deserealize input nodes
	for (std::size_t i = 0; i < inputNodesCount; ++i)
	{
		// Deserialize Type
		graphs::NodeType type;	size += shade::Serializer::Deserialize(stream, type);
		BaseNode* pNode					= CreateNodeByType(type);

		// Deserialize node
		size += Serializer::Deserialize(stream, *pNode);
		// Add to inputs nodes
		m_InputNodes.emplace(pNode->GetName(), pNode);
	}

	// Deserialize root node id  
	shade::graphs::NodeIdentifier rootId;	  size += shade::Serializer::Deserialize(stream, rootId);

	if (rootId != shade::graphs::INVALID_NODE_IDENTIFIER)
		SetRootNode(GetGraphContext()->FindInternalNode(this, rootId));

	// Deserialize Graph context
	size += Serializer::Deserialize(stream, *GetGraphContext());

	for (auto [node, pack] : GetGraphContext()->GetNodes())
	{
		// Try to process state once it was created to get all animtions created and played 
		(state_machine::StateNode::GetNodeStaticType() == node->GetNodeType()) ? node->Evaluate({ 0 }) : void();
	}

	return size;
}
