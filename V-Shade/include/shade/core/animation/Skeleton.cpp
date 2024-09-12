#include "shade_pch.h"
#include "Skeleton.h"

shade::Skeleton::Skeleton(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
{
	const std::string filePath = assetData->GetAttribute<std::string>("Path");

	if(file::File file = file::FileManager::LoadFile(filePath, "@s_skel"))
	{
		file.Read(*this);
	}
	else
	{
		SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath)
	}
}

shade::Skeleton::BoneNode& shade::Skeleton::AddBone(const std::string& name, const glm::mat4& transform, const glm::mat4& inverseBindPose)
{
	auto& bone = m_BoneNodes.emplace(name, std::move(Skeleton::BoneNode(m_BoneNodes.size(), name))).first->second;

	bone.InverseBindPose = inverseBindPose;

	if (m_BoneNodes.size() == 1) m_RootNode = &bone;

	math::DecomposeMatrix(transform, bone.Translation, bone.Rotation, bone.Scale);
	return bone;
}

shade::Skeleton::BoneNode& shade::Skeleton::AddNode(const shade::Skeleton::BoneNode& node)
{
	auto& _node = m_BoneNodes.emplace(node.Name, node).first->second;
	if (m_BoneNodes.size() == 1)
		m_RootNode = &_node;

	return _node;
}

const shade::Skeleton::BoneNode* shade::Skeleton::GetBone(const std::string& name) const
{
	const auto bone = m_BoneNodes.find(name);
	if (bone != m_BoneNodes.end())
		return &bone->second;
	else
		return nullptr;
}

const shade::Skeleton::BoneNode* shade::Skeleton::GetBone(std::size_t id) const
{
	for (const auto& [name, node] : m_BoneNodes)
		if (node.ID == id)
			return &node;

	return nullptr;
}

shade::Skeleton::BoneArmature& shade::Skeleton::AddArmature(const glm::mat4& transform)
{
	m_Armature = BoneArmature(transform);
	return m_Armature;
}

const shade::Skeleton::BoneArmature& shade::Skeleton::GetArmature() const
{
	return m_Armature;
}

const shade::Skeleton::BoneNode* shade::Skeleton::GetRootNode() const
{
	return m_RootNode;
}
const shade::Skeleton::BoneNodes& shade::Skeleton::GetBones() const
{
	return m_BoneNodes;
}

void DeserializeNode(std::istream& stream, shade::Skeleton& skeleton, shade::Skeleton::BoneNode** child = nullptr);
void DeserializeChildren(std::istream& stream, shade::Skeleton& skeleton, std::vector<shade::Skeleton::BoneNode*>& children);

void SerializeNode(std::ostream& stream, const shade::Skeleton& skeleton, const shade::Skeleton::BoneNode& node);
void SerializeChildren(std::ostream& stream, const shade::Skeleton& skeleton, const std::vector<shade::Skeleton::BoneNode*>& children);

// TIP: Not tested 
void SerializeNode(std::ostream& stream, const shade::Skeleton& skeleton, const shade::Skeleton::BoneNode& node)
{
	shade::serialize::Serializer::Serialize<std::uint32_t>(stream, node.ID);
	shade::serialize::Serializer::Serialize<std::string>(stream, node.Name);
	shade::serialize::Serializer::Serialize<glm::vec3>(stream, node.Translation);
	shade::serialize::Serializer::Serialize<glm::quat>(stream, node.Rotation);
	shade::serialize::Serializer::Serialize<glm::vec3>(stream, node.Scale);
	shade::serialize::Serializer::Serialize<glm::mat4>(stream, node.InverseBindPose);

	SerializeChildren(stream, skeleton, node.Children);
}
// TIP: Not tested 
void SerializeChildren(std::ostream& stream, const shade::Skeleton& skeleton, const std::vector<shade::Skeleton::BoneNode*>& children)
{
	std::uint32_t count = children.size();
	if (count == UINT32_MAX)
		throw std::out_of_range(std::format("Incorrect array size = {}", count));

	shade::serialize::Serializer::Serialize<std::uint32_t>(stream, count);

	for (const auto& node : children)
		SerializeNode(stream, skeleton, *node);
}

void DeserializeChildren(std::istream& stream, shade::Skeleton& skeleton, std::vector<shade::Skeleton::BoneNode*>& children)
{
	std::uint32_t count = 0;
	// Read size first.
	shade::serialize::Serializer::Deserialize<std::uint32_t>(stream, count);
	if (count == UINT32_MAX)
		throw std::out_of_range(std::format("Incorrect array size = {}", count));

	children.resize(count);

	for (auto& child : children)
		DeserializeNode(stream, skeleton, &child);
}

void DeserializeNode(std::istream& stream, shade::Skeleton& skeleton, shade::Skeleton::BoneNode** child)
{
	shade::Skeleton::BoneNode _node;

	shade::serialize::Serializer::Deserialize<std::uint32_t>(stream, _node.ID);
	shade::serialize::Serializer::Deserialize<std::string>(stream, _node.Name);
	shade::serialize::Serializer::Deserialize<glm::vec3>(stream, _node.Translation);
	shade::serialize::Serializer::Deserialize<glm::quat>(stream, _node.Rotation);
	shade::serialize::Serializer::Deserialize<glm::vec3>(stream, _node.Scale);
	shade::serialize::Serializer::Deserialize<glm::mat4>(stream, _node.InverseBindPose);

	auto& node = skeleton.AddNode(_node);

	if (child) *child = &node;

	DeserializeChildren(stream, skeleton, node.Children);
}

void shade::Skeleton::Deserialize(std::istream& stream)
{
	serialize::Serializer::Deserialize<glm::mat4>(stream, m_Armature.Transform);
	DeserializeNode(stream, *this);
}

void shade::Skeleton::Serialize(std::ostream& stream) const
{
	serialize::Serializer::Serialize<glm::mat4>(stream, m_Armature.Transform);
	SerializeNode(stream, *this, *m_RootNode);
}
