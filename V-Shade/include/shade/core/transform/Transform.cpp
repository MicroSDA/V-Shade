#include "shade_pch.h"
#include "Transform.h"

shade::Transform::Transform():
	m_Position(0.0f, 0.0f, 0.0f),
	m_RotationQuat(glm::identity<glm::quat>()),
	m_Scale(1.0f, 1.0f, 1.0f)
{
}

void shade::Transform::Serialize(std::ostream& stream) const
{
	serialize::Serializer::Serialize(stream, m_Position);
	serialize::Serializer::Serialize(stream, m_RotationQuat);
	serialize::Serializer::Serialize(stream, m_Scale);
}

void shade::Transform::Deserialize(std::istream& stream)
{
	serialize::Serializer::Deserialize(stream, m_Position);
	serialize::Serializer::Deserialize(stream, m_RotationQuat);
	serialize::Serializer::Deserialize(stream, m_Scale);
}
