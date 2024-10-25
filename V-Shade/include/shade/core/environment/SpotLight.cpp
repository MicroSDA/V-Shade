#include "shade_pch.h"
#include "SpotLight.h"

shade::SpotLight::SpotLight()
{
	++m_sTotalCount;
}

shade::SpotLight::~SpotLight()
{
	--m_sTotalCount;
}

shade::SpotLight::RenderData shade::SpotLight::GetRenderData(const glm::vec3& position, const glm::vec3& derection, const SharedPointer<Camera>& camera) const
{
	RenderData renderData{ Intensity, DiffuseColor, SpecularColor, position, glm::normalize(derection), Distance, Falloff, glm::radians(MinAngle), glm::radians(MaxAngle) };

	// Нужно использовать дистанс для расчета фов
	float fov = glm::acos(glm::radians(MaxAngle)) * 2, zNear = 0.1f, distance = Distance;

	renderData.Cascade = GetSpotLightCascade(fov, position, derection, zNear, distance);
	return  renderData;
}

const bool shade::SpotLight::IsMeshInside(const glm::mat4& cascade, const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt)
{
	// Calculates the clip space matrix, which is the result of multiplying the View-Projection matrix by the transform matrix.
	glm::mat4 clipSpaceMatrix = cascade * transform;

	// Defines an array of 8 points of a bounding box, each one represented as a homogeneous coordinate in a vec4.
	glm::vec4 points[8]{
		{minHalfExt.x, maxHalfExt.y, minHalfExt.z, 1.f},
		{minHalfExt.x, maxHalfExt.y, maxHalfExt.z, 1.f},
		{maxHalfExt.x, maxHalfExt.y, maxHalfExt.z, 1.f},
		{maxHalfExt.x, maxHalfExt.y, minHalfExt.z, 1.f},
		{maxHalfExt.x, minHalfExt.y, minHalfExt.z, 1.f},
		{maxHalfExt.x, minHalfExt.y, maxHalfExt.z, 1.f},
		{minHalfExt.x, minHalfExt.y, maxHalfExt.z, 1.f},
		{minHalfExt.x, minHalfExt.y, minHalfExt.z, 1.f}
	};

	// Applies the clip space matrix to every point in the array of points.
	for (glm::vec4& point : points)
		point = clipSpaceMatrix * point;

	// Initializes a variable to keep track if the point is outside of the view frustum.
	bool outside = false;

	// Loop through the axis-oriented planes of the view frustum (i.e., the three first planes, ignoring the far plane),
	// and for each plane, loop through the points of the bounding box, checking if every point is outside
	// of the positive and negative half space of the plane.
	for (int i = 0; i < 3; i++)
	{
		bool outsidePositivePlane = true;
		bool outsideNegativePlane = true;

		for (glm::vec4& point : points)
		{
			outsidePositivePlane &= point[i] > point.w;
			outsideNegativePlane &= point[i] < -point.w;
		}

		outside |= outsidePositivePlane || outsideNegativePlane;
	}

	// The point is inside of the view frustum if it is not outside of the view frustum.
	return !outside;
}

shade::SpotLight::ShadowCascade shade::SpotLight::GetSpotLightCascade(float fov, const glm::vec3& position, const glm::vec3& direction, float zNear, float zFar) const
{
	glm::mat4 lightProjection = glm::perspective(fov, 1.0f, zNear, zFar);
	glm::mat4 lightView = glm::lookAt(position, position + direction, glm::vec3(0.0, 1.0, 0.0));

	return { lightProjection * lightView , zFar };
}

void shade::SpotLight::Serialize(std::ostream& stream) const
{
	serialize::Serializer::Serialize(stream, DiffuseColor);
	serialize::Serializer::Serialize(stream, SpecularColor);
	serialize::Serializer::Serialize(stream, Intensity);
	serialize::Serializer::Serialize(stream, Distance);
	serialize::Serializer::Serialize(stream, Falloff);
	serialize::Serializer::Serialize(stream, MinAngle);
	serialize::Serializer::Serialize(stream, MaxAngle);
}

void shade::SpotLight::Deserialize(std::istream& stream)
{
	serialize::Serializer::Deserialize(stream, DiffuseColor);
	serialize::Serializer::Deserialize(stream, SpecularColor);
	serialize::Serializer::Deserialize(stream, Intensity);
	serialize::Serializer::Deserialize(stream, Distance);
	serialize::Serializer::Deserialize(stream, Falloff);
	serialize::Serializer::Deserialize(stream, MinAngle);
	serialize::Serializer::Deserialize(stream, MaxAngle);
}
