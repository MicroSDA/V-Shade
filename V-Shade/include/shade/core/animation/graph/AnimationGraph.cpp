#include "shade_pch.h"
#include "AnimationGraph.h"

shade::AssetMeta::Type shade::animation::AnimationGraph::GetAssetStaticType()
{
	// TODO: Change !!
	return AssetMeta::Type::Asset;
}

shade::AssetMeta::Type shade::animation::AnimationGraph::GetAssetType() const
{
	return GetAssetStaticType();
}

shade::animation::AnimationGraph::AnimationGraph(const Asset<Skeleton>& skeleton)
{
	m_AnimationController = AnimationController::Create();
	m_GraphContext.Controller = m_AnimationController;
	m_GraphContext.Skeleton = skeleton;

	m_RootNode = SharedPointer<OutputPoseNode>::Create(~0, m_GraphContext);
}

shade::Asset<shade::animation::AnimationGraph> shade::animation::AnimationGraph::CreateEXP(const Asset<Skeleton>& skeleton)
{
	return SharedPointer<AnimationGraph>::Create(skeleton);
}

bool shade::animation::AnimationGraph::AddConnection(GraphNode::NodeIDX inputNode, GraphNode::EndpointIDX inputEndpoint, GraphNode::NodeIDX outputNode, GraphNode::EndpointIDX outputEndpoint)
{
	if (inputNode == ~0)
	{
		AddRootConnection(outputNode, outputEndpoint);
	}
	else
	{
		auto input			= FindNode(inputNode);
		auto output			= FindNode(outputNode);

		if (!input || !output || inputNode == outputNode) return false;

		auto inputValue		= input->__GET_ENDPOINT<GraphNode::Connection::Input>(inputEndpoint);
		auto outputValue	= output->__GET_ENDPOINT<GraphNode::Connection::Output>(outputEndpoint);

		if (!inputValue || !outputValue) return false;

		if (inputValue->get()->GetType() != outputValue->get()->GetType()) return false;

		bool hasBeenConnected = ConnectValues(inputValue, outputValue);

		if (hasBeenConnected)
		{
			input->AddConnection(inputEndpoint, outputNode, outputEndpoint, GraphNode::Connection::Input);
			input->AddChild(output);

			input->OnConnect(GraphNode::Connection::Type::Input,	inputValue->get()->GetType(),	inputEndpoint);
			output->OnConnect(GraphNode::Connection::Type::Output,	outputValue->get()->GetType(),	outputEndpoint);
		}

		return hasBeenConnected;
	}
}

bool shade::animation::AnimationGraph::RemoveConnection(GraphNode::NodeIDX inputNode, GraphNode::EndpointIDX inputEndpoint)
{
	if (inputNode == ~0)
	{
		RemoveRootConnection();
	} 
	else
	{
		auto input = FindNode(inputNode);

		if (!input) return false;

		auto connection = input->FindConnection(inputEndpoint);

		if (connection.InputEndpoint != ~0u)
		{
			auto output = FindNode(connection.OutputNodeIdx);

			auto inputValue = input->__GET_ENDPOINT<GraphNode::Connection::Input>(inputEndpoint);
			auto outputValue = output->__GET_ENDPOINT<GraphNode::Connection::Output>(connection.OutputEndpoint);

			if (!inputValue || !outputValue) return false;

			//TODO: We need to garanty that there is only one same child connected once !!!!!!
			if (input->RemoveConnection(inputEndpoint))
			{
				if (output) input->RemoveChild(output);

				input->OnDisconnect(GraphNode::Connection::Type::Input, inputValue->get()->GetType(), inputEndpoint);
				output->OnDisconnect(GraphNode::Connection::Type::Output, outputValue->get()->GetType(), connection.OutputEndpoint);

				return true;

			}
			
			return false;
		}

		return false;
	}
}

