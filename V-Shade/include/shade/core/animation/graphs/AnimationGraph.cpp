#include "shade_pch.h"
#include "AnimationGraph.h"

//shade::AssetMeta::Type shade::animation::AnimationGraph::GetAssetStaticType()
//{
//	// TODO: Change !!
//	return AssetMeta::Type::Asset;
//}
//
//shade::AssetMeta::Type shade::animation::AnimationGraph::GetAssetType() const
//{
//	return GetAssetStaticType();
//}
//
//shade::animation::AnimationGraph::AnimationGraph(const Asset<Skeleton>& skeleton, ecs::Entity entity)
//{
//	m_AnimationController = AnimationController::Create();
//	m_GraphContext.Controller = m_AnimationController;
//	m_GraphContext.Skeleton = skeleton;
//
//	InitializeGraph(&m_GraphContext);
//
//	// Change it !!
//	m_RootNode = SharedPointer<OutputPoseNode>::Create(~0, &m_GraphContext);
//}
//
//shade::Asset<shade::animation::AnimationGraph> shade::animation::AnimationGraph::CreateEXP(const Asset<Skeleton>& skeleton, ecs::Entity entity)
//{
//	return SharedPointer<AnimationGraph>::Create(skeleton, entity);
//}
//
//void shade::animation::AnimationGraph::SetSkeleton(const Asset<Skeleton>& skeleton)
//{
//	m_GraphContext.Skeleton = skeleton;
//}
//
//const shade::Asset<shade::Skeleton>& shade::animation::AnimationGraph::GetSkeleton() const
//{
//	return m_GraphContext.Skeleton;
//}
//
//
//shade::SharedPointer<shade::animation::OutputPoseNode> shade::animation::AnimationGraph::GetOutputPoseNode()
//{
//	return GetRootNode();
//}
//
//shade::animation::AnimationGraph::AnimationGraph(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
//{
//	assert(false && "How about Context !");
//	m_AnimationController = AnimationController::Create();
//	m_RootNode = SharedPointer<OutputPoseNode>::Create(~0, &m_GraphContext);
//	m_AnimationController = AnimationController::Create();
//}
//
//shade::animation::AnimationGraph* shade::animation::AnimationGraph::Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour)
//{
//	return new AnimationGraph(assetData, lifeTime, behaviour);
//}
//
//std::size_t shade::animation::AnimationGraph::Serialize(std::ostream& stream) const
//{
//	return std::size_t();
//}
//
//std::size_t shade::animation::AnimationGraph::Deserialize(std::istream& stream)
//{
//	return std::size_t();
//}

shade::animation::AnimationGraph::AnimationGraph(graphs::GraphContext* context) : BaseNode(context, 0u)
{
	context->GetNodes().emplace(this, graphs::NodesPack{});

	m_OutPutPoseNode = CreateNode<OutputPoseNode>();
	SetRootNode(m_OutPutPoseNode);
}

void shade::animation::AnimationGraph::ProcessGraph(const shade::FrameTimer& deltaTime)
{
	ProcessBranch(deltaTime);
}

const shade::animation::Pose* shade::animation::AnimationGraph::GetOutPutPose() const
{
	return m_OutPutPoseNode->GetFinalPose();
}

void shade::animation::AnimationGraph::Evaluate(const FrameTimer& deltaTime)
{
	GetRootNode()->ProcessBranch(deltaTime);
}

std::size_t shade::animation::AnimationGraph::Serialize(std::ostream& stream) const
{
	return std::size_t();
}

std::size_t shade::animation::AnimationGraph::Deserialize(std::istream& stream)
{
	return std::size_t();
}

shade::graphs::BaseNode* shade::animation::AnimationGraph::GetInputNode(const std::string& name)
{
	auto node = m_InputNodes.find(name);
	assert(node != m_InputNodes.end() && "Input node has not been found!");
	return node->second;
}

const shade::graphs::BaseNode* shade::animation::AnimationGraph::GetInputNode(const std::string& name) const
{
	auto node = m_InputNodes.find(name);
	assert(node != m_InputNodes.end() && "Input node has not been found!");
	return node->second;
}
