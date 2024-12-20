#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/scene/Scene.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/event/Event.h>
#include <shade/core/render/buffers/RenderCommandBuffer.h>
#include <shade/core/render/RenderPipeline.h>
#include <shade/core/render/buffers/IndexBuffer.h>
#include <shade/core/render/buffers/FrameBuffer.h>
#include <shade/core/camera/Camera.h>
#include <shade/core/render/Renderer.h>
#include <shade/core/render/drawable/primitives/Plane.h>
#include <shade/core/render/drawable/primitives/Box.h>
#include <shade/core/render/drawable/primitives/Sphere.h>
#include <shade/core/render/drawable/primitives/Cone.h>

namespace shade
{
	// TODO: Make as virtual base parent!
	class SHADE_API SceneRenderer
	{
	public:
		struct BloomSettings
		{
			enum class Stage : std::uint32_t
			{
				HDR	= 0,
				DownSample,	
				UpSample,
				Combine
			};
			struct RenderData
			{
				glm::vec3		Curve = glm::vec3(0.f);
				float			Exposure = 1.0;
				float			Threshold = 0.5;
				float			Knee = 0.1;
				std::uint32_t	Lod = 0;
				Stage			Stage = Stage::HDR;
			};

			float			Exposure = 1.0;
			float			Threshold = 0.5;
			float			Knee = 0.1;
			std::uint32_t	Samples = 5;

			RenderData GetRenderData() { return { GetCurve(), Exposure, Threshold, Knee }; }
			glm::vec3 GetCurve() { return { Threshold - Knee, Knee * 2, 0.25f / Knee }; }
		};
		struct ColorCorrection
		{
			struct RenderData
			{
				float Exposure = 1.f;
				float Gamma = 1.f;
			};
			float Exposure = 1.f;
			float Gamma    = 1.f;
			bool  Enabled  = true;

			RenderData GetRenderData() { return { Exposure, Gamma }; }
		};
		struct SSAO
		{
			enum class Stage : std::uint32_t
			{
				Generate		= 0,
				BlurHorizontal	= 1,
				BlurVertical	= 2,
				Zoom			= 3,
				Combine			= 4
			};
			static constexpr std::uint32_t		MAX_SAMPLES = 64;
			std::uint32_t						SamplesCount = MAX_SAMPLES;
			float								Radius = 0.5f;
			float								Bias = 0.025f;
			std::uint32_t						BlurSamples = 2;

			static std::array<glm::vec4, MAX_SAMPLES> GenerateSamples()
			{
				static std::array<glm::vec4, MAX_SAMPLES> samples;
				std::uniform_real_distribution<float> distribution{ 0.f, 1.f };
				//std::random_device randomDevice;
				std::default_random_engine generator;

				for (std::uint32_t i = 0; i < MAX_SAMPLES; ++i)
				{
					samples[i] = glm::vec4(distribution(generator) * 2.f - 1.f, distribution(generator) * 2.f - 1.f, distribution(generator), 0.f);

					samples[i] = glm::normalize(samples[i]);
					samples[i] *= distribution(generator);

					float scale = static_cast<float>(i) / static_cast<float>(MAX_SAMPLES);
					samples[i] *= math::Lerp(0.1f, 1.f, scale * scale);
				}

				return samples;
			}
			static std::array<glm::vec4, 32> GenerateNoise()
			{
				static std::array<glm::vec4, 32> noise;
				std::uniform_real_distribution<float> distribution{ 0.f, 1.f };
				//std::random_device randomDevice;
				std::default_random_engine generator;

				for (std::uint32_t i = 0; i < 32; i++)
					noise[i] = glm::vec4(distribution(generator) * 2.f - 1.f, distribution(generator) * 2.f - 1.f, 0.f, 0.f);
				return noise;
			}
			struct RenderData
			{
				std::uint32_t						SamplesCount = MAX_SAMPLES;
				float								Radius = 0.5f;
				float								Bias = 0.025f;
				SSAO::Stage							Stage = SSAO::Stage::Generate;
				std::uint32_t						BlurSamples = 2;
			};
			struct RenderBuffer
			{
				// Actually we use only vec3, but std 140 require 16 byte alignment so vec4
				std::array<glm::vec4, MAX_SAMPLES>	Samples;
				std::array<glm::vec4, 32>			Noise;
			};
			RenderData		GetRenderData()   { return { SamplesCount, Radius, Bias, SSAO::Stage::Generate, BlurSamples }; }
			RenderBuffer    GetRenderBuffer() { return { GenerateSamples(), GenerateNoise() }; }
		};
		struct Settings
		{
			RenderAPI::SceneRenderData				RenderData;
			RenderAPI::RenderSettings				RenderSettings;

