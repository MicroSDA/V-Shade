#include "shade_pch.h"
#include "Box.h"

shade::SharedPointer<shade::Box> shade::Box::Create(const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt)
{
    return SharedPointer<Box>::Create(minHalfExt, maxHalfExt);
}

const glm::vec3& shade::Box::GetMinHalfExt() const
{
    return m_MinHalfExt;
}

const glm::vec3& shade::Box::GetMaxHalfExt() const
{
    return m_MaxHalfExt;
}

shade::Box::Box(const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt) :
    m_MinHalfExt(minHalfExt), m_MaxHalfExt(maxHalfExt)
{
    // Lines
    AddVertex({ .Position = {m_MinHalfExt.x, m_MinHalfExt.y, m_MinHalfExt.z} });
    AddVertex({ .Position = {m_MaxHalfExt.x, m_MinHalfExt.y, m_MinHalfExt.z} });
    AddVertex({ .Position = {m_MaxHalfExt.x, m_MaxHalfExt.y, m_MinHalfExt.z} });
    AddVertex({ .Position = {m_MinHalfExt.x, m_MaxHalfExt.y, m_MinHalfExt.z} });
    AddVertex({ .Position = {m_MinHalfExt.x, m_MinHalfExt.y, m_MaxHalfExt.z} });
    AddVertex({ .Position = {m_MaxHalfExt.x, m_MinHalfExt.y, m_MaxHalfExt.z} });
    AddVertex({ .Position = {m_MaxHalfExt.x, m_MaxHalfExt.y, m_MaxHalfExt.z} });
    AddVertex({ .Position = {m_MinHalfExt.x, m_MaxHalfExt.y, m_MaxHalfExt.z} });

    AddIndices(Indices
        {
            0, 1, 1, 2, 2, 3, 3, 0, 
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7
        });
}
