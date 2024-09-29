#include "shade_pch.h"
#include "Renderer.h"
#include <GLFW/glfw3.h>
#include <shade/core/application/Application.h>
#include <shade/utils/Utils.h>
// For BONE_TRANSFORMS DATA SIZE
#include <shade/core/animation/Animation.h>

shade::UniquePointer<shade::RenderAPI> shade::Renderer::m_sRenderAPI;
shade::UniquePointer<shade::RenderContext> shade::Renderer::m_sRenderContext;

std::vector<shade::PointLight::RenderData> shade::Renderer::m_sSubmitedPointLightRenderData;
std::vector<shade::SpotLight::RenderData>  shade::Renderer::m_sSubmitedSpotLightRenderData;

shade::SharedPointer<shade::Texture2D> shade::Renderer::m_sDefaultDiffuseTexture;
shade::SharedPointer<shade::Material> shade::Renderer::m_sDefaultMaterial;

shade::SharedPointer<shade::Texture2D>& shade::Renderer::GetDefaultDiffuseTexture()
{
	return m_sDefaultDiffuseTexture;
}

shade::SharedPointer<shade::Material>& shade::Renderer::GetDefaultMaterial()
{
	return m_sDefaultMaterial;
}

void shade::Renderer::Initialize(const RenderAPI::API& api, const SystemsRequirements& requirements)
{
	int status = glfwInit();
	if (!status)
		SHADE_CORE_ERROR("Could not initialize GLFW!");

	// Create and init Render api.
	m_sRenderAPI = RenderAPI::Create(api);
	// Create and init Render context.
	m_sRenderContext = m_sRenderAPI->Initialize(requirements);

	m_sRenderAPI->m_sSubmitedSceneRenderData.CameraBuffer				= UniformBuffer::Create(UniformBuffer::Usage::CPU_GPU, RenderAPI::CAMERA_BINDING, CAMERA_DATA_SIZE, GetFramesCount(), 0);
	m_sRenderAPI->m_sSubmitedSceneRenderData.SceneRenderDataBuffer		= UniformBuffer::Create(UniformBuffer::Usage::CPU_GPU, RenderAPI::SCENE_RENDER_DATA_BINDING, SCENE_RENDER_DATA_SIZE, GetFramesCount(), 0);
	m_sRenderAPI->m_sSubmitedSceneRenderData.RenderSettingsDataBuffer	= UniformBuffer::Create(UniformBuffer::Usage::CPU_GPU, RenderAPI::RENDER_SETTINGS_BINDING, RENDER_SETTINGS_DATA_SIZE, GetFramesCount(), 0);

	m_sRenderAPI->m_sSubmitedSceneRenderData.MaterialsBuffer			= StorageBuffer::Create(StorageBuffer::Usage::CPU_GPU, RenderAPI::MATERIAL_BINDING, MATERIAL_DATA_SIZE, GetFramesCount(), 50);
	// Make max 10 directional lights at this moment. 
	m_sRenderAPI->m_sSubmitedSceneRenderData.GlobalLightsBuffer			= StorageBuffer::Create(StorageBuffer::Usage::CPU_GPU, RenderAPI::GLOBAL_LIGHT_BINDING, GLOBAL_LIGHTS_DATA_SIZE(RenderAPI::MAX_GLOBAL_LIGHTS_COUNT), GetFramesCount(), 0);
	// Make max 100 poit lights at this moment. 
	m_sRenderAPI->m_sSubmitedSceneRenderData.PointsLightsBuffer			= StorageBuffer::Create(StorageBuffer::Usage::CPU_GPU, RenderAPI::POINT_LIGHT_BINDING,	POINT_LIGHTS_DATA_SIZE(RenderAPI::MAX_POINT_LIGHTS_COUNT), GetFramesCount(), 100);
	m_sRenderAPI->m_sSubmitedSceneRenderData.SpotLightsBuffer			= StorageBuffer::Create(StorageBuffer::Usage::CPU_GPU, RenderAPI::SPOT_LIGHT_BINDING,	SPOT_LIGHTS_DATA_SIZE(RenderAPI::MAX_SPOT_LIGHTS_COUNT), GetFramesCount(),   100);

	m_sRenderAPI->m_sSubmitedSceneRenderData.BoneTransfromsBuffer       = StorageBuffer::Create(StorageBuffer::Usage::CPU_GPU, RenderAPI::BONE_TRANSFORMS_BINDING,	BONE_TRANSFORM_DATA_SIZE, GetFramesCount(),  50);
	

	for (std::uint32_t i = 0; i < GetFramesCount(); i++)
	{
		m_sRenderAPI->m_sSubmitedSceneRenderData.TransformBuffers.emplace_back() = VertexBuffer::Create(VertexBuffer::Usage::CPU_GPU, TRANSFORM_DATA_SIZE, 50);
	}

	render::Image diffuseImage; diffuseImage.GenerateDiffuseTexture();
	m_sDefaultDiffuseTexture = Texture2D::CreateEXP(render::Image2D::Create(diffuseImage));
	render::Image normalImage; normalImage.GenerateDiffuseTexture();
	
	m_sDefaultMaterial = SharedPointer<Material>::Create();
}

