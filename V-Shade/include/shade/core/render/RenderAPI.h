#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/system/SystemsRequirements.h>
#include <shade/utils/Logger.h>
#include <shade/core/render/shader/ShaderLibrary.h>

#include <shade/core/render/SwapChain.h>
#include <shade/core/render/buffers/FrameBuffer.h>

#include <shade/core/render/RenderPipeline.h>
#include <shade/core/render/buffers/RenderCommandBuffer.h>

#include <shade/core/render/buffers/VertexBuffer.h>
#include <shade/core/render/buffers/IndexBuffer.h>

#include <shade/core/render/buffers/StorageBuffer.h>
#include <shade/core/render/buffers/UniformBuffer.h>
#include <shade/core/render/RenderContext.h>

#include <shade/core/camera/Camera.h>

#include <shade/core/render/drawable/Drawable.h>
#include <shade/core/render/drawable/Material.h>

#include <shade/core/render/RenderData.h>

namespace shade
{
	class SHADE_API RenderAPI
	{
	public:
		// TODO: Probably need to change it or way to how it works, mby we need to parse data form shader and set binding ids from here !		
		// Set 0
		static constexpr std::uint32_t CAMERA_BINDING					= 0;
		static constexpr std::uint32_t SCENE_RENDER_DATA_BINDING		= 1;
		static constexpr std::uint32_t RENDER_SETTINGS_BINDING			= 2;
		static constexpr std::uint32_t GLOBAL_LIGHT_BINDING				= 3;
		static constexpr std::uint32_t POINT_LIGHT_BINDING				= 4;
		static constexpr std::uint32_t SPOT_LIGHT_BINDING				= 5;
		// Set 1
		static constexpr std::uint32_t MATERIAL_BINDING					= 0;
		static constexpr std::uint32_t DIFFUSE_TEXTURE_BINDING			= 1;
		static constexpr std::uint32_t SPECULAR_TEXTURE_BINDING			= 2;
		static constexpr std::uint32_t NORMAL_TEXTURE_BINDING			= 3;
		static constexpr std::uint32_t GLOBAL_SHADOW_MAP_BINDING		= 4;
		static constexpr std::uint32_t SPOT_SHADOW_MAP_BINDING			= 5;
		static constexpr std::uint32_t POINT_SHADOW_MAP_BINDING			= 6;
		static constexpr std::uint32_t SPOT_LIGHT_INDINCES_BINDING		= 7;
		static constexpr std::uint32_t POINT_LIGHT_INDINCES_BINDING		= 8;
		static constexpr std::uint32_t BONE_TRANSFORMS_BINDING			= 9;

		static constexpr std::uint32_t MAX_GLOBAL_LIGHTS_COUNT			= 10;
		static constexpr std::uint32_t MAX_GLOBAL_SHADOW_CASTERS		= 1;
		static constexpr std::uint32_t MAX_POINT_LIGHTS_COUNT			= 1024;
		static constexpr std::uint32_t MAX_POINT_SHADOW_CASTERS			= 50;
		static constexpr std::uint32_t MAX_SPOT_LIGHTS_COUNT			= 1024;
		static constexpr std::uint32_t MAX_SPOT_SHADOW_CASTERS			= 50;
		static constexpr std::uint32_t MAX_ANIMATION_PLAY				= 1024;
		static constexpr std::uint32_t MAX_BONES_PER_INSTANCE			= 100;

		struct SceneRenderData
		{
		private:
			friend class	Renderer;
			std::uint32_t	GlobalLightCount		= 0;
			std::uint32_t	PointsLightCount		= 0;
			std::uint32_t	SpotLightCount			= 0;
			std::uint32_t	SubmitedBoneTransfroms  = 0;
		public:
			const std::uint32_t GetGlobalLightCount() { return GlobalLightCount; }
			const std::uint32_t GetPointsLightCount() { return PointsLightCount; }
			const std::uint32_t GetSpotLightCount()   { return SpotLightCount; }
		};

		struct VramUsage
		{
			struct Heap
			{
				std::size_t UsedMemoryBytes = 0;
				std::size_t TotalMemoryBytes = 0;
				std::size_t AvailableMemory = 0;
			};

			std::vector<Heap> Heaps;
		};

