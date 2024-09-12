#include "shade_pch.h"
#include "PointLight.h"

std::uint32_t shade::PointLight::m_sTotalCount = 0;

shade::PointLight::PointLight()
{
    ++m_sTotalCount;
}

shade::PointLight::~PointLight()
{
    --m_sTotalCount;
}

shade::PointLight::RenderData shade::PointLight::GetRenderData(const glm::vec3& position, const SharedPointer<Camera>& camera) const
{
    RenderData renderData{ Intensity, DiffuseColor, SpecularColor, position, Distance, Falloff };

    GLM_CONSTEXPR float fov = glm::radians(90.0f);
    float zNear = 0.1f, aspect = 1.0f, distance = Distance + 1.f;
    ShadowCascade cascades[6];

    renderData.Cascades[0] = GetPointLightCascade(fov, position, glm::vec3(1.0, 0.0, 0.0),  glm::vec3(0.0, -1.0, 0.0),  zNear, distance);	 // x
    renderData.Cascades[1] = GetPointLightCascade(fov, position, glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0),  zNear, distance);    //-x
    renderData.Cascades[2] = GetPointLightCascade(fov, position, glm::vec3(0.0, 1.0, 0.0),  glm::vec3(0.0, 0.0, 1.0),   zNear, distance);	 // y
    renderData.Cascades[3] = GetPointLightCascade(fov, position, glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0),  zNear, distance);    //-y
    renderData.Cascades[4] = GetPointLightCascade(fov, position, glm::vec3(0.0, 0.0,  1.0), glm::vec3(0.0, -1.0, 0.0),  zNear, distance);	 // z
    renderData.Cascades[5] = GetPointLightCascade(fov, position, glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0),  zNear, distance);    //-z

   return  renderData;
}

bool shade::PointLight::IsMeshInside(const glm::vec3& position, float radius, const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt)
{
#if 1
	glm::vec3 t, r, scale;
	math::DecomposeMatrix(transform, t, r, scale);
	glm::vec3 localPos = glm::inverse(glm::scale(transform, glm::vec3(1.f))) * glm::vec4(position, 1.f);
#if 0
	float x = std::max(minHalfExt.x, std::min(realCenter.x, maxHalfExt.x));
	float y = std::max(minHalfExt.y, std::min(realCenter.y, maxHalfExt.y));
	float z = std::max(minHalfExt.z, std::min(realCenter.z, maxHalfExt.z));
	glm::vec3 closetPoint(x, y, z);
#else
	glm::vec3 closetPoint = glm::clamp(localPos, minHalfExt, maxHalfExt);
#endif
	glm::vec3 localPoint = closetPoint - localPos;

	return (glm::length(localPoint * scale) <= radius);
#else
	/* Without scale !*/
	/* Transform light position to obb local sapce without scale */
	glm::vec3 localPos = glm::inverse(glm::scale(transform, glm::vec3(1.f))) * glm::vec4(position, 1.f);
	/* Getting closet point */
#if 1
	float x = std::max(minHalfExt.x, std::min(localPos.x, maxHalfExt.x));
	float y = std::max(minHalfExt.y, std::min(localPos.y, maxHalfExt.y));
	float z = std::max(minHalfExt.z, std::min(localPos.z, maxHalfExt.z));
	glm::vec3 closetPoint(x, y, z);
#else
	glm::vec3 closetPoint = glm::clamp(localPos, minHalfExt, maxHalfExt);
#endif
	glm::vec3 distance = closetPoint - localPos;
	return (glm::length(distance) <= radius)
#endif
}

bool shade::PointLight::IsMeshInside(const glm::mat4& cascade, const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt)
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

std::uint32_t shade::PointLight::GetTotalCount()
{
    return m_sTotalCount;
}

shade::PointLight::ShadowCascade shade::PointLight::GetPointLightCascade(float fov, const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up, float zNear, float zFar) const
{
    glm::mat4 lightProjection = glm::perspective(fov, 1.0f, zNear, zFar);
    glm::mat4 lightView = glm::lookAt(position, position + direction, up);

    return { lightProjection * lightView , zFar };

    return ShadowCascade();
}

void shade::PointLight::Serialize(std::ostream& stream) const
{
	
	serialize::Serializer::Serialize(stream, DiffuseColor);
	serialize::Serializer::Serialize(stream, SpecularColor);
	serialize::Serializer::Serialize(stream, Intensity);
	serialize::Serializer::Serialize(stream, Distance);
	serialize::Serializer::Serialize(stream, Falloff);
}

void shade::PointLight::Deserialize(std::istream& stream)
{
	serialize::Serializer::Deserialize(stream, DiffuseColor);
	serialize::Serializer::Deserialize(stream, SpecularColor);
	serialize::Serializer::Deserialize(stream, Intensity);
	serialize::Serializer::Deserialize(stream, Distance);
	serialize::Serializer::Deserialize(stream, Falloff);
}
