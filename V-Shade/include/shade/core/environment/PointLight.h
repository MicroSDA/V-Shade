#pragma once
#include <shade/core/environment/Light.h>
#include <shade/core/camera/Camera.h>

namespace shade
{
	class SHADE_API PointLight : public Light
	{
	public:
		struct ShadowCascade
		{
			glm::mat4 ViewProjectionMatrix;
			alignas(16) float Distacne = 0.0f;
		};
		struct RenderData : public Light::RenderData
		{
			alignas(16) glm::vec3	Position;
			float					Distance;
			float					Falloff;
			ShadowCascade			Cascades[6]; // Where 6 represent each side of cube 
		};
	public:
		PointLight();
		virtual ~PointLight();
		RenderData GetRenderData(const glm::vec3& position, const SharedPointer<Camera>& camera) const;

		static bool IsMeshInside(const glm::vec3& position, float radius, const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);
		// Test intersection between obb and shadow cascade
		static bool IsMeshInside(const glm::mat4& cascade, const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);

	public:
		float		Distance = 2.f;
		float		Falloff	 = 0.25f;
		float		Fov = 90.f;
		static std::uint32_t GetTotalCount();
	private:
		static std::uint32_t m_sTotalCount;
		ShadowCascade    GetPointLightCascade(float fov, const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up, float zNear, float zFar) const;
	private:
		friend class serialize::Serializer;
		void Serialize(std::ostream& stream) const;
		void Deserialize(std::istream& stream);
	};
#ifndef POINT_LIGHT_DATA_SIZE
	#define POINT_LIGHT_DATA_SIZE (sizeof(PointLight::RenderData))
#endif // !POINT_LIGHT_DATA_SIZE

#ifndef POINT_LIGHTS_DATA_SIZE
	#define POINT_LIGHTS_DATA_SIZE(count) (POINT_LIGHT_DATA_SIZE * static_cast<std::uint32_t>(count))
#endif // !POINT_LIGHTS_DATA_SIZE
}

namespace shade
{
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const PointLight& light)
	{
		light.Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const SharedPointer<PointLight>& light)
	{
		light->Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, PointLight& light)
	{
		light.Deserialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, SharedPointer<PointLight>& light)
	{
		light->Deserialize(stream);
	}
}