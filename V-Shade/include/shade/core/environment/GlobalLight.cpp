#include "shade_pch.h"
#include "GlobalLight.h"
#include <glm/glm/gtx/compatibility.hpp>

shade::DirectionalLight::DirectionalLight()
{
	++m_sTotalCount;
}

shade::DirectionalLight::~DirectionalLight()
{
	--m_sTotalCount;
}

shade::DirectionalLight::RenderData shade::DirectionalLight::GetRenderData(const glm::vec3& direction, const SharedPointer<Camera>& camera) const
{
	RenderData renderData{ Intensity, DiffuseColor, SpecularColor, direction };
	std::array<float, SHADOW_CASCADES_COUNT> cascadeSplits;
	// Get camera near and far clip planes and calculate the range
	float nearClip = camera->GetNear(), farClip = camera->GetFar();
	float clipRange = farClip - nearClip;
	// Set minimum and maximum z depth values based on clip plane values
	float minZ = nearClip, maxZ = nearClip + clipRange;
	// Calculate the range and ratio of z depth values
	float range = maxZ - minZ, ratio = maxZ / minZ;
	// Calculate split depths based on view camera frustum
	// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
	for (uint32_t i = 0; i < SHADOW_CASCADES_COUNT; i++) {
		float p = (i + 1) / static_cast<float>(SHADOW_CASCADES_COUNT);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = m_sCascadeSplitLambda * (log - uniform) + uniform;
		// Assign calculated split depths to cascadeSplits array
		cascadeSplits[i] = farClip * (d - nearClip) / clipRange;
	}
	// Calculate shadow cascades for each split depth segment
	for (auto i = 0; i < SHADOW_CASCADES_COUNT; i++)
	{
		// Calculate shadow cascade based on current and previous split depths
		if (i < 1)
			renderData.Cascades[i] = GetShadowCascade(camera, direction, camera->GetNear(), cascadeSplits[i], cascadeSplits[i], clipRange);
		else if (i < SHADOW_CASCADES_COUNT - 1)
			renderData.Cascades[i] = GetShadowCascade(camera, direction, cascadeSplits[i - 1], cascadeSplits[i], cascadeSplits[i], clipRange);
		else
			renderData.Cascades[i] = GetShadowCascade(camera, direction, cascadeSplits[i - 1], camera->GetFar(), cascadeSplits[i], clipRange);
	}
	return renderData;
}

std::array<glm::vec3, 8> shade::DirectionalLight::GetCameraFrustumCorners(const glm::mat4& projection, const glm::mat4& view) const
{
	//// Create the view projection matrix by multiplying the flipped Y projection matrix with the view matrix.
	glm::mat4 cameraViewProjection = glm::inverse(projection * view);
	// Define the 8 corners of the frustum in world space.
	std::array<glm::vec3, 8> corners 
	{
		glm::vec3( -1.0f,	1.0f,	0.0f),
		glm::vec3(	1.0f,	1.0f,	0.0f),
		glm::vec3(	1.0f,  -1.0f,	0.0f),
		glm::vec3( -1.0f,  -1.0f,	0.0f),

		glm::vec3( -1.0f,	1.0f,	1.0f),
		glm::vec3(	1.0f,   1.0f,	1.0f),
		glm::vec3(	1.0f,  -1.0f,	1.0f),
		glm::vec3( -1.0f,  -1.0f,	1.0f),
	};

	// Transform the corners of the frustum by multiplying each of them with the view projection matrix
	for (auto i = 0; i < corners.size(); i++)
	{
		const glm::vec4 pt = cameraViewProjection * glm::vec4(corners[i], 1.f);
		// Divide by the W component of the transformed corners to convert them from homogeneous coordinates.
		corners[i] = pt / pt.w;
	}
	// Return the transformed corners of the frustum as an array of 8 vectors.
	return corners;
}

shade::DirectionalLight::ShadowCascade shade::DirectionalLight::GetShadowCascade(const shade::SharedPointer<Camera>& camera, const glm::vec3& direction, float zNear, float zFar, float splitDistance, float clipRange) const
{
	//Create the projection matrix using the camera's field of view, aspect ratio, and near and far planes
	glm::mat4 projection = glm::perspective(camera->GetFov(), camera->GetAspect(), zNear, zFar);
	//Get the frustum corners for the camera and projection matrix
	auto frustumCorners = GetCameraFrustumCorners(projection, camera->GetView());
	glm::vec3 frustumCenter(0.0f);
	//Calculate the center point of the frustum
	for (const auto& corner : frustumCorners)
		frustumCenter += glm::vec3(corner);
	frustumCenter /= 8.0f; // 8
	
	
	float radius = 0.0f;
	//Calculate the radius of a sphere that tightly fits around the frustum
	for (const auto& corner : frustumCorners)
	{
		float distance = glm::length(corner - frustumCenter);
		radius = glm::max(radius, distance);
	}
	//Round up the radius for appropriate shadow mapping
	radius = std::ceil(radius * 16.0f) / 16.0f;
	//Calculate the max and min extends based on the radius
	glm::vec3 maxExtents = glm::vec3(radius);
	glm::vec3 minExtents = -maxExtents;

	//Create a view matrix for the light based on the frustum center and direction, with an up vector pointing upwards
	const glm::mat4 lightView = glm::lookAt(frustumCenter - direction * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
	//Create the orthographic projection matrix for the light based on the max and min extends calculated and the camera's far plane
	const glm::mat4 lightProjection = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, -camera->GetFar() + m_szNearPlaneOffset, (maxExtents.z - minExtents.z) + m_szFarPlaneOffset);
	//Return the combined light view and projection matrices, along with the negative radius
	return  { lightProjection * lightView, radius * -1.f };
}

void shade::DirectionalLight::Serialize(std::ostream& stream) const
{
	serialize::Serializer::Serialize(stream, DiffuseColor);
	serialize::Serializer::Serialize(stream, SpecularColor);
	serialize::Serializer::Serialize(stream, Intensity);
}

void shade::DirectionalLight::Deserialize(std::istream& stream)
{
	serialize::Serializer::Deserialize(stream, DiffuseColor);
	serialize::Serializer::Deserialize(stream, SpecularColor);
	serialize::Serializer::Deserialize(stream, Intensity);
}


