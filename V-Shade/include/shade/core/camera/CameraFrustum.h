#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/math/Math.h>
#include <glm/glm/glm.hpp>

namespace shade
{
	class SHADE_API CameraFrustum
	{
	public:
		enum class Side
		{
			Left, Right, Top, Bottom, Near, Far
		};

		CameraFrustum(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
		CameraFrustum(const glm::mat4& viewProjectionMatrix);
		~CameraFrustum() = default;

		const glm::vec4& GetSide(const CameraFrustum::Side& side) const;
		const std::array<glm::vec4, 6>& GetSides() const;

		bool IsInFrustum(const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);
		// Point light test
		bool IsInFrustum(const glm::vec3& position, float radius);
		// Spot light test
		bool IsInFrustum(const glm::vec3& position, const glm::vec3& direction, float length, float radius);
	private:
		// Camera View and Projection product
		glm::mat4 m_VP_Matrix; 
		std::array<glm::vec4, 6> m_Frustum;
	private:
		void CalculateFrustum(const glm::mat4& viewProjectionMatrix);
		void Normalize(glm::vec4& side);
		bool AABBTest(const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);
		bool SSE_AABBTest(const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);
		bool OBBTest(const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);
		bool SSE_OBBTest(const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);
		bool Sphere_Test(const glm::vec3& position, float radius);
	};
}