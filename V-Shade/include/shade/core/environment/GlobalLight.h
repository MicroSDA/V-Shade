#pragma once
#include <shade/core/environment/Light.h>
#include <shade/core/camera/Camera.h>

namespace shade
{
	class SHADE_API GlobalLight : public Light // Rename to DirectLight
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
		struct RenderShadowSettings
		{
			struct PCFP // Percentage closer filtering + poisson 
			{
				alignas(4) std::uint32_t Samples	= 2; // 2 = x16, 3 = x36
				alignas(4) float Smooth			= 0.5f;
			};
			struct PCSS // Percentage closer filtering soft shadows
			{
				
			};
			enum ShadowKind : uint32_t
			{
				_PCFP = 0,
				_PCSS = 1
			};

			alignas(4) std::uint32_t	Kind = ShadowKind::_PCFP;

			float __std140Padding[3];
			RenderShadowSettings::PCFP	PCFP;
			//RenderShadowSettings::PCSS	PCSS;
		};
	public:
		GlobalLight();
		virtual ~GlobalLight();
		RenderData GetRenderData(const glm::vec3& direction, const SharedPointer<Camera>& camera) const;
	private:
		std::array<glm::vec3, 8> GetCameraFrustumCorners(const glm::mat4& projection, const glm::mat4& veiw) const;
		ShadowCascade GetShadowCascade(const shade::SharedPointer<Camera>& camera, const glm::vec3& direction, float zNear, float zFar, float splitDistance, float clipRange) const;
	public:
		SHADE_INLINE static std::uint32_t  GetTotalCount()
		{
			return m_sTotalCount;
		}

		SHADE_INLINE static RenderShadowSettings& GetRenderShadowSettings()
		{
			return m_sRenderShadowSettings;
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
			
		static inline RenderShadowSettings				m_sRenderShadowSettings;
	private:
		friend class serialize::Serializer;
		void Serialize(std::ostream& stream) const;
		void Deserialize(std::istream& stream);
	};

#ifndef GLOBAL_LIGHT_DATA_SIZE
	#define GLOBAL_LIGHT_DATA_SIZE (sizeof(GlobalLight::RenderData))
#endif // !GLOBAL_LIGHT_DATA_SIZE

#ifndef GLOBAL_LIGHTS_DATA_SIZE
	#define GLOBAL_LIGHTS_DATA_SIZE(count) (GLOBAL_LIGHT_DATA_SIZE * static_cast<std::uint32_t>(count))
#endif // !GLOBAL_LIGHTS_DATA_SIZE
}

namespace shade
{
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const GlobalLight& light)
	{
		light.Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const SharedPointer<GlobalLight>& light)
	{
		light->Serialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, GlobalLight& light)
	{
		light.Deserialize(stream);
	}

	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, SharedPointer<GlobalLight>& light)
	{
		light->Deserialize(stream);
	}
}
