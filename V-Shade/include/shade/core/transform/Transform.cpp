#include "shade_pch.h"
#include "Transform.h"

shade::Transform::Transform():
	m_Possition(0.0f, 0.0f, 0.0f),
	m_Rotation(0.0f, 0.0f, 0.0f),
	m_Scale(1.0f, 1.0f, 1.0f)
{
}

shade::Transform::Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale):
	m_Possition(position), m_Rotation(rotation), m_Scale(scale)
{
}

void shade::Transform::SetDirection(const glm::vec3& direction)
{
	//m_Rotation = direction * glm::vec3(0, 0, 1);
}

std::size_t shade::Transform::Serialize(std::ostream& stream) const
{
	std::uint32_t size = Serializer::Serialize(stream, m_Possition);
	size += Serializer::Serialize(stream, m_Rotation);
	size += Serializer::Serialize(stream, m_Scale);
	return size;
}

std::size_t shade::Transform::Deserialize(std::istream& stream)
{
	std::uint32_t size = Serializer::Deserialize(stream, m_Possition);
	size += Serializer::Deserialize(stream, m_Rotation);
	size += Serializer::Deserialize(stream, m_Scale);
	return size;
}