			SceneRenderer::BloomSettings			Bloom;
			SceneRenderer::ColorCorrection			ColorCorrection;
			SceneRenderer::SSAO						SSAO;
		};
		// TODO: Remove ? 
		struct Statistic
		{
			std::uint32_t SubmitedInstances			= 0;
			std::uint32_t SubmitedOmnidirectLights	= 0;
			std::uint32_t SubmitedSpotLights		= 0;

			void Reset() { (*this) = Statistic{}; }
		};
	public:
		static SharedPointer<SceneRenderer> Create(bool swapChainAsMainTarget = false);
		SceneRenderer(bool swapChainAsMainTarget);
		virtual ~SceneRenderer() = default;
                            
		void OnUpdate(SharedPointer<Scene>& scene, const shade::CameraComponent& camera, const FrameTimer& deltaTime, const ecs::Entity& activeEntity = ecs::Entity{});
		void OnRender(SharedPointer<Scene>& scene, const FrameTimer& deltaTime);
		void OnEvent(SharedPointer<Scene>& scene, const Event& event, const FrameTimer& deltaTime);

		SharedPointer<FrameBuffer>& GetMainTargetFrameBuffer();
		SharedPointer<Texture2D>&	GetBloomRenderTarget();
		SharedPointer<Texture2D>&	GetSAAORenderTarget();

		Settings& GetSettings();
		const Statistic& GetStatistic() const;

		SharedPointer<Camera>& GetActiveCamera();

		SharedPointer<Pipeline>& RegisterNewPipeline(const SharedPointer<Pipeline>& pipeline);
		SharedPointer<Pipeline>  GetPipeline(const std::string& name);

		std::map<std::string, SharedPointer<Pipeline>>& GetPipelines();
	private:

		SharedPointer<RenderCommandBuffer> m_MainCommandBuffer;

		SharedPointer<FrameBuffer>		m_MainTargetFrameBuffer;
		SharedPointer<FrameBuffer>		m_LightCullingPreDepthFrameBuffer;
		SharedPointer<FrameBuffer>		m_GlobalLightShadowFrameBuffer;
		SharedPointer<FrameBuffer>		m_SpotLightShadowFrameBuffer;
		SharedPointer<FrameBuffer>		m_PointLightShadowFrameBuffer;
		
		SharedPointer<Texture2D>		m_BloomTarget;
		SharedPointer<Texture2D>		m_SSAOTarget;

		SharedPointer<StorageBuffer>	m_VisiblePointLightIndicesBuffer;
		SharedPointer<StorageBuffer>	m_VisibleSpotLightIndicesBuffer;
		SharedPointer<UniformBuffer>	m_SSAOSamplesBuffer;

		SharedPointer<Camera>			m_Camera;

		SharedPointer<Material>			m_AABBMaterial;
		SharedPointer<Material>			m_OBBMaterial;
		SharedPointer<Material>			m_LightVisualizingMaterial;
		SharedPointer<Material>			m_JoinVisualizingMaterial;
		
	private:
		// Settings
		Settings m_Settings;
		Statistic m_Statistic;

		SharedPointer<Plane>	m_Plane;
		SharedPointer<Box>		m_OBB;
		SharedPointer<Sphere>	m_Sphere;
		SharedPointer<Cone>		m_Cone;

		std::map<std::string, SharedPointer<Pipeline>> m_Pipelines;
	private:

		void GlobalLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void SpotLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void PointLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void InstancedGeometryPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void LightCullingPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void GridPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void LightVisualizationPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void SkeletonVisualizationPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);

		void FlatPipeline(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);

		void LightCullingComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex);
		void ColorCorrectionComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex);
		void BloomComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex);
		void SSAOComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex);
	};
}