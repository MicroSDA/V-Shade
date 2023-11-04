#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/render/RenderAPI.h>
#include <shade/core/render/RenderContext.h>
#include <shade/core/render/shader/Shader.h>
#include <shade/core/render/SwapChain.h>
#include <shade/core/render/buffers/RenderCommandBuffer.h>
#include <shade/core/environment/GlobalLight.h>
#include <shade/core/environment/PointLight.h>
#include <shade/core/environment/SpotLight.h>

namespace shade
{
	class SHADE_API Renderer
	{
	public:
		// TODO: Add const As<> function to all
		static SharedPointer<Texture2D>& GetDefaultDiffuseTexture();
		static SharedPointer<Material>&  GetDefaultMaterial();
	public:

		// Initializes a rendering API and sets the required system specifications to be met
		static void Initialize(const RenderAPI::API& api = RenderAPI::API::None, const SystemsRequirements& requirements = SystemsRequirements{});
		static void ShutDown();

		static void BeginFrame(std::uint32_t frameIndex);
		static void EndFrame(std::uint32_t frameIndex);

		static void BeginScene(SharedPointer<Camera>& camera, const RenderAPI::RenderSettings& renderSettings, std::uint32_t frameIndex);
		static void EndScene(std::uint32_t frameIndex);

		static void BeginCompute(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex);
		static void EndCompute(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex, bool submit = false);

		static void BeginRender(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::uint32_t frameIndex, bool clear, std::uint32_t clearCount = 0);

		static void BeginRenderWithCustomomViewPort(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::uint32_t frameIndex, glm::vec2 viewPort, bool clear);


		static void EndRender(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex);

		static void DrawInstanced(SharedPointer<RenderCommandBuffer>& commandBuffer, const SharedPointer<VertexBuffer>& vertices, const SharedPointer<IndexBuffer>& indices, const SharedPointer<VertexBuffer>& transforms, std::uint32_t count, std::uint32_t transformOffset = 0);
		static void DrawSubmitedInstanced(SharedPointer<RenderCommandBuffer>& commandBuffer, const SharedPointer<RenderPipeline>& pipeline, std::size_t instance, std::size_t material, std::uint32_t frameIndex, std::size_t lod = 0, std::uint32_t splitOffset = 0);

		static void SubmitStaticMesh(const SharedPointer<RenderPipeline>& pipeline, const Asset<Drawable>& drawable, const Asset<Material>& material, const glm::mat4& transform, std::uint32_t splitOffset = 0);
		static void SubmitStaticMesh(const SharedPointer<RenderPipeline>& pipeline, const SharedPointer<Drawable>& drawable, const Asset<Material>& material, const glm::mat4& transform, std::uint32_t splitOffset = 0);

		static void SubmitStaticMeshDynamicLOD(const SharedPointer<RenderPipeline>& pipeline, const Asset<Drawable>& drawable, const Asset<Material>& material, const glm::mat4& transform, std::uint32_t splitOffset = 0);
		static void SubmitStaticMeshDynamicLOD(const SharedPointer<RenderPipeline>& pipeline, const SharedPointer<Drawable>& drawable, const Asset<Material>& material, const glm::mat4& transform, std::uint32_t splitOffset = 0);

		static bool ExecuteSubmitedRenderPipeline(SharedPointer<RenderPipeline>& pipeline, std::uint32_t frameIndex);
		static bool ExecuteComputePipeline(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex);

		static void SubmitLight(const SharedPointer<GlobalLight>& light, const glm::mat4& transform, const SharedPointer<Camera>& camera);
		static void SubmitLight(const SharedPointer<SpotLight>& light, const glm::mat4& transform, const SharedPointer<Camera>& camera);
		static void SubmitLight(const SharedPointer<PointLight>& light, const glm::mat4& transform, const SharedPointer<Camera>& camera);
		
		static void UpdateSubmitedMaterial(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, const Asset<Drawable>& instance, const Asset<Material>& material, std::uint32_t frameIndex, std::size_t lod = 0);
		static void UpdateSubmitedMaterial(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::size_t instance, const Asset<Material>& material, std::uint32_t frameIndex, std::size_t lod = 0);
		

		static const SpotLight::RenderData& GetSubmitedSpotLightRenderData(std::uint32_t lightIndex);
		static const std::uint32_t GetSubmitedSpotLightCount();

		static const PointLight::RenderData& GetSubmitedPointLightRenderData(std::uint32_t lightIndex);
		static const std::uint32_t GetSubmitedPointLightCount();

		static void  BeginTimestamp(SharedPointer<RenderCommandBuffer>& commandBuffer, const std::string& name);
		static float EndTimestamp(SharedPointer<RenderCommandBuffer>& commandBuffer, const std::string& name);
		
		static std::size_t GetLodLevelBasedOnDistance(const SharedPointer<Camera> camera, std::size_t lodsCount, const glm::mat4& transform, const glm::vec3& minHaflExt, const glm::vec3& maxHalfExt);

		static std::uint32_t GetCurrentFrameIndex();
		static UniquePointer<SwapChain>& GetSwapChain();
		static std::uint32_t GetFramesCount();

		static RenderAPI::SceneRenderData& GetRenderData();

		static std::uint32_t GetMaxImageLayers();
		static std::uint32_t GetMaxViewportsCount();
	protected:

	private:
		void static CreateInstancedGeometryBuffer(const Asset<Drawable>& drawable, std::size_t lod = 0);
		void static CreateInstancedGeometryBuffer(const SharedPointer<Drawable>& drawable, std::size_t lod = 0);
	private:
		static UniquePointer<RenderAPI> m_sRenderAPI;
		static UniquePointer<RenderContext> m_sRenderContext;	
		static std::vector<PointLight::RenderData> m_sSubmitedPointLightRenderData;
		static std::vector<SpotLight::RenderData> m_sSubmitedSpotLightRenderData;
	private:
		static SharedPointer<Texture2D> m_sDefaultDiffuseTexture;
		static SharedPointer<Material>  m_sDefaultMaterial;
	public:


	};
}