		struct RenderSettings
		{
			RenderSettings() {
				// Shader interpriate bool as uint with size = 4,
				// so we need to have proper memory alignment here for each elements.
				// Since bool it's uint in the shader we need to use memset to set all additional bits to 0 after alignment.
				memset(this, 0, sizeof(RenderSettings));
				// Common default values.
				LightCulling			= false;
				ShowLightComplexity		= false;
				GlobalShadowsEnabled	= false;
				SpotShadowEnabled		= false;
				PointShadowEnabled		= false;
				ShowShadowCascades		= false;
				SSAOEnabled				= false;
			}
			alignas(4) bool LightCulling;
			alignas(4) bool ShowLightComplexity;
			alignas(4) bool GlobalShadowsEnabled;
			alignas(4) bool SpotShadowEnabled;
			alignas(4) bool PointShadowEnabled;
			alignas(4) bool ShowShadowCascades;
			alignas(4) bool SSAOEnabled;
		};
	public:
		enum class API
		{
			None = 0,
			Vulkan = 1,
			OpenGL = 2
		};
	public:
		virtual UniquePointer<RenderContext> Initialize(const SystemsRequirements& requirements) = 0;
		static UniquePointer<RenderAPI> Create(const RenderAPI::API& api);
		virtual ~RenderAPI() = default;
		virtual void ShutDown() = 0;

		virtual void BeginFrame(std::uint32_t frameIndex) = 0;
		virtual void EndFrame(std::uint32_t frameIndex) = 0;

		virtual void BeginScene(SharedPointer<Camera>& camera, std::uint32_t frameIndex) = 0;
		virtual void EndScene(std::uint32_t frameIndex) = 0;

		virtual void BeginRender(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::uint32_t frameIndex, bool clear, std::uint32_t clearCount) = 0;
		virtual void BeginRenderWithCustomomViewPort(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::uint32_t frameIndex, glm::vec2 viewPort, bool isClear) = 0;

		virtual void EndRender(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex) = 0;
		virtual void DrawInstanced(
			SharedPointer<RenderCommandBuffer>& commandBuffer,
			const SharedPointer<VertexBuffer>& vertices,
			const SharedPointer<IndexBuffer>& indices,
			const SharedPointer<VertexBuffer>& transforms,
			std::uint32_t count,
			std::uint32_t transformOffset) = 0;

		virtual void DrawInstancedAnimated(
			SharedPointer<RenderCommandBuffer>& commandBuffer,
			const SharedPointer<VertexBuffer>& vertices,
			const SharedPointer<IndexBuffer>& indices,
			const SharedPointer<VertexBuffer>& bones,
			const SharedPointer<VertexBuffer>& transforms,
			std::uint32_t count,
			std::uint32_t transformOffset) = 0;

		virtual void DummyInvocation(
			SharedPointer<RenderCommandBuffer>& commandBuffer,
			const SharedPointer<VertexBuffer>& vertices,
			const SharedPointer<IndexBuffer>& indices,
			const SharedPointer<VertexBuffer>& bones,
			const SharedPointer<VertexBuffer>& transforms,
			std::uint32_t count,
			std::uint32_t transformOffset) = 0;

		virtual void BeginTimestamp(SharedPointer<RenderCommandBuffer>& commandBuffer, const std::string& name) = 0;
		virtual float EndTimestamp(SharedPointer<RenderCommandBuffer>& commandBuffer, const std::string& name) = 0;
		virtual VramUsage GetVramMemoryUsage() = 0;

		// Return current frame index which set by BeginFrame in SwapChain 
		static std::uint32_t GetCurrentFrameIndex();
		static std::uint32_t GetFramesCount();
		static RenderAPI::API GetCurrentAPI();
		static const char* GetCurrentAPIAsString();

		virtual std::uint32_t GetMaxImageLayers() const = 0;
		virtual std::uint32_t GetMaxViewportsCount() const = 0;
	private:
		static RenderAPI::API m_sRenderAPI;
	protected:
		static void SetCurrentFrameIndex(std::uint32_t frameIndex);
		static std::uint32_t m_sCurrentFrameIndex;
		static SystemsRequirements m_sSystemsRequirements;
	protected:
		static render::SubmitedSceneRenderData m_sSubmitedSceneRenderData;
		// Where std::size_t is SharedPointer<RenderPipeline> hash - > Submited instances.
		static std::unordered_map<std::size_t, render::SubmitedInstances> m_sSubmitedPipelines;
		static RenderAPI::SceneRenderData m_sSceneRenderData;
		static RenderAPI::RenderSettings  m_sRenderSettings;

		friend class Renderer;

	};

#ifndef SCENE_RENDER_DATA_SIZE
	#define SCENE_RENDER_DATA_SIZE (sizeof(RenderAPI::SceneRenderData))
#endif // SCENE_RENDER_DATA_SIZE
#ifndef RENDER_SETTINGS_DATA_SIZE
	#define RENDER_SETTINGS_DATA_SIZE (sizeof(RenderAPI::RenderSettings))
#endif // RENDER_SETTINGS_DATA_SIZE
}
