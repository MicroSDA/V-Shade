#include "shade_pch.h"
#include "Skeleton.h"


shade::SharedPointer<shade::Skeleton::BoneNode>& shade::Skeleton::AddBone(const std::string& name, const glm::mat4& transform)
{
    auto& bone = m_BoneNodes.emplace(name, SharedPointer<BoneNode>::Create(name)).first->second;
    if (m_BoneNodes.size() == 1) m_RootBone = bone;

    math::DecomposeMatrix(transform, bone->Translation, bone->Rotation, bone->Scale);
    return bone;
}

std::uint32_t shade::Skeleton::AddBone(const std::string& name, std::uint32_t parentID, const glm::mat4& transform)
{
   std::uint32_t index = m_BonesData.Names.size();
   m_BonesData.Names.emplace_back(name);
   m_BonesData.ParentIDs.emplace_back(parentID);
   math::DecomposeMatrix(transform, m_BonesData.Translations.emplace_back(), m_BonesData.Rotations.emplace_back(), m_BonesData.Scales.emplace_back());
   return index;
}

std::uint32_t shade::Skeleton::GetBoneID(const std::string& name) const
{
    const auto iterator = std::find(std::begin(m_BonesData.Names), std::end(m_BonesData.Names),
        name);

    if (iterator != m_BonesData.Names.end())
        return std::distance(m_BonesData.Names.begin(), iterator);
    else
        return NULL_BONE_ID;
}

const std::string& shade::Skeleton::GetBoneName(std::size_t id) const
{
    assert(id < m_BonesData.Names.size() && "Bone id is out of range!");
    return m_BonesData.Names[id];
}

const shade::Skeleton::BonesData& shade::Skeleton::GetSkeletonBones() const
{
    return m_BonesData;
}

shade::Skeleton::BonesData& shade::Skeleton::GetSkeletonBones()
{
    return m_BonesData;
}

std::size_t shade::Skeleton::GetBonesCount() const
{
    return m_BonesData.Names.size();
}

shade::Skeleton::BoneData shade::Skeleton::GetBoneData(const std::string& name)  const
{
    const auto id = GetBoneID(name);

    assert((id != NULL_BONE_ID && id < m_BonesData.Names.size()) && "Couldn't find bone!");

    if(id != NULL_BONE_ID)
        return { m_BonesData.ParentIDs[id], m_BonesData.Translations[id], m_BonesData.Rotations[id], m_BonesData.Scales[id]};
}

shade::Skeleton::BoneData  shade::Skeleton::GetBoneData(std::size_t id) const
{
    assert((id != NULL_BONE_ID && id < m_BonesData.Names.size()) && "Couldn't find bone!");
    return { m_BonesData.ParentIDs[id], m_BonesData.Translations[id], m_BonesData.Rotations[id], m_BonesData.Scales[id] };
}