std::uint32_t shade::Renderer::GetFramesCount()
{
	return m_sRenderAPI->GetFramesCount();
}

const std::uint32_t shade::Renderer::GetSubmitedSpotLightCount()
{
	return m_sSubmitedSpotLightRenderData.size();
}

const std::uint32_t shade::Renderer::GetSubmitedPointLightCount()
{
	return m_sSubmitedPointLightRenderData.size();
}

shade::RenderAPI::SceneRenderData& shade::Renderer::GetRenderData()
{
	return m_sRenderAPI->m_sSceneRenderData;
}

std::uint32_t shade::Renderer::GetMaxImageLayers()
{
	return m_sRenderAPI->GetMaxImageLayers();
}

std::uint32_t shade::Renderer::GetMaxViewportsCount()
{
	return m_sRenderAPI->GetMaxViewportsCount();
}

void shade::Renderer::ShutDown()
{
	// clear submitted pipelines and scene rendering data
	m_sRenderAPI->m_sSubmitedPipelines.clear();
	m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.clear();
	m_sRenderAPI->m_sSubmitedSceneRenderData.TransformBuffers.clear();
	m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData.clear();
	m_sRenderAPI->m_sSubmitedSceneRenderData.BoneOffsetsData.clear();

	m_sRenderAPI->m_sSubmitedSceneRenderData.MaterialsBuffer			= nullptr;
	m_sRenderAPI->m_sSubmitedSceneRenderData.CameraBuffer				= nullptr;
	m_sRenderAPI->m_sSubmitedSceneRenderData.SceneRenderDataBuffer		= nullptr;
	m_sRenderAPI->m_sSubmitedSceneRenderData.RenderSettingsDataBuffer	= nullptr;
	m_sRenderAPI->m_sSubmitedSceneRenderData.GlobalLightsBuffer			= nullptr;
	m_sRenderAPI->m_sSubmitedSceneRenderData.PointsLightsBuffer			= nullptr;
	m_sRenderAPI->m_sSubmitedSceneRenderData.SpotLightsBuffer			= nullptr;
	m_sRenderAPI->m_sSubmitedSceneRenderData.BoneTransfromsBuffer		= nullptr;

	m_sDefaultDiffuseTexture	= nullptr;
	m_sDefaultMaterial			= nullptr;

	ShaderLibrary::ShutDown();
	// shut down render API and context
	m_sRenderAPI->ShutDown();
	m_sRenderContext->ShutDown();

}

