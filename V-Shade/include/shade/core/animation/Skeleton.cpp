#include "shade_pch.h"
#include "Skeleton.h"

shade::Skeleton::Skeleton(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
{
    const std::string filePath = assetData->GetAttribute<std::string>("Path");

    shade::File file(filePath, shade::File::In, "@s_skeleton", shade::File::VERSION(0, 0, 1));
    if (!file.IsOpen())
        SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath)
    else
    {
        file.Read(*this);
        file.CloseFile();
    }
}

shade::Skeleton* shade::Skeleton::Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour)
{
    return new Skeleton(assetData, lifeTime, behaviour);
}

shade::AssetMeta::Type shade::Skeleton::GetAssetStaticType()
{
    return AssetMeta::Type::Skeleton;
}

shade::AssetMeta::Type shade::Skeleton::GetAssetType() const
{
    return GetAssetStaticType();
}

shade::SharedPointer<shade::Skeleton> shade::Skeleton::CreateEXP()
{
    return SharedPointer<Skeleton>::Create();
}

shade::SharedPointer<shade::Skeleton::BoneNode>& shade::Skeleton::AddBone(const std::string& name, const glm::mat4& transform, const glm::mat4& inverseBindPose)
{
    auto& bone = m_BoneNodes.emplace(name, SharedPointer<BoneNode>::Create(m_BoneNodes.size(), name)).first->second;

    bone->InverseBindPose = inverseBindPose;

    if (m_BoneNodes.size() == 1) m_RootNode = bone;

    math::DecomposeMatrix(transform, bone->Translation, bone->Rotation, bone->Scale);
    return bone;
}

const shade::SharedPointer<shade::Skeleton::BoneNode>& shade::Skeleton::GetBone(const std::string& name) const
{
    const auto bone = m_BoneNodes.find(name);
    if (bone != m_BoneNodes.end())
        return bone->second;
    else
        return nullptr;
}

const shade::SharedPointer<shade::Skeleton::BoneNode>& shade::Skeleton::GetBone(std::size_t id) const
{
    assert(false);
    return nullptr;
}

shade::SharedPointer<shade::Skeleton::BoneArmature>& shade::Skeleton::AddArmature(const glm::mat4& transform)
{
    m_Armature = SharedPointer<BoneArmature>::Create(transform);
    return m_Armature;
}

const shade::SharedPointer<shade::Skeleton::BoneArmature>& shade::Skeleton::GetArmature() const
{
    return m_Armature;
}

const shade::SharedPointer<shade::Skeleton::BoneNode>& shade::Skeleton::GetRootNode() const
{
    return m_RootNode;
}
const shade::Skeleton::BoneNodes& shade::Skeleton::GetBones() const
{
    return m_BoneNodes;
}

std::size_t shade::Skeleton::Serialize(std::ostream& stream) const
{
    assert(false);
    return std::size_t();
}

std::size_t shade::Skeleton::Deserialize(std::istream& stream)
{
    assert(false);
    return std::size_t();
}
