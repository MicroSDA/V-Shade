#include "shade_pch.h"
#include "MeshShape.h"
#include <shade/core/physics/algo/GJK.h>

shade::physic::CollisionShape::Manifold shade::physic::MeshShape::TestCollision(const glm::mat<4,4, scalar_t>& transform, const CollisionShape& otherShape, const glm::mat<4, 4, scalar_t>& otherTransform) const
{
	auto [hasCollision, simplex] = algo::GJK(*this, transform, otherShape, otherTransform);
	if (hasCollision)
		return algo::EPA(simplex, *this, transform, otherShape, otherTransform);
	else
		return { false }; // Return no collision
}

glm::vec<3, shade::physic::scalar_t> shade::physic::MeshShape::FindFurthestPointWorld(const glm::mat<4, 4, scalar_t>& transform, const glm::vec<3, scalar_t>& direction) const
{
	glm::vec<3, shade::physic::scalar_t> maxPoint(0.f);
	float     maxDistance = -FLT_MAX;
	for (const glm::vec<3, shade::physic::scalar_t>& v : m_Vertices)
	{
		// We need non-transposed matrix but glm allready use transposed matrices, so that's why we are transposing matrix back
		const glm::vec<3, scalar_t> vertex = glm::vec<3, scalar_t>(glm::vec<4, scalar_t>(v, 1.0) * glm::transpose(transform));
		const float distance = glm::dot(vertex, direction);
		if (distance > maxDistance) 
		{
			maxDistance = distance;
			maxPoint = vertex;
		}
	}
	return maxPoint;
}


void shade::physic::MeshShape::AddVertex(const glm::vec<3, scalar_t>& vertex)
{
	m_Vertices.emplace_back(vertex);
}

void shade::physic::MeshShape::AddVertices(const std::vector<glm::vec<3, scalar_t>>& vertices)
{
	std::copy(vertices.begin(), vertices.end(), std::back_inserter(m_Vertices));
}

void shade::physic::MeshShape::SetVertices(std::vector<glm::vec<3, scalar_t>>& vertices)
{
	m_Vertices = std::move(vertices);
}

const std::vector<glm::vec<3, shade::physic::scalar_t>>& shade::physic::MeshShape::GetVertices() const
{
	return m_Vertices;
}

std::vector<glm::vec<3, shade::physic::scalar_t>>& shade::physic::MeshShape::GetVertices()
{
	return m_Vertices;
}

void shade::physic::MeshShape::Serialize(std::ostream& stream) const
{
	serialize::Serializer::Serialize(stream, static_cast<std::uint32_t>(GetShape()));
	serialize::Serializer::Serialize(stream, static_cast<std::uint32_t>(m_Vertices.size()));

	for (const auto& vertex : m_Vertices)
	{
		serialize::Serializer::Serialize(stream, static_cast<float>(vertex.x));
		serialize::Serializer::Serialize(stream, static_cast<float>(vertex.y));
		serialize::Serializer::Serialize(stream, static_cast<float>(vertex.z));
	}
}

void shade::physic::MeshShape::Deserialize(std::istream& stream)
{
	
}
