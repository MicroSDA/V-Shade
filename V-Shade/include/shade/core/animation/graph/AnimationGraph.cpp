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

bool shade::animation::AnimationGraph::AddConnection(GraphNode::NodeIDX sourceNode, GraphNode::EndpointIDX sourceEndpoint, GraphNode::NodeIDX destinationNode, GraphNode::EndpointIDX destinationEndpoint)
{
	auto source			= FindNode(sourceNode);
	auto destination    = FindNode(destinationNode);

	if (!source || !destination) return false;

	auto sourceOutValue = source->__GetEndpoint<GraphNode::Connection::Output>(sourceEndpoint);
	auto destinationInValue = destination->__GetEndpoint<GraphNode::Connection::Input>(destinationEndpoint);

	if (!sourceOutValue || !destinationInValue) return false;

	if (sourceOutValue->get()->GetType() != destinationInValue->get()->GetType()) return false;

	bool hasBeenConnected = ConnectValues(sourceOutValue, destinationInValue);

	if (hasBeenConnected)
	{
		destination->AddChild(source); 
		source->OnConnect(GraphNode::Connection::Type::Output, sourceOutValue->get()->GetType(), sourceEndpoint);
		destination->OnConnect(GraphNode::Connection::Type::Input, destinationInValue->get()->GetType(), destinationEndpoint);
	}
	
	return hasBeenConnected;
}

bool shade::animation::AnimationGraph::RemoveConnection(GraphNode::NodeIDX sourceNode, GraphNode::EndpointIDX sourceEndpoint, GraphNode::NodeIDX destinationNode, GraphNode::EndpointIDX destinationEndpoint)
{
	return false;
}

bool shade::animation::AnimationGraph::AddRootConnection(GraphNode::NodeIDX sourceNode, GraphNode::EndpointIDX sourceEndpoint)
{
	auto source	= FindNode(sourceNode);
	if (!source) return false;

	auto sourceOutValue = source->__GetEndpoint<GraphNode::Connection::Output>(sourceEndpoint);

	if (!sourceOutValue || sourceOutValue->get()->GetType() != NodeValueType::Pose) return false;

	auto rootNode = m_RootNode->__GetEndpoint<GraphNode::Connection::Input>(0);
	bool hasBeenConnected = ConnectValues(sourceOutValue, rootNode);

	if (hasBeenConnected)
	{
		m_RootNode->AddChild(source); // Can create special for OutputPoseNode SetChild becase there only one child !
		source->OnConnect(GraphNode::Connection::Type::Output, NodeValueType::Pose, sourceEndpoint);
		m_RootNode->OnConnect(GraphNode::Connection::Type::Input, NodeValueType::Pose, 0);
	}
	
	return hasBeenConnected;
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

bool shade::animation::AnimationGraph::ConnectValues(const std::shared_ptr<NodeValue>* sourceEndpoint, std::shared_ptr<NodeValue>* destinationEndpoint)
{
	if (!sourceEndpoint || !destinationEndpoint)
		return false;

	*destinationEndpoint = *sourceEndpoint;

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
