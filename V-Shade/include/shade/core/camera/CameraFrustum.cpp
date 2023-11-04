#include "shade_pch.h"
#include "CameraFrustum.h"
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

shade::CameraFrustum::CameraFrustum(const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
{
	CalculateFrustum(viewMatrix * projMatrix);
}

shade::CameraFrustum::CameraFrustum(const glm::mat4& viewProjectionMatrix)
{
	CalculateFrustum(viewProjectionMatrix);
}

const glm::vec4& shade::CameraFrustum::GetSide(const CameraFrustum::Side& side) const
{
	return m_Frustum[static_cast<std::uint32_t>(side)];
}

const std::array<glm::vec4, 6>& shade::CameraFrustum::GetSides() const
{
	return m_Frustum;
}

bool shade::CameraFrustum::IsInFrustum(const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt)
{
	return SSE_OBBTest(transform, minHalfExt, maxHalfExt);
}

bool shade::CameraFrustum::IsInFrustum(const glm::vec3& position, float radius)
{
	return Sphere_Test(position, radius);
}

bool shade::CameraFrustum::IsInFrustum(const glm::vec3& position, const glm::vec3& direction, float length, float radius)
{
	// Only apex works !
	glm::vec3 apex = position;
	bool isApexInside  = true;
	bool isPlaneInside = true;
	// Check if apex is inside the frustum
	for (const glm::vec4& plane : m_Frustum)
	{
		if (glm::dot(glm::vec4(apex, 1.0f), plane) < 0.0f)
		{
			isApexInside = false;
			break;
		}	
	}


	glm::vec3 baseCenter = apex + direction * length;
	glm::vec3 baseAxis1 = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction));
	glm::vec3 baseAxis2 = glm::normalize(glm::cross(direction, baseAxis1));

	glm::vec3 baseVertex1 = baseCenter + baseAxis1 * radius;
	glm::vec3 baseVertex2 = baseCenter - baseAxis1 * radius;
	glm::vec3 baseVertex3 = baseCenter + baseAxis2 * radius;
	glm::vec3 baseVertex4 = baseCenter - baseAxis2 * radius;

	for (const glm::vec4& plane : m_Frustum)
	{
		float dist1 = glm::dot(glm::vec4(baseVertex1, 1.0f), plane);
		float dist2 = glm::dot(glm::vec4(baseVertex2, 1.0f), plane);
		float dist3 = glm::dot(glm::vec4(baseVertex3, 1.0f), plane);
		float dist4 = glm::dot(glm::vec4(baseVertex4, 1.0f), plane);

		if (dist1 < 0.0f && dist2 < 0.0f && dist3 < 0.0f && dist4 < 0.0f)
			return (isApexInside || false);
	}

	return (isApexInside || isPlaneInside);

	//glm::vec3 pos = { std::cos(a) * radius, std::sin(a) * radius, length };
	//std::cos(a)* radius, std::sin(a)* radius, length
}

void shade::CameraFrustum::CalculateFrustum(const glm::mat4& viewProjectionMatrix)
{
	m_VP_Matrix = viewProjectionMatrix;
	glm::mat4 matrix = glm::transpose(m_VP_Matrix);
	//glm::mat4 matrix = glm::inverse(m_VP_Matrix);

	m_Frustum[0] = matrix[3] + matrix[0]; // Left
	m_Frustum[1] = matrix[3] - matrix[0]; // Right
	m_Frustum[2] = matrix[3] - matrix[1]; // Top
	m_Frustum[3] = matrix[3] + matrix[1]; // Bottom
	m_Frustum[4] = matrix[3] + matrix[2]; // Near
	m_Frustum[5] = matrix[3] - matrix[2]; // Far

	// Normalizing x, y, z
	for (auto i = 0; i < 6; i++)
		Normalize(m_Frustum[i]);
}

void shade::CameraFrustum::Normalize(glm::vec4& side)
{
	/* Only for x, y, z*/
	float magnitude = (float)sqrt(side.x * side.x + side.y * side.y + side.z * side.z);
	side.x /= magnitude;
	side.y /= magnitude;
	side.z /= magnitude;
	side.w /= magnitude;
}

bool shade::CameraFrustum::AABBTest(const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt)
{
	bool inside = true;
	for (int i = 0; i < 6; i++)
	{
		float d = std::max(minHalfExt.x * m_Frustum[i].x, maxHalfExt.x * m_Frustum[i].x)
			+ std::max(minHalfExt.y * m_Frustum[i].y, maxHalfExt.y * m_Frustum[i].y)
			+ std::max(minHalfExt.z * m_Frustum[i].z, maxHalfExt.z * m_Frustum[i].z)
			+ m_Frustum[i].w;
		inside &= d > 0;
	}
	return inside;
}


