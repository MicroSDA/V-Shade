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

std::size_t shade::animation::AnimationGraph::Serialize(std::ostream& stream) const
{

	//1. ������� ���������� ���
	//2. ���������������� ��� �������� ������� ���� �� ��� �� ����� ���������
	//3. ������� ���������� �������, ����� ��� ���, ����� ����������
	//4. �������� �������� ���������� ��� ������������� ��� �� ���� ����� ����� ������������� ���������
	//5. �������� ������� � ������������� ����� ����� � �� ���������� � ���������, ������ ���� ��������� ������� ��� ��� �����
	//6. ������� ���� ��� ������� ����, ����� ������� ��� �������� ������������
	// How many childrens 
	Serializer::Serialize(stream, std::uint32_t(GetInternalNodes().size()));

	for (const auto node : GetInternalNodes())
	{
		/*Serializer::Serialize(stream, std::uint32_t(lod.Vertices.size()));
		Serializer::Serialize(stream, std::uint32_t(lod.Indices.size()));
		Serializer::Serialize(stream, std::uint32_t(lod.Bones.size()));*/
	}

	for (std::uint32_t i = 0; i < GetInternalNodes().size(); ++i)
	{
		GetInternalNodes();
	}
	

	GetGraphContext()->GetConnections(this);
	return std::size_t();
}

std::size_t shade::animation::AnimationGraph::Deserialize(std::istream& stream)
{
	return std::size_t();
}