bool shade::animation::AnimationGraph::AddRootConnection(GraphNode::NodeIDX outputNode, GraphNode::EndpointIDX outputEndpoint)
{
	auto output	= FindNode(outputNode);

	if (!output) return false;

	auto outputValue = output->__GET_ENDPOINT<GraphNode::Connection::Output>(outputEndpoint);

	if (!outputValue || outputValue->get()->GetType() != NodeValueType::Pose) return false;

	auto rootNode = m_RootNode->__GET_ENDPOINT<GraphNode::Connection::Input>(0);
	bool hasBeenConnected = ConnectValues(rootNode, outputValue);

	if (hasBeenConnected)
	{
		m_RootNode->AddConnection(0, outputNode, outputEndpoint, GraphNode::Connection::Input);
		m_RootNode->AddChild(output);

		m_RootNode->OnConnect(GraphNode::Connection::Type::Input, outputValue->get()->GetType(), 0);
		output->OnConnect(GraphNode::Connection::Type::Output, outputValue->get()->GetType(), outputEndpoint);

	}
	
	return hasBeenConnected;
}

bool shade::animation::AnimationGraph::RemoveRootConnection()
{
	auto connection = m_RootNode->FindConnection(0);

	if (connection.InputEndpoint != ~0u)
	{
		auto output = FindNode(connection.OutputNodeIdx);

		auto inputValue		= m_RootNode->__GET_ENDPOINT<GraphNode::Connection::Input>(0);
		auto outputValue	= output->__GET_ENDPOINT<GraphNode::Connection::Output>(connection.OutputEndpoint);

		if (!inputValue || !outputValue) return false;

		//TODO: We need to garanty that there is only one same child connected once !!!!!!
		if (m_RootNode->RemoveConnection(0))
		{
			m_RootNode->RemoveChild(output);

			m_RootNode->OnDisconnect(GraphNode::Connection::Type::Input, inputValue->get()->GetType(), 0);
			output->OnDisconnect(GraphNode::Connection::Type::Output, outputValue->get()->GetType(), connection.OutputEndpoint);

			return true;

		}
	
		return false;
	}

	return false;
}

void shade::animation::AnimationGraph::SetSkeleton(const Asset<Skeleton>& skeleton)
{
	m_GraphContext.Skeleton = skeleton;
}

const shade::Asset<shade::Skeleton>& shade::animation::AnimationGraph::GetSkeleton() const
{
	return m_GraphContext.Skeleton;
}

void shade::animation::AnimationGraph::Evaluate(const FrameTimer& delatTime)
{
	m_RootNode->ProcessBranch(delatTime);
}

const shade::SharedPointer<shade::animation::OutputPoseNode>& const shade::animation::AnimationGraph::GetOutputPoseNode()
{
	return m_RootNode;
}

shade::SharedPointer<shade::animation::GraphNode> shade::animation::AnimationGraph::FindNode(GraphNode::NodeIDX idx)
{
	auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(), [idx](const shade::SharedPointer<GraphNode>& node)
		{
			return node->GetNodeIndex() == idx;
		});

	if (it != m_Nodes.end()) return *it;

	return nullptr;
}

bool shade::animation::AnimationGraph::ConnectValues(std::shared_ptr<NodeValue>* inputEndpoint, const std::shared_ptr<NodeValue>* outputEndpoint)
{
	if (!inputEndpoint || !outputEndpoint)
		return false;

	*inputEndpoint = *outputEndpoint;

	return true;
}


shade::animation::AnimationGraph::AnimationGraph(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
{
	assert(false && "How about Context !");
	m_AnimationController = AnimationController::Create();
	m_RootNode = SharedPointer<OutputPoseNode>::Create(~0, m_GraphContext);
	m_AnimationController = AnimationController::Create();
}

shade::animation::AnimationGraph* shade::animation::AnimationGraph::Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour)
{
	return new AnimationGraph(assetData, lifeTime, behaviour);
}

std::size_t shade::animation::AnimationGraph::Serialize(std::ostream& stream) const
{
	return std::size_t();
}

std::size_t shade::animation::AnimationGraph::Deserialize(std::istream& stream)
{
	return std::size_t();
}