void shade::Renderer::BeginFrame(std::uint32_t frameIndex)
{
	m_sRenderAPI->BeginFrame(frameIndex);

	//// Don't work as expected !!
	//for (auto boneTransform = m_sRenderAPI->m_sSubmitedSceneRenderData.BoneOffsetsData.begin(); boneTransform != m_sRenderAPI->m_sSubmitedSceneRenderData.BoneOffsetsData.end();)
	//{
	//	bool hasFound = false;

	//	for (auto& [pipeline, instances] : m_sRenderAPI->m_sSubmitedPipelines)
	//	{
	//		std::size_t modelHash = boneTransform->first ^ pipeline;

	//		for (auto& [combinedHash, instance] : instances.Instances)
	//		{
	//			if (instance.second == modelHash)
	//			{
	//				hasFound = true; break;
	//			}
	//		}

	//		if (hasFound) break;
	//	}

	//	if (!hasFound)
	//	{
	//		boneTransform = m_sRenderAPI->m_sSubmitedSceneRenderData.BoneOffsetsData.erase(boneTransform);
	//	}
	//	else
	//	{
	//		++boneTransform;
	//	}
	//}

	// Iterate over GeometryBuffers inside the object m_sSubmitedSceneRenderData.
	for (auto drawable = m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.begin(); drawable != m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.end();)
	{
		// Initialize a flag called hasFound to false.
		bool hasFound = false;

		// Iterate over m_sSubmitedPipelines.
		for (auto& [pipeline, instances] : m_sRenderAPI->m_sSubmitedPipelines)
		{
			// If the Instances map inside the current pipeline contains a drawable with a matching key as the one we're currently iterating over.
			if (instances.Instances.find(drawable->first) != instances.Instances.end())
			{
				// Set the hasFound flag to true.
				hasFound = true;
				// Break out of the loop since we already found the drawable.
				break;
			}
		}

		// If we couldn't find the current drawable we're iterating over, then remove it from the map.
		if (!hasFound)
		{
			// Erase the current drawable from the map, and get the iterator of the next drawable.
			drawable = m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.erase(drawable);
		}
		else // Otherwise, move on to the next drawable.
		{
			++drawable;
		}
	}

	// Iterate over through all submitted transform and material data and calculate offset.
	std::uint32_t count = 0;
	for (auto& [hash, instance] : m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData)
	{
		// Set the transform and material offsets for each instance
		instance.TransformOffset = TRANSFORMS_DATA_SIZE(count);
		instance.MaterialOffset = MATERIALS_DATA_SIZE(count);

		for (auto i = 0; i < instance.Transforms.size(); ++i)
		{
			// Increment the count of transforms and materials.
			++count;
		}
	}
	// Resize the transform and materials buffers based on the number instances.
	m_sRenderAPI->m_sSubmitedSceneRenderData.TransformBuffers[frameIndex]->Resize(TRANSFORMS_DATA_SIZE(count));
	// Should be at least size 1
	m_sRenderAPI->m_sSubmitedSceneRenderData.MaterialsBuffer->Resize(MATERIALS_DATA_SIZE(count));

	for (auto& [hash, instance] : m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData)
	{
		// Set the transform and material data for each instance in the buffers
		m_sRenderAPI->m_sSubmitedSceneRenderData.TransformBuffers[frameIndex]->SetData(TRANSFORMS_DATA_SIZE(instance.Transforms.size()), instance.Transforms.data(), instance.TransformOffset);
		m_sRenderAPI->m_sSubmitedSceneRenderData.MaterialsBuffer->SetData(MATERIALS_DATA_SIZE(instance.Materials.size()), instance.Materials.data(), frameIndex, instance.MaterialOffset);
	}

	std::uint32_t bonesCount = 0;
	for (auto& [hash, boneData] : m_sRenderAPI->m_sSubmitedSceneRenderData.BoneOffsetsData)
	{
		boneData.PipelineModelOffset = BONE_TRANSFORMS_DATA_SIZE(bonesCount);

		for (auto i = 0; i < boneData.BoneTransforms.size(); ++i)
		{
			++bonesCount;
		}
	}

	m_sRenderAPI->m_sSubmitedSceneRenderData.BoneTransfromsBuffer->Resize(BONE_TRANSFORMS_DATA_SIZE(bonesCount));

	for (auto& [hash, boneData] : m_sRenderAPI->m_sSubmitedSceneRenderData.BoneOffsetsData)
	{
		for (std::uint32_t instance = 0; instance < boneData.BoneTransforms.size(); ++instance)
		{
			auto value = boneData.PipelineModelOffset + BONE_TRANSFORMS_DATA_SIZE(instance);

			m_sRenderAPI->m_sSubmitedSceneRenderData.BoneTransfromsBuffer->SetData(BONE_TRANSFORM_DATA_SIZE,
				boneData.BoneTransforms[instance]->data(), frameIndex, boneData.PipelineModelOffset + BONE_TRANSFORMS_DATA_SIZE(instance));
		}
	}
}

void shade::Renderer::EndFrame(std::uint32_t frameIndex)
{
	m_sRenderAPI->EndFrame(frameIndex);

	// Loop through submitted pipelines
	//for (auto& [hash, pipeline] : m_sRenderAPI->m_sSubmitedPipelines)
	//{
	//	// Async
	//	// Loop through instances of the pipeline in parallel
	//	std::for_each(std::execution::par, pipeline.Instances.begin(), pipeline.Instances.end(),
	//		[](std::pair<const std::size_t, render::MaterialModelPair>& drawable)
	//		{
	//			// Async
	//			// Loop through materials of the instance in parallel
	//			std::for_each(std::execution::par, drawable.second.Materials.begin(), drawable.second.Materials.end(),
	//			[](std::pair<const Asset<Material>, bool>& material)
	//				{
	//					// Mark the material as expired
	//					material.second = true;
	//				});

	//			//// Sync
	//			//// Loop through materials of the instance
	//			//for (auto& [material, expired] : drawable.second.Materials)
	//			//{
	//			//	// Mark the material as expired
	//			//	expired = true;
	//			//}

	//		});

	//	//// Sync
	//	//for (auto& drawable : pipeline.Instances)
	//	//{
	//	//	for (auto& [material, expired] : drawable.second.Materials)
	//	//		expired = true;
	//	//}

	//}

	// Clear all transform and material data.
	m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData.clear();
	m_sRenderAPI->m_sSubmitedSceneRenderData.BoneOffsetsData.clear();

	m_sSubmitedPointLightRenderData.clear();
	m_sSubmitedSpotLightRenderData.clear();

	m_sRenderAPI->m_sSceneRenderData.GlobalLightCount = 0;
	m_sRenderAPI->m_sSceneRenderData.PointsLightCount = 0;
	m_sRenderAPI->m_sSceneRenderData.SpotLightCount = 0;
	// Clear all submited instances, so make them as expired.
	// If no one will be submited again, so we remove instance geometry buffer at begin of the frame.
	for (auto& [pipeline, istnaces] : m_sRenderAPI->m_sSubmitedPipelines)
		istnaces.Instances.clear();
}

