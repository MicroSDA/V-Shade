#include "shade_pch.h"
#include "Skeleton.h"

shade::Skeleton::Skeleton(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
{
    const std::string filePath = assetData->GetAttribute<std::string>("Path");

    shade::File file(filePath, shade::File::In, "@s_skel", shade::File::VERSION(0, 0, 1));
    if (!file.IsOpen())
        SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath)
    else
    {
        file.Read(*this);
        file.CloseFile();
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

std::size_t DeserializeNode(std::istream& stream, shade::Skeleton& skeleton, shade::Skeleton::BoneNode** child = nullptr);
std::size_t DeserializeChildren(std::istream& stream, shade::Skeleton& skeleton, std::vector<shade::Skeleton::BoneNode*>& children);

std::size_t SerializeNode(std::ostream& stream, const shade::Skeleton& skeleton, const shade::Skeleton::BoneNode& node);
std::size_t SerializeChildren(std::ostream& stream, const shade::Skeleton& skeleton, const std::vector<shade::Skeleton::BoneNode*>& children);

// TIP: Not tested 
std::size_t SerializeNode(std::ostream& stream, const shade::Skeleton& skeleton, const shade::Skeleton::BoneNode& node)
{
    std::size_t size = shade::Serializer::Serialize<std::uint32_t>(stream, node.ID);
    size += shade::Serializer::Serialize<std::string>(stream, node.Name);
    size += shade::Serializer::Serialize<glm::vec3>(stream, node.Translation);
    size += shade::Serializer::Serialize<glm::quat>(stream, node.Rotation);
    size += shade::Serializer::Serialize<glm::vec3>(stream, node.Scale);
    size += shade::Serializer::Serialize<glm::mat4>(stream, node.InverseBindPose);

    size += SerializeChildren(stream, skeleton, node.Children);
    return size;
}
// TIP: Not tested 
std::size_t SerializeChildren(std::ostream& stream, const shade::Skeleton& skeleton, const std::vector<shade::Skeleton::BoneNode*>& children)
{
    std::uint32_t count = children.size();
    if (count == UINT32_MAX)
        throw std::out_of_range(std::format("Incorrect array size = {}", count));

    std::uint32_t totalSize = shade::Serializer::Serialize<std::uint32_t>(stream, count);

    for (const auto& node : children)
        totalSize += SerializeNode(stream, skeleton, *node);

    return totalSize;
}

std::size_t DeserializeChildren(std::istream& stream, shade::Skeleton& skeleton, std::vector<shade::Skeleton::BoneNode*>& children)
{
    std::uint32_t count = 0;
    // Read size first.
    std::size_t totalSize = shade::Serializer::Deserialize<std::uint32_t>(stream, count);
    if (count == UINT32_MAX)
        throw std::out_of_range(std::format("Incorrect array size = {}", count));

    children.resize(count);

    for (auto& child : children)
    {
        totalSize += DeserializeNode(stream, skeleton, &child);
    }

    return totalSize;
}

std::size_t DeserializeNode(std::istream& stream, shade::Skeleton& skeleton, shade::Skeleton::BoneNode** child)
{
    shade::Skeleton::BoneNode _node;

    std::size_t size = shade::Serializer::Deserialize<std::uint32_t>(stream, _node.ID);
    size += shade::Serializer::Deserialize<std::string>(stream, _node.Name);
    size += shade::Serializer::Deserialize<glm::vec3>(stream, _node.Translation);
    size += shade::Serializer::Deserialize<glm::quat>(stream, _node.Rotation);
    size += shade::Serializer::Deserialize<glm::vec3>(stream, _node.Scale);
    size += shade::Serializer::Deserialize<glm::mat4>(stream, _node.InverseBindPose);

    auto& node = skeleton.AddNode(_node);

    if (child) *child = &node;

    size += DeserializeChildren(stream, skeleton, node.Children);

    return size;
}

std::size_t shade::Skeleton::Deserialize(std::istream& stream)
{
    std::size_t totalSize = Serializer::Deserialize<glm::mat4>(stream, m_Armature.Transform);
    totalSize += DeserializeNode(stream, *this);

    return totalSize;
}

std::size_t shade::Skeleton::Serialize(std::ostream& stream) const
{
    std::size_t size = Serializer::Serialize<glm::mat4>(stream, m_Armature.Transform);
    size += SerializeNode(stream, *this, *m_RootNode);
    return size;
}
