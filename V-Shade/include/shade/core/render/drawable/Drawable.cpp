#include "shade_pch.h"
#include "Drawable.h"

void shade::Drawable::AddVertex(const Vertex& vertex, std::size_t lodLevel)
{
    GetLod(lodLevel).Vertices.emplace_back(vertex);
}

void shade::Drawable::AddIndex(std::uint32_t index, std::size_t lodLevel)
{
    GetLod(lodLevel).Indices.emplace_back(index);
}

void shade::Drawable::AddVertices(const Vertices& vertices, std::size_t lodLevel)
{
    std::copy(vertices.begin(), vertices.end(), std::back_inserter(GetLod(lodLevel).Vertices));
}

void shade::Drawable::AddIndices(const Indices& indices, std::size_t lodLevel)
{
    std::copy(indices.begin(), indices.end(), std::back_inserter(GetLod(lodLevel).Indices));
}

void shade::Drawable::SetVertices(Vertices& vertices, std::size_t lodLevel)
{
    GetLod(lodLevel).Vertices = std::move(vertices);
}

void shade::Drawable::SetVertices(std::vector<glm::vec3>& vertices, std::size_t lodLevel)
{
    for(const glm::vec3& vertex: vertices)
        GetLod(lodLevel).Vertices.emplace_back(vertex);
}

void shade::Drawable::SetIndices(Indices& indices, std::size_t lodLevel)
{
    GetLod(lodLevel).Indices = std::move(indices);
}

const shade::Drawable::Lod& shade::Drawable::GetLod(std::size_t level) const
{
    assert(level < m_Lods.size() && "Trying to get lod which greater than 10.");
    return m_Lods[level];
}

shade::Drawable::Lod& shade::Drawable::GetLod(std::size_t level)
{
    assert(level < m_Lods.size() && "Trying to get lod which greater than 10.");
    return m_Lods[level];
}

const std::array<shade::Drawable::Lod, 10>& shade::Drawable::GetLods() const
{
    return m_Lods;
}

std::array<shade::Drawable::Lod, 10>& shade::Drawable::GetLods()
{
    return m_Lods;
}
void shade::Drawable::RecalculateLod(std::size_t level, std::size_t faces)
{
    Lod highPolyLod = GetLod(0);
    algo::SimplifyMesh(highPolyLod.Vertices, highPolyLod.Indices, faces);
    GetLod(level) = highPolyLod;
}
void shade::Drawable::RecalculateAllLods(std::size_t levelCount, std::size_t maxFaces, std::size_t minFaces, float splitLambda)
{
    std::vector<std::size_t> faceCounts = shade::algo::CalculateFaceCountLodLevel(Drawable::MAX_LEVEL_OF_DETAIL, maxFaces, minFaces, splitLambda);

    Lod highPolyLod = GetLod(0);

    for (std::size_t i = 1; i < Drawable::MAX_LEVEL_OF_DETAIL; i++)
    {
        Lod simlyfied = highPolyLod;
        algo::SimplifyMesh(simlyfied.Vertices, simlyfied.Indices, faceCounts[i]);
        GetLod(i) = simlyfied;
    }
}
const shade::Vertices& shade::Drawable::GetVertices() const
{
    return GetLod(0).Vertices;
}

const shade::Indices& shade::Drawable::GetIndices() const
{
    return GetLod(0).Indices;
}

shade::Vertices& shade::Drawable::GetVertices()
{
    return GetLod(0).Vertices;
}

shade::Indices& shade::Drawable::GetIndices()
{
    return GetLod(0).Indices;
}

void shade::Drawable::SetMaterial(Asset<Material> material)
{
    m_Material = material;
}

const shade::Asset<shade::Material>& shade::Drawable::GetMaterial() const
{
    return m_Material;
}

shade::Asset<shade::Material>& shade::Drawable::GetMaterial()
{
    return m_Material;
}

void shade::Drawable::SetMinHalfExt(const glm::vec3& ext)
{
    m_MinHalfExt = ext;
}

void shade::Drawable::SetMaxHalfExt(const glm::vec3& ext)
{
    m_MaxHalfExt = ext;
}

const glm::vec3& shade::Drawable::GetMinHalfExt() const
{
    return m_MinHalfExt;
}

const glm::vec3& shade::Drawable::GetMaxHalfExt() const
{
    return m_MaxHalfExt;
}