// TODO ADD FRAME INDEX INTO IT
void shade::Renderer::BeginScene(SharedPointer<Camera>& camera, const RenderAPI::RenderSettings& renderSettings, std::uint32_t frameIndex)
{
	Camera::RenderData cameraRenderData = camera->GetRenderData();

	// Is thats makes sense ?
	cameraRenderData.Near = (-cameraRenderData.Projection[3][2]);    
	cameraRenderData.Far  = (cameraRenderData.Projection[2][2]); 

	if (cameraRenderData.Near * cameraRenderData.Far < 0)
		cameraRenderData.Far = -cameraRenderData.Far;

	m_sRenderAPI->m_sRenderSettings = renderSettings;
	m_sRenderAPI->m_sSubmitedSceneRenderData.Camera = camera;
	m_sRenderAPI->m_sSubmitedSceneRenderData.CameraBuffer->SetData(CAMERA_DATA_SIZE, &cameraRenderData, frameIndex);
	m_sRenderAPI->m_sSubmitedSceneRenderData.SceneRenderDataBuffer->SetData(SCENE_RENDER_DATA_SIZE, &m_sRenderAPI->m_sSceneRenderData, frameIndex);
	m_sRenderAPI->m_sSubmitedSceneRenderData.RenderSettingsDataBuffer->SetData(RENDER_SETTINGS_DATA_SIZE, &m_sRenderAPI->m_sRenderSettings, frameIndex);
	m_sRenderAPI->m_sSubmitedSceneRenderData.PointsLightsBuffer->SetData(POINT_LIGHTS_DATA_SIZE(m_sRenderAPI->m_sSceneRenderData.PointsLightCount), m_sSubmitedPointLightRenderData.data(), frameIndex);
	m_sRenderAPI->m_sSubmitedSceneRenderData.SpotLightsBuffer->SetData(SPOT_LIGHTS_DATA_SIZE(m_sRenderAPI->m_sSceneRenderData.SpotLightCount), m_sSubmitedSpotLightRenderData.data(), frameIndex);

	
	m_sRenderAPI->BeginScene(camera, frameIndex);
}

void shade::Renderer::EndScene(std::uint32_t frameIndex)
{
	
	m_sRenderAPI->EndScene(frameIndex);
}

void shade::Renderer::BeginCompute(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex)
{
	pipeline->Begin(commandBuffer, frameIndex);
}

void shade::Renderer::EndCompute(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex, bool submit)
{
	// Uslees righ now and doesnt work !
	if (submit)
	{
		commandBuffer->End(frameIndex);
		commandBuffer->Submit(frameIndex);
	}
}

void shade::Renderer::BeginRender(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::uint32_t frameIndex, bool clear, std::uint32_t clearCount)
{
	m_sRenderAPI->BeginRender(commandBuffer, pipeline, frameIndex, clear, clearCount);
}

void shade::Renderer::BeginRenderWithCustomomViewPort(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::uint32_t frameIndex, glm::vec2 viewPort, bool clear)
{
	m_sRenderAPI->BeginRenderWithCustomomViewPort(commandBuffer, pipeline, frameIndex, viewPort, clear);
}

void shade::Renderer::EndRender(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex)
{
	m_sRenderAPI->EndRender(commandBuffer, frameIndex);
}

