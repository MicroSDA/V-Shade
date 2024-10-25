#pragma once
#include <shade/core/environment/Light.h>
#include <shade/core/camera/Camera.h>

namespace shade
{
	class SHADE_API OmnidirectionalLight : public Light
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
		struct RenderSettings
		{
			bool SplitBySides		= false;
		};
	public:
		OmnidirectionalLight();
		virtual ~OmnidirectionalLight();
		RenderData GetRenderData(const glm::vec3& position, const SharedPointer<Camera>& camera) const;

		static bool IsMeshInside(const glm::vec3& position, float radius, const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);
		// Test intersection between obb and shadow cascade
		static bool IsMeshInside(const glm::mat4& cascade, const glm::mat4& transform, const glm::vec3& minHalfExt, const glm::vec3& maxHalfExt);

		SHADE_INLINE static std::uint32_t  GetTotalCount()
		{
			return m_sTotalCount;
		}

		SHADE_INLINE static RenderSettings& GetRenderSettings()
		{
			return m_sRenderSettings;
		}

	public:
		float		Distance = 2.f;
		float		Falloff	 = 0.25f;
		float		Fov = 90.f;
	private:
		static inline std::uint32_t		m_sTotalCount = 0;
		static inline RenderSettings	m_sRenderSettings;

		ShadowCascade GetLightCascade(float fov, const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up, float zNear, float zFar) const;
	private:
		friend class serialize::Serializer;
		void Serialize(std::ostream& stream) const;
		void Deserialize(std::istream& stream);
	};
#ifndef OMNIDIRECTIONAL_LIGHT_DATA_SIZE
	#define OMNIDIRECTIONAL_LIGHT_DATA_SIZE (sizeof(OmnidirectionalLight::RenderData))
#endif // !OMNIDIRECTIONAL_LIGHT_DATA_SIZE

#ifndef OMNIDIRECTIONAL_LIGHTS_DATA_SIZE
	#define OMNIDIRECTIONAL_LIGHTS_DATA_SIZE(count) (OMNIDIRECTIONAL_LIGHT_DATA_SIZE * static_cast<std::uint32_t>(count))
#endif // !OMNIDIRECTIONAL_LIGHTS_DATA_SIZE
}

namespace shade
{
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const OmnidirectionalLight& light)
	{
		light.Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const SharedPointer<OmnidirectionalLight>& light)
	{
		light->Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, OmnidirectionalLight& light)
	{
		light.Deserialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, SharedPointer<OmnidirectionalLight>& light)
	{
		light->Deserialize(stream);
	}
}