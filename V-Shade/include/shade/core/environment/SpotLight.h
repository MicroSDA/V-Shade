#pragma once
#include <shade/core/environment/Light.h>
#include <shade/core/camera/Camera.h>

namespace shade
{
	class SHADE_API SpotLight : public Light
	{
	public:
		struct ShadowCascade
		{
			glm::mat4 ViewProjectionMatrix;
			alignas(16) float SplitDistacne = 0.0f;
		};
		struct RenderData : public Light::RenderData
		{
			alignas(16) glm::vec3	Position;
			alignas(16) glm::vec3	Direction;
			float					Distance;
			float					Falloff;
			float					MinAngle;
			float					MaxAngle;
			ShadowCascade			Cascade;
		};
		struct RenderSettings
		{
			// TODO:
		};
	public:
		SpotLight();
		virtual ~SpotLight();
		RenderData GetRenderData(const glm::vec3& position, const glm::vec3& derection, const SharedPointer<Camera>& camera) const;

		SHADE_INLINE static std::uint32_t GetTotalCount()
		{
			return m_sTotalCount;
		}
		// Test collsion between obb and shadow cascade
		static const bool IsMeshInside(const glm::mat4& cascade, const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);
	public:
		float	Distance = 20.f;
		float	Falloff = 0.25f;
		float	MinAngle = 46.f; // degrees
		float	MaxAngle = 45.f; // degrees
	private:
		ShadowCascade GetSpotLightCascade(float fov, const glm::vec3& position, const glm::vec3& direction, float zNear, float zFar) const;
	private:
		static inline std::uint32_t m_sTotalCount = 0;
	private:
		friend class serialize::Serializer;
		void Serialize(std::ostream& stream) const;
		void Deserialize(std::istream& stream);
	};

#ifndef SPOT_LIGHT_DATA_SIZE
	#define SPOT_LIGHT_DATA_SIZE (sizeof(SpotLight::RenderData))
#endif // !SPOT_LIGHT_DATA_SIZE

#ifndef SPOT_LIGHTS_DATA_SIZE
	#define SPOT_LIGHTS_DATA_SIZE(count) (SPOT_LIGHT_DATA_SIZE * static_cast<std::uint32_t>(count))
#endif // !SPOT_LIGHTS_DATA_SIZE
}
namespace shade
{
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const SpotLight& light)
	{
		light.Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const SharedPointer<SpotLight>& light)
	{
		light->Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, SpotLight& light)
	{
		light.Deserialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, SharedPointer<SpotLight>& light)
	{
		light->Deserialize(stream);
	}
}