void shade::Renderer::DrawInstanced(SharedPointer<RenderCommandBuffer>& commandBuffer, const SharedPointer<VertexBuffer>& vertices, const SharedPointer<IndexBuffer>& indices, const SharedPointer<VertexBuffer>& transforms, const SharedPointer<VertexBuffer>& bones, std::uint32_t count, std::uint32_t transformOffset)
{
	m_sRenderAPI->DrawInstanced(commandBuffer, vertices, indices, transforms, count, transformOffset);
}
//DrawSubmitedInstancedAnimated
void shade::Renderer::DrawSubmitedInstanced(SharedPointer<RenderCommandBuffer>& commandBuffer, const SharedPointer<RenderPipeline>& pipeline, std::size_t instance, std::size_t material, std::uint32_t frameIndex, std::size_t lod, std::uint32_t splitOffset)
{
	const std::size_t hashCombined = render::PointerHashCombine(pipeline, instance, material, lod, splitOffset);

	auto rawData = m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData.find(hashCombined);
	
	// In case we are iterating through split offsets this is ok when there is no entry
	if (rawData != m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData.end())
	{
		m_sRenderAPI->DrawInstanced(commandBuffer,
			m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.at(instance)[lod].VB,
			m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.at(instance)[lod].IB,
			m_sRenderAPI->m_sSubmitedSceneRenderData.TransformBuffers[frameIndex],
			rawData->second.Transforms.size(), rawData->second.TransformOffset);
	}
}
void shade::Renderer::DrawSubmitedInstancedAnimated(SharedPointer<RenderCommandBuffer>& commandBuffer, const SharedPointer<RenderPipeline>& pipeline, std::size_t instance, std::size_t material, std::uint32_t frameIndex, std::size_t lod, std::uint32_t splitOffset)
{
	const std::size_t hashCombined = render::PointerHashCombine(pipeline, instance, material, lod, splitOffset);

	auto rawData = m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData.find(hashCombined);

	// In case we are iterating through split offsets this is ok when there is no entry
	if (rawData != m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData.end())
	{
		// Draw as animated with bone data.
		m_sRenderAPI->DrawInstancedAnimated(commandBuffer,
			m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.at(instance)[lod].VB,
			m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.at(instance)[lod].IB,
			m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.at(instance)[lod].BW,
			m_sRenderAPI->m_sSubmitedSceneRenderData.TransformBuffers[frameIndex],
			rawData->second.Transforms.size(), rawData->second.TransformOffset);
	}
}

void shade::Renderer::SubmitStaticMesh(const SharedPointer<RenderPipeline>& pipeline, const Asset<Drawable>& drawable, const Asset<Material>& material, const Asset<Model>& model, const glm::mat4& transform, std::uint32_t splitOffset)
{
	const std::size_t lod = 0;
	// Calculates a hash value based on the pipeline, drawable, and material and stores it in a variable
	const std::size_t combinedHash = render::PointerHashCombine(pipeline, drawable, material, lod, splitOffset);

	// Add transform and material to the instance raw data for the given combined hash
	m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData[combinedHash].Transforms.emplace_back(transform);
	m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData[combinedHash].Materials.emplace_back((material) ? material->GetRenderData() : GetDefaultMaterial()->GetRenderData());

	// Add the material and model hash to the instances for the given pipeline and drawable
	m_sRenderAPI->m_sSubmitedPipelines[pipeline].Instances[drawable].Materials.insert({ lod, material });
	m_sRenderAPI->m_sSubmitedPipelines[pipeline].Instances[drawable].ModelHash = (model) ? model : 0u;

	// If the drawable is not already in the geometry buffers, create instanced geometry buffers for each level of detail
	if (m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.find(drawable) == m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.end()) 
	{
		for (std::size_t i = 0; i < Drawable::MAX_LEVEL_OF_DETAIL; i++)
			CreateInstancedGeometryBuffer(drawable, i);
	}
}

void shade::Renderer::SubmitStaticMesh(const SharedPointer<RenderPipeline>& pipeline, const SharedPointer<Drawable>& drawable, const Asset<Material>& material, const SharedPointer<Model>& model, const glm::mat4& transform, std::uint32_t splitOffset)
{
	const std::size_t lod = 0;
	// Calculates a hash value based on the pipeline, drawable, and material and stores it in a variable
	const std::size_t combinedHash = render::PointerHashCombine(pipeline, drawable, material, lod, splitOffset);

	// Add transform and material to the instance raw data for the given combined hash
	m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData[combinedHash].Transforms.emplace_back(transform);
	m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData[combinedHash].Materials.emplace_back((material) ? material->GetRenderData() : GetDefaultMaterial()->GetRenderData());

	// Add the material and model hash to the instances for the given pipeline and drawable
	m_sRenderAPI->m_sSubmitedPipelines[pipeline].Instances[drawable].Materials.insert({ lod, material });
	m_sRenderAPI->m_sSubmitedPipelines[pipeline].Instances[drawable].ModelHash = (model) ? model : 0u;

	// If the drawable is not already in the geometry buffers, create instanced geometry buffers for each level of detail
	if (m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.find(drawable) == m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.end())
	{
		for (std::size_t i = 0; i < Drawable::MAX_LEVEL_OF_DETAIL; i++)
			CreateInstancedGeometryBuffer(drawable, i);
	}
}

