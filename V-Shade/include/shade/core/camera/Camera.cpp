#include "shade_pch.h"
#include "Camera.h"

shade::Camera::Camera():
	m_Fov(45.0f), m_Aspect(1), // Be aware about aspect 1
	m_zNear(0.1f), m_zFar(1000.0f)
{
	RecalculatePerpective();

	m_Position = glm::vec3(0, 0, -10);
	m_Forward = glm::vec3(0, 0, 1); // - Z
	m_Up = glm::vec3(0, 1, 0);
}

shade::Camera::Camera(const glm::vec3& position, float fov, float aspect, float zNear, float zFar)
{
	m_Fov = fov;
	m_Aspect = aspect;
	m_zNear = zNear;
	m_zFar = zFar;
	m_Position = position;
	m_Forward = glm::vec3(0, 0, 1);
	m_Up = glm::vec3(0, 1, 0);

	RecalculatePerpective();
}

void shade::Camera::Serialize(std::ostream& stream) const
{
	serialize::Serializer::Serialize(stream, m_Fov);
	serialize::Serializer::Serialize(stream, m_Aspect);
	serialize::Serializer::Serialize(stream, m_zNear);
	serialize::Serializer::Serialize(stream, m_zFar);
	serialize::Serializer::Serialize(stream, m_Position);
	serialize::Serializer::Serialize(stream, m_Forward);
	serialize::Serializer::Serialize(stream, m_Up);
	serialize::Serializer::Serialize(stream, m_IsPrimary);
}

void shade::Camera::Deserialize(std::istream& stream)
{
	serialize::Serializer::Deserialize(stream, m_Fov);
	serialize::Serializer::Deserialize(stream, m_Aspect);
	serialize::Serializer::Deserialize(stream, m_zNear);
	serialize::Serializer::Deserialize(stream, m_zFar);
	serialize::Serializer::Deserialize(stream, m_Position);
	serialize::Serializer::Deserialize(stream, m_Forward);
	serialize::Serializer::Deserialize(stream, m_Up);
	serialize::Serializer::Deserialize(stream, m_IsPrimary);
}