bool shade::CameraFrustum::SSE_AABBTest(const glm::vec3 & minHalfExt, const glm::vec3 & maxHalfExt)
{
	return false;
}

bool shade::CameraFrustum::OBBTest(const glm::mat4 & transform, const glm::vec3 & minHalfExt, const glm::vec3 & maxHalfExt)
{
	// Calculates the clip space matrix, which is the result of multiplying the View-Projection matrix by the transform matrix.
	glm::mat4 clipSpaceMatrix = m_VP_Matrix * transform;

	// Defines an array of 8 points of a bounding box, each one represented as a homogeneous coordinate in a vec4.
	glm::vec4 points[8] {
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

bool shade::CameraFrustum::SSE_OBBTest(const glm::mat4 & transform, const glm::vec3 & minHalfExt, const glm::vec3 & maxHalfExt)
{
	math::sse_mat4f clipSpaceMatrix = math::sse_mat4f(m_VP_Matrix) * math::sse_mat4f(transform);
	//box points in local space
	math::sse_vec4f obb_points_sse[8];
	obb_points_sse[0] = _mm_set_ps(1.f, minHalfExt.z, maxHalfExt.y, minHalfExt.x);
	obb_points_sse[1] = _mm_set_ps(1.f, maxHalfExt.z, maxHalfExt.y, minHalfExt.x);
	obb_points_sse[2] = _mm_set_ps(1.f, maxHalfExt.z, maxHalfExt.y, maxHalfExt.x);
	obb_points_sse[3] = _mm_set_ps(1.f, minHalfExt.z, maxHalfExt.y, maxHalfExt.x);
	obb_points_sse[4] = _mm_set_ps(1.f, minHalfExt.z, minHalfExt.y, maxHalfExt.x);
	obb_points_sse[5] = _mm_set_ps(1.f, maxHalfExt.z, minHalfExt.y, maxHalfExt.x);
	obb_points_sse[6] = _mm_set_ps(1.f, maxHalfExt.z, minHalfExt.y, minHalfExt.x);
	obb_points_sse[7] = _mm_set_ps(1.f, minHalfExt.z, minHalfExt.y, minHalfExt.x);

	__m128 zero_v = _mm_setzero_ps();
	//initially assume that planes are separating
	//if any axis is separating - we get 0 in certain outside_* place
	__m128 outside_positive_plane = _mm_set1_ps(-1.f); //NOTE: there should be negative value..
	__m128 outside_negative_plane = _mm_set1_ps(-1.f); //because _mm_movemask_ps (while storing result) cares about 'most significant bits' (it is sign of float value)

	//for all 8 box points
	for (auto i = 0u; i < 8; i++)
	{
		//transform point to clip space
		math::sse_vec4f obb_transformed_point = clipSpaceMatrix * obb_points_sse[i];

		//gather w & -w
		__m128 wwww = _mm_shuffle_ps(obb_transformed_point.xyzw, obb_transformed_point.xyzw, _MM_SHUFFLE(3, 3, 3, 3)); //get w
		__m128 wwww_neg = _mm_sub_ps(zero_v, wwww);  // negate all elements

		//box_point.xyz > box_point.w || box_point.xyz < -box_point.w ?
		//similar to point normalization: point.xyz /= point.w; And compare: point.xyz > 1 && point.xyz < -1
		__m128 outside_pos_plane = _mm_cmpge_ps(obb_transformed_point.xyzw, wwww);
		__m128 outside_neg_plane = _mm_cmple_ps(obb_transformed_point.xyzw, wwww_neg);

		//if at least 1 of 8 points in front of the plane - we get 0 in outside_* flag
		outside_positive_plane = _mm_and_ps(outside_positive_plane, outside_pos_plane);
		outside_negative_plane = _mm_and_ps(outside_negative_plane, outside_neg_plane);
	}

	//all 8 points xyz < -1 or > 1 ?
	__m128 outside = _mm_or_ps(outside_positive_plane, outside_negative_plane);

	//store result, if any of 3 axes is separating (i.e. outside != 0) - object outside frustum
	//so, object inside frustum only if outside == 0 (there are no separating axes)
	return ((_mm_movemask_ps(outside) & 0x7) == 0); //& 0x7 mask, because we interested only in 3 axes
}

bool shade::CameraFrustum::Sphere_Test(const glm::vec3 & position, float radius)
{
	bool result = true;
	for (const auto& plane : m_Frustum)
	{
		if (plane.x * position.x + plane.y * position.y + plane.z * position.z + plane.w <= -radius)
			result = false;
	}
	return result;
}