void shade::Renderer::SubmitStaticMeshDynamicLOD(const SharedPointer<RenderPipeline>& pipeline, const Asset<Drawable>& drawable, const Asset<Material>& material, const Asset<Model>& model, const glm::mat4& transform, std::uint32_t splitOffset)
{
	const std::size_t lod = m_sRenderAPI->m_sSubmitedSceneRenderData.Camera ? GetLodLevelBasedOnDistance(m_sRenderAPI->m_sSubmitedSceneRenderData.Camera, Drawable::MAX_LEVEL_OF_DETAIL, transform, glm::vec3(0), glm::vec3(0)) : 0;
	// Calculates a hash value based on the pipeline, drawable, and material and stores it in a variable
	const std::size_t combinedHash = render::PointerHashCombine(pipeline, drawable, material, lod, splitOffset);

	// Add transform and material to the instance raw data for the given combined hash
	m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData[combinedHash].Transforms.emplace_back(transform);
	m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData[combinedHash].Materials.emplace_back((material) ? material->GetRenderData() : GetDefaultMaterial()->GetRenderData());

	// Add the material and model hash to the instances for the given pipeline and drawable
	m_sRenderAPI->m_sSubmitedPipelines[pipeline].Instances[drawable].Materials.insert({ lod, material });
	m_sRenderAPI->m_sSubmitedPipelines[pipeline].Instances[drawable].ModelHash = (model) ? model : 0u;

	// If the drawable is not already in the geometry buffers, create instanced geometry buffers for each level of detail
	if (m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.find(drawable) == m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.end())
	{
		for (std::size_t i = 0; i < Drawable::MAX_LEVEL_OF_DETAIL; i++)
			CreateInstancedGeometryBuffer(drawable, i);
	}
}

void shade::Renderer::SubmitStaticMeshDynamicLOD(const SharedPointer<RenderPipeline>& pipeline, const SharedPointer<Drawable>& drawable, const Asset<Material>& material, const SharedPointer<Model>& model, const glm::mat4& transform, std::uint32_t splitOffset)
{
	const std::size_t lod = m_sRenderAPI->m_sSubmitedSceneRenderData.Camera ? GetLodLevelBasedOnDistance(m_sRenderAPI->m_sSubmitedSceneRenderData.Camera, Drawable::MAX_LEVEL_OF_DETAIL, transform, glm::vec3(0), glm::vec3(0)) : 0;
	// Calculates a hash value based on the pipeline, drawable, and material and stores it in a variable
	const std::size_t combinedHash = render::PointerHashCombine(pipeline, drawable, material, lod, splitOffset);

	// Add transform and material to the instance raw data for the given combined hash
	m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData[combinedHash].Transforms.emplace_back(transform);
	m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData[combinedHash].Materials.emplace_back((material) ? material->GetRenderData() : GetDefaultMaterial()->GetRenderData());

	// Add the material and model hash to the instances for the given pipeline and drawable
	m_sRenderAPI->m_sSubmitedPipelines[pipeline].Instances[drawable].Materials.insert({ lod, material });
	m_sRenderAPI->m_sSubmitedPipelines[pipeline].Instances[drawable].ModelHash = (model) ? model : 0u;

	// If the drawable is not already in the geometry buffers, create instanced geometry buffers for each level of detail
	if (m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.find(drawable) == m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers.end())
	{
		for (std::size_t i = 0; i < Drawable::MAX_LEVEL_OF_DETAIL; i++)
			CreateInstancedGeometryBuffer(drawable, i);
	}
}

bool shade::Renderer::ExecuteSubmitedRenderPipeline(SharedPointer<RenderPipeline> pipeline, std::uint32_t frameIndex, bool isForceClear)
{
	// Search for the submitted pipelines map
	auto search = m_sRenderAPI->m_sSubmitedPipelines.find(pipeline);
	// If the pipeline is found in the submitted pipelines map
	if (search != m_sRenderAPI->m_sSubmitedPipelines.end())
	{
		// Process the pipeline
		pipeline->Process(pipeline, search->second, m_sRenderAPI->m_sSubmitedSceneRenderData, frameIndex, isForceClear);
		return true;
	}
	// If the pipeline is not found, return false
	return false;
}

bool shade::Renderer::ExecuteComputePipeline(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex)
{
	pipeline->Process(pipeline, frameIndex);
	return true;
}

