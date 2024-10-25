#pragma once
#include <shade/core/environment/Light.h>
#include <shade/core/camera/Camera.h>

namespace shade
{
	class SHADE_API DirectionalLight : public Light // Rename to DirectLight
	{
	public:
		static constexpr std::uint32_t SHADOW_CASCADES_COUNT = 4;
		struct ShadowCascade
		{
			glm::mat4 ViewProjectionMatrix;
			alignas(16) float SplitDistacne = 0.0f;
		};
		struct RenderData : public Light::RenderData
		{
			alignas(16)		glm::vec3 Direction; // Do we need alignas there ?
			ShadowCascade   Cascades[SHADOW_CASCADES_COUNT];
		};
		struct RenderSettings
		{
			std::uint32_t CascadesCount = SHADOW_CASCADES_COUNT;
			// And so on 
		};
	public:
		DirectionalLight();
		virtual ~DirectionalLight();
		RenderData GetRenderData(const glm::vec3& direction, const SharedPointer<Camera>& camera) const;
	private:
		std::array<glm::vec3, 8> GetCameraFrustumCorners(const glm::mat4& projection, const glm::mat4& veiw) const;
		ShadowCascade GetShadowCascade(const shade::SharedPointer<Camera>& camera, const glm::vec3& direction, float zNear, float zFar, float splitDistance, float clipRange) const;
	public:
		SHADE_INLINE static std::uint32_t  GetTotalCount()
		{
			return m_sTotalCount;
		}

		SHADE_INLINE static RenderSettings& GetRenderSettings() 
		{
			return m_sRenderSettings;
		}

		SHADE_INLINE static float GetShadowCascadeSplitLambda()
		{
			return m_sCascadeSplitLambda;
		}

		SHADE_INLINE static void SetShadowCascadeSplitLambda(float lambda)
		{
			m_sCascadeSplitLambda = lambda;
		}

		SHADE_INLINE static float& GetZNearPlaneOffset()
		{
			return m_szNearPlaneOffset;
		}

		SHADE_INLINE static float& GetZFarPlaneOffset()
		{
			return m_szFarPlaneOffset;
		}
	private:
		static inline std::uint32_t m_sTotalCount		= 0;
		static inline float m_sCascadeSplitLambda		= 0.97f;
		static inline float m_szNearPlaneOffset			= 0.f;
		static inline float m_szFarPlaneOffset			= 0.f;
			
		static inline RenderSettings					m_sRenderSettings;
	private:
		friend class serialize::Serializer;
		void Serialize(std::ostream& stream) const;
		void Deserialize(std::istream& stream);
	};

#ifndef DIRECTIONAL_LIGHT_DATA_SIZE
	#define DIRECTIONAL_LIGHT_DATA_SIZE (sizeof(DirectionalLight::RenderData))
#endif // !GLOBAL_LIGHT_DATA_SIZE

#ifndef DIRECTIONAL_LIGHTS_DATA_SIZE
	#define DIRECTIONAL_LIGHTS_DATA_SIZE(count) (DIRECTIONAL_LIGHT_DATA_SIZE * static_cast<std::uint32_t>(count))
#endif // !GLOBAL_LIGHTS_DATA_SIZE
}

namespace shade
{
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const DirectionalLight& light)
	{
		light.Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const SharedPointer<DirectionalLight>& light)
	{
		light->Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, DirectionalLight& light)
	{
		light.Deserialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, SharedPointer<DirectionalLight>& light)
	{
		light->Deserialize(stream);
	}
}
