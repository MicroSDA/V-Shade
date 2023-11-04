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

// TODO: maby need to set viewProjection and recalculate it, check !
void shade::Camera::SetVeiw(const glm::mat4& view)
{
	glm::mat4 matrix = GetProjection() * glm::inverse(view);
	//glm::vec2 right	= glm::vec3(matrix[0][0], matrix[1][0], matrix[2][0]);
	m_Forward = glm::vec3(matrix[0][2], matrix[1][2], matrix[2][2]);
	m_Up = glm::vec3(matrix[0][1], matrix[1][1], matrix[2][1]);
	m_Position = glm::vec3(view[3][0], view[3][1], view[3][2]);
}

void shade::Camera::RotateY(float angle)
{
	/* With counterclockwise issue */
	// glm::mat4 rotation = glm::rotate(angle, UP);
	/* Without counterclockwise issue */
	glm::mat4 rotation = glm::rotate(angle, glm::dot(UP, m_Up) < 0.f ? -UP : UP);
	m_Forward = glm::vec3(glm::normalize(rotation * glm::vec4(m_Forward, 0.f)));
	m_Up = glm::vec3(glm::normalize(rotation * glm::vec4(m_Up, 0.f)));
}
void shade::Camera::RotateX(float angle)
{
	glm::vec3 right = glm::normalize(glm::cross(m_Up, m_Forward));
	m_Forward = glm::normalize(glm::rotate(-angle, right) * glm::vec4(m_Forward, 0.f));
	m_Up = glm::normalize(glm::cross(m_Forward, right));
}
void shade::Camera::RotateZ(float angle)
{
	m_Up = glm::normalize(glm::rotate(-angle, m_Forward) * glm::vec4(m_Up, 0.f));
}

void shade::Camera::Rotate(float x, float y, float z)
{
	RotateY(-x);
	RotateX(-y);
	RotateZ(z);
}

void shade::Camera::Rotate(const glm::vec3& angle)
{
	RotateY(-angle.x);
	RotateX(-angle.y);
	RotateZ(angle.z);
}

void shade::Camera::Resize(float aspect)
{
	// TODO Zoom
	// TODO m_Fov has to be in radians ?
	if (aspect)
	{
		m_Aspect = aspect;
		RecalculatePerpective();
	}
	else
	{
		RecalculatePerpective();
	}	
}

std::size_t shade::Camera::SerializeAsComponent(std::ostream& stream) const
{
	std::uint32_t size = 0u;
	size += Serializer::Serialize(stream, m_Fov);
	size += Serializer::Serialize(stream, m_Aspect);
	size += Serializer::Serialize(stream, m_zNear);
	size += Serializer::Serialize(stream, m_zFar);
	size += Serializer::Serialize(stream, m_Position);
	size += Serializer::Serialize(stream, m_Forward);
	size += Serializer::Serialize(stream, m_Up);
	size += Serializer::Serialize(stream, m_IsPrimary);

	return size;
}

std::size_t shade::Camera::DeserializeAsComponent(std::istream& stream)
{
	std::uint32_t size = 0u;
	size += Serializer::Deserialize(stream, m_Fov);
	size += Serializer::Deserialize(stream, m_Aspect);
	size += Serializer::Deserialize(stream, m_zNear);
	size += Serializer::Deserialize(stream, m_zFar);
	size += Serializer::Deserialize(stream, m_Position);
	size += Serializer::Deserialize(stream, m_Forward);
	size += Serializer::Deserialize(stream, m_Up);
	size += Serializer::Deserialize(stream, m_IsPrimary);

	return size;
}