void shade::Renderer::SubmitLight(const SharedPointer<GlobalLight>& light, const glm::mat4& transform, const SharedPointer<Camera>& camera)
{
	// Get direction vector from rotation !
																					/* Forward direction */
	auto renderData = light->GetRenderData(glm::normalize(glm::mat3(transform) * glm::vec3(0.f, 0.f, 1.f)), camera);
	
	assert(RenderAPI::MAX_GLOBAL_LIGHTS_COUNT >= m_sRenderAPI->m_sSceneRenderData.GlobalLightCount + 1, "Current directional light count > RenderAPI::MAX_DIRECTIONAL_LIGHTS_COUNT");

	m_sRenderAPI->m_sSubmitedSceneRenderData.GlobalLightsBuffer->SetData(
		GLOBAL_LIGHT_DATA_SIZE, &renderData, GetCurrentFrameIndex(),
		GLOBAL_LIGHT_DATA_SIZE * m_sRenderAPI->m_sSceneRenderData.GlobalLightCount++);
}

void shade::Renderer::SubmitLight(const SharedPointer<PointLight>& light, const glm::mat4& transform, const SharedPointer<Camera>& camera)
{
	auto renderData = light->GetRenderData(Transform::GetTransformFromMatrix(transform).GetPosition(), camera);

	assert(RenderAPI::MAX_POINT_LIGHTS_COUNT >= m_sRenderAPI->m_sSceneRenderData.PointsLightCount + 1, "Current point light count > RenderAPI::MAX_POINT_LIGHTS_COUNT");

	m_sSubmitedPointLightRenderData.emplace_back(renderData);
	m_sRenderAPI->m_sSceneRenderData.PointsLightCount++;

}

void shade::Renderer::SubmitLight(const SharedPointer<SpotLight>& light, const glm::mat4& transform, const SharedPointer<Camera>& camera)
{																																/* Forward direction */
	auto renderData = light->GetRenderData(Transform::GetTransformFromMatrix(transform).GetPosition(), glm::normalize(glm::mat3(transform) * glm::vec3(0.f, 0.f, 1.f)), camera);

	assert(RenderAPI::MAX_SPOT_LIGHTS_COUNT >= m_sRenderAPI->m_sSceneRenderData.SpotLightCount + 1, "Current spot light count > RenderAPI::MAX_SPOT_LIGHTS_COUNT");
	m_sSubmitedSpotLightRenderData.emplace_back(renderData);
	m_sRenderAPI->m_sSceneRenderData.SpotLightCount++;
}

void shade::Renderer::UpdateSubmitedMaterial(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, const Asset<Drawable>& instance, const Asset<Material>& material, std::uint32_t frameIndex, std::size_t lod)
{
	UpdateSubmitedMaterial(commandBuffer, pipeline, static_cast<std::size_t>(instance), material, frameIndex, lod);
}

void shade::Renderer::UpdateSubmitedMaterial(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::size_t instance, const Asset<Material>& material, std::uint32_t frameIndex, std::size_t lod)
{
	// Combines the hash values of pipeline, instance, and material using a custom hash function
	std::size_t combinedHash = render::PointerHashCombine(pipeline, instance, material, lod);
	// Searches for the combinedHash in the map containing instance raw data
	auto rawData = m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData.find(combinedHash);
	// If the rawData is found in the map
	if (rawData != m_sRenderAPI->m_sSubmitedSceneRenderData.InstanceRawData.end())
	{
		// Assigns the material stored in the buffer associated with the rawData
		// to the graphic pipeline
		pipeline->SetMaterial(m_sRenderAPI->m_sSubmitedSceneRenderData.MaterialsBuffer, rawData->second.MaterialOffset, material, frameIndex);
		// Updates graphics resources associated with the pipeline
		pipeline->UpdateResources(commandBuffer, frameIndex);
	}
}

void shade::Renderer::SubmitBoneTransforms(const SharedPointer<RenderPipeline>& pipeline, const Asset<Model>& instance, const SharedPointer<std::vector<glm::mat4>>& transform)
{
	std::size_t combinedHash = render::PointerHashCombine(pipeline, instance);
	m_sRenderAPI->m_sSubmitedSceneRenderData.BoneOffsetsData[combinedHash].BoneTransforms.emplace_back(transform);
}

void shade::Renderer::UpdateSubmitedBonesData(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::size_t modelInstance, std::uint32_t frameIndex)
{
	std::size_t combinedHash = render::PointerHashCombine(pipeline, modelInstance);
	auto rawData = m_sRenderAPI->m_sSubmitedSceneRenderData.BoneOffsetsData.find(combinedHash);
	if (rawData != m_sRenderAPI->m_sSubmitedSceneRenderData.BoneOffsetsData.end())
	{
		pipeline->SetResource(m_sRenderAPI->m_sSubmitedSceneRenderData.BoneTransfromsBuffer, Pipeline::Set::PerInstance, frameIndex, rawData->second.PipelineModelOffset);
	}
}

void shade::Renderer::BeginTimestamp(SharedPointer<RenderCommandBuffer>& commandBuffer, const std::string& name)
{
	m_sRenderAPI->BeginTimestamp(commandBuffer, name);
}

