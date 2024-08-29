#pragma once
#include <shade/core/environment/Light.h>
#include <shade/core/camera/Camera.h>

namespace shade
{
	class SHADE_API GlobalLight : public Light
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
	public:
		GlobalLight();
		virtual ~GlobalLight();
		RenderData GetRenderData(const glm::vec3& direction, const SharedPointer<Camera>& camera) const;
		static float GetShadowCascadeSplitLambda();
		static void SetShadowCascadeSplitLambda(float lambda);

		float zNearPlaneOffset = 0.f, zFarPlaneOffset = 0.f;
	private:
		std::array<glm::vec3, 8> GetCameraFrustumCorners(const glm::mat4& projection, const glm::mat4& veiw) const;
		ShadowCascade GetShadowCascade(const shade::SharedPointer<Camera>& camera, const glm::vec3& direction, float zNear, float zFar, float splitDistance, float clipRange) const;
	public:
		static std::uint32_t GetTotalCount();
	private:
		static std::uint32_t m_sTotalCount;
		static float m_CascadeSplitLambda;
	private:
		friend class Serializer;
		std::size_t Serialize(std::ostream& stream) const;
		std::size_t Deserialize(std::istream& stream);
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
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const GlobalLight& light, std::size_t)
	{
		return light.Serialize(stream);
	}

	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const SharedPointer<GlobalLight>& light, std::size_t)
	{
		return light->Serialize(stream);
	}

	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, GlobalLight& light, std::size_t)
	{
		return light.Deserialize(stream);
	}

	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, SharedPointer<GlobalLight>& light, std::size_t)
	{
		return light->Deserialize(stream);
	}
}
