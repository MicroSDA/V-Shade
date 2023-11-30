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
		struct Bloom
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
			bool			Enabled = true;

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
				Combine			= 3
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
			RenderAPI::SceneRenderData		RenderData;
			RenderAPI::RenderSettings       RenderSettings;

			Bloom BloomSettings;
			ColorCorrection ColorCorrectionSettings;
			SSAO SSAOSettings;
			bool IsAABB_OBBShow = false;
			bool IsGridShow = true;
			bool IsPointLightShow = false;
			bool IsSpotLightShow  = false;
			bool IsPhysicsContactPointShow = true;
			bool IsPointLightShadowSplitBySides = false;
		};
		struct Statistic
		{
			// Timing
			float InstanceGeometry = 0;
			float GlobalLightPreDepth = 0;
			float SpotLightPreDepth = 0;
			float PointLightPreDepth = 0;
			float Bloom = 0;
			float ColorCorrection = 0;
			float LightCullingPreDepth = 0;
			float LightCullingCompute = 0;
			// Others
			std::uint32_t SubmitedInstances   = 0;
			std::uint32_t SubmitedPointLights = 0;
			std::uint32_t SubmitedSpotLights  = 0;

			void Reset() { (*this) = Statistic{}; }
		};
	public:
		static SharedPointer<SceneRenderer> Create(bool swapChainAsMainTarget = false);
		SceneRenderer(bool swapChainAsMainTarget);
		virtual ~SceneRenderer() = default;
                            
		void OnUpdate(SharedPointer<Scene>& scene, const FrameTimer& deltaTime);
		void OnRender(SharedPointer<Scene>& scene, const FrameTimer& deltaTime);
		void OnEvent(SharedPointer<Scene>& scene, const Event& event, const FrameTimer& deltaTime);

		std::vector<SharedPointer<FrameBuffer>>& GetMainTargetFrameBuffer();
		SharedPointer<Texture2D>&	GetBloomRenderTarget();
		SharedPointer<Texture2D>&	GetSAAORenderTarget();

		Settings& GetSettings();
		const Statistic& GetStatistic() const;

		SharedPointer<Camera>& GetActiveCamera();

		void RecompileAllPipelines();
	private:

		SharedPointer<RenderCommandBuffer> m_MainCommandBuffer;

		std::vector<SharedPointer<FrameBuffer>>	m_MainTargetFrameBuffer;
		SharedPointer<RenderPipeline>	m_MainGeometryPipelineStatic;
		SharedPointer<RenderPipeline>	m_MainGeometryPipelineAnimated;

		SharedPointer<FrameBuffer>		m_LightCullingPreDepthFrameBuffer;
		SharedPointer<RenderPipeline>	m_LightCullingPreDepthPipeline;
		SharedPointer<ComputePipeline>	m_LightCullingPipeline;

		SharedPointer<FrameBuffer>		m_GlobalLightShadowFrameBuffer;
		SharedPointer<RenderPipeline>	m_GlobalLightShadowDepthPipeline;

		SharedPointer<FrameBuffer>		m_SpotLightShadowFrameBuffer;
		SharedPointer<RenderPipeline>	m_SpotLightShadowDepthPipeline;

		SharedPointer<FrameBuffer>		m_PointLightShadowFrameBuffer;
		SharedPointer<RenderPipeline>	m_PointLightShadowDepthPipeline;

		SharedPointer<Texture2D>		m_BloomTarget;
		SharedPointer<ComputePipeline>	m_BloomPipeline;

		SharedPointer<RenderPipeline>	m_GridPipeline;
		SharedPointer<RenderPipeline>	m_AABB_OBB_Pipeline;

		SharedPointer<RenderPipeline>	m_PointLightVisualizationPipeline;
		SharedPointer<RenderPipeline>	m_SpotLightVisualizationPipeline;

		SharedPointer<ComputePipeline>	m_ColorCorrectionPipeline;

		SharedPointer<ComputePipeline>	m_ScreenSpaceAmbientOcclusionPipeline;
		SharedPointer<Texture2D>		m_ScreenSpaceAmbientOcclusionTarget;

		SharedPointer<StorageBuffer>	m_VisiblePointLightIndicesBuffer;
		SharedPointer<StorageBuffer>	m_VisibleSpotLightIndicesBuffer;
		SharedPointer<UniformBuffer>	m_SSAOSamplesBuffer;


		SharedPointer<Camera>			m_Camera;

		SharedPointer<Material>			m_CollisionPointsMaterial;
		SharedPointer<Material>			m_AABBMaterial;
		SharedPointer<Material>			m_OBBMaterial;
		SharedPointer<Material>			m_ConvexMeshMaterial;
		SharedPointer<Material>			m_LightVisualizingMaterial;
		
	private:
		// Settings
		Settings m_Settings;
		Statistic m_Statistic;

		SharedPointer<Plane>	m_Plane;
		SharedPointer<Box>		m_OBB;
		SharedPointer<Sphere>	m_Sphere;
		SharedPointer<Cone>		m_Cone;
	private:

		void GlobalLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void SpotLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void PointLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void InstancedGeometryPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void LightCullingPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void GridPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);
		void LightVisualizationPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);

		void FlatPipeline(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear = false);

		void LightCullingComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex);
		void ColorCorrectionComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex);
		void BloomComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex);
		void SSAOComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex);

	};
}