float shade::Renderer::EndTimestamp(SharedPointer<RenderCommandBuffer>& commandBuffer, const std::string& name)
{
	return m_sRenderAPI->EndTimestamp(commandBuffer, name);
}

std::size_t shade::Renderer::GetLodLevelBasedOnDistance(const SharedPointer<Camera> camera, std::size_t lodsCount, const glm::mat4& transform, const glm::vec3& minHaflExt, const glm::vec3& maxHalfExt)
{
	const float currentDistance = glm::distance(camera->GetPosition(), glm::vec3(transform[3][0], transform[3][1], transform[3][2]));
	// Calculate the depth range.
	float depthRange = camera->GetFar() - camera->GetNear();

	// Calculate the depth interval for each split.
	float splitInterval = depthRange / lodsCount;

	// Calculate the normalized distance within the range [0, 1].
	float normalizedDistance = (currentDistance - camera->GetNear()) / depthRange;

	// Calculate the split index using linear interpolation.
	std::size_t splitIndex = static_cast<size_t>(floor(normalizedDistance * lodsCount));

	// Ensure the split index is within the valid range.
	splitIndex = std::max(std::size_t(0), std::min(splitIndex, lodsCount - 1));

	return splitIndex;
}

std::uint32_t shade::Renderer::GetCurrentFrameIndex()
{
	return m_sRenderAPI->GetCurrentFrameIndex();
}

shade::UniquePointer<shade::SwapChain>& shade::Renderer::GetSwapChain()
{
	return Application::GetWindow()->GetSwapChain();
}

const shade::SpotLight::RenderData& shade::Renderer::GetSubmitedSpotLightRenderData(std::uint32_t lightIndex)
{
	assert(lightIndex < m_sSubmitedSpotLightRenderData.size() && "lightIndex > current spot light count");
	return m_sSubmitedSpotLightRenderData[lightIndex];
}

const shade::PointLight::RenderData& shade::Renderer::GetSubmitedPointLightRenderData(std::uint32_t lightIndex)
{
	assert(lightIndex < m_sSubmitedPointLightRenderData.size() && "lightIndex > current point light count");
	return m_sSubmitedPointLightRenderData[lightIndex];
}

// TODO: Probably we can keep only asset funciton during to SharedPointer can be converted into Asset ?
void shade::Renderer::CreateInstancedGeometryBuffer(const Asset<Drawable>& drawable, std::size_t lod)
{
	// Get the geometry buffer of the current drawable object
	auto& buffer = m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers[drawable];
	
	const auto& vertices	= drawable->GetLod((lod) ? lod : 0).Vertices;
	const auto& indices		= drawable->GetLod((lod) ? lod : 0).Indices;
	const auto& bones		= drawable->GetLod((lod) ? lod : 0).Bones;

	if (vertices.size() && indices.size())
	{
		// Create a vertex buffer for the drawable object's vertices
		buffer[lod].VB = VertexBuffer::Create(VertexBuffer::Usage::GPU, VERTICES_DATA_SIZE(vertices.size()), 0, vertices.data());
		// Create an index buffer for the drawable object's indices
		buffer[lod].IB = IndexBuffer::Create(IndexBuffer::Usage::GPU, INDICES_DATA_SIZE(indices.size()), 0, indices.data());

		if (bones.size())
			buffer[lod].BW = VertexBuffer::Create(VertexBuffer::Usage::GPU, BONES_DATA_SIZE(bones.size()), 0, bones.data());
	}
}

void shade::Renderer::CreateInstancedGeometryBuffer(const SharedPointer<Drawable>& drawable, std::size_t lod)
{
	// Get the geometry buffer of the current drawable object
	auto& buffer = m_sRenderAPI->m_sSubmitedSceneRenderData.GeometryBuffers[drawable];

	const auto& vertices	= drawable->GetLod((lod) ? lod : 0).Vertices;
	const auto& indices		= drawable->GetLod((lod) ? lod : 0).Indices;
	const auto& bones		= drawable->GetLod((lod) ? lod : 0).Bones;

	if (vertices.size() && indices.size())
	{
		// Create a vertex buffer for the drawable object's vertices
		buffer[lod].VB = VertexBuffer::Create(VertexBuffer::Usage::GPU, VERTICES_DATA_SIZE(vertices.size()), 0, vertices.data());
		// Create an index buffer for the drawable object's indices
		buffer[lod].IB = IndexBuffer::Create(IndexBuffer::Usage::GPU, INDICES_DATA_SIZE(indices.size()), 0, indices.data());

		if (bones.size())
			buffer[lod].BW = VertexBuffer::Create(VertexBuffer::Usage::GPU, BONES_DATA_SIZE(bones.size()), 0, bones.data());
	}
}

