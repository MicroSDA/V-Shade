#include "shade_pch.h"
#include "Skeleton.h"


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