#include "shade_pch.h"
#include "SceneRenderer.h"
#include <shade/core/event/Input.h>
#include <shade/core/application/Application.h>

#include <glm/glm/gtx/hash.hpp>

#include <shade/core/layer/imgui/ImGuiLayer.h>

shade::SharedPointer<shade::SceneRenderer> shade::SceneRenderer::Create(bool swapChainAsMainTarget)
{
	return SharedPointer<SceneRenderer>::Create(swapChainAsMainTarget);
}

shade::SceneRenderer::SceneRenderer(bool swapChainAsMainTarget)
{
	if (swapChainAsMainTarget)
		m_MainCommandBuffer = RenderCommandBuffer::CreateFromSwapChain();
	else
		m_MainCommandBuffer = RenderCommandBuffer::Create(RenderCommandBuffer::Type::Primary, RenderCommandBuffer::Family::Graphic, Renderer::GetFramesCount());

	VertexBuffer::Layout mainGeometryVertexlayoutStatic =
	{
		{
			VertexBuffer::Layout::Usage::PerVertex,
			{
				{ "a_Position",			 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_Normal",			 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_Tangent",			 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_Bitangent",		 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_UV_Coordinates",	 Shader::DataType::Float2, VertexBuffer::Layout::Usage::PerVertex},
			}
		},
		{
			VertexBuffer::Layout::Usage::PerInstance,
			{
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance},
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance},
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance},
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance}
			}
		}
	};
		
	VertexBuffer::Layout mainGeometryVertexlayoutAnimated =
	{
		{
			VertexBuffer::Layout::Usage::PerVertex,
			{
				{ "a_Position",			 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_Normal",			 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_Tangent",			 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_Bitangent",		 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_UV_Coordinates",	 Shader::DataType::Float2, VertexBuffer::Layout::Usage::PerVertex},
			}
		},
		{
			VertexBuffer::Layout::Usage::PerVertex,
			{
				{ "a_BoneIds",			 Shader::DataType::Int4,   VertexBuffer::Layout::Usage::PerVertex},
				{ "a_BoneWeights",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerVertex},
			}
		},
		{
			VertexBuffer::Layout::Usage::PerInstance,
			{
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance},
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance},
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance},
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance}
			}
		}
	};

	VertexBuffer::Layout skeletonVisualizinglayout =
	{
		{
			VertexBuffer::Layout::Usage::PerInstance,
			{
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance},
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance},
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance},
				{ "a_Transform",		 Shader::DataType::Float4, VertexBuffer::Layout::Usage::PerInstance}
			}
		}
	};

	VertexBuffer::Layout gridVertexlayout =
	{
		{
			VertexBuffer::Layout::Usage::PerVertex,
			{
				{ "a_Position",			 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_Normal",			 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_Tangent",			 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_Bitangent",		 Shader::DataType::Float3, VertexBuffer::Layout::Usage::PerVertex},
				{ "a_UV_Coordinates",	 Shader::DataType::Float2, VertexBuffer::Layout::Usage::PerVertex},
			}
		}
	};

	if (swapChainAsMainTarget)
		m_MainTargetFrameBuffer = FrameBuffer::CreateFromSwapChain();
	else
	{
		m_MainTargetFrameBuffer = { Renderer::GetFramesCount(), FrameBuffer::Create({ 1000, 1000, {0.05f, 0.05f, 0.05f, 1.f}, 1.f, { { render::Image::Format::RGBA32F}, { render::Image::Format::RGBA32F}, { render::Image::Format::RGBA32F}, { render::Image::Format::DEPTH32F } } }) };
	}
	
	m_LightCullingPreDepthFrameBuffer	= FrameBuffer::Create({ 1, 1, {0.0f, 0.0f, 0.0f, 1.f}, 1.f, { { render::Image::Format::DEPTH24STENCIL8 } } });
	m_GlobalLightShadowFrameBuffer		= FrameBuffer::Create({ 6000, 6000, {0.0f, 0.0f, 0.0f, 1.f}, 1.f, { { render::Image::Format::Depth, render::Image::Usage::Attachment, render::Image::Clamp::REPEAT, 1, GlobalLight::SHADOW_CASCADES_COUNT } } });
	m_SpotLightShadowFrameBuffer		= FrameBuffer::Create({ 2000, 2000, {0.0f, 0.0f, 0.0f, 1.f}, 1.f, { { render::Image::Format::Depth, render::Image::Usage::Attachment, render::Image::Clamp::REPEAT, 1,  RenderAPI::MAX_SPOT_SHADOW_CASTERS }} });
	m_PointLightShadowFrameBuffer		= FrameBuffer::Create({ 1000, 1000, {0.0f, 0.0f, 0.0f, 1.f}, 1.f, { { render::Image::Format::Depth, render::Image::Usage::Attachment, render::Image::Clamp::REPEAT,1,  RenderAPI::MAX_POINT_SHADOW_CASTERS * 6, 1, 1, true}} });
	m_BloomTarget						= Texture2D::CreateEXP({ render::Image::Format::RGBA32F, render::Image::Usage::Storage, render::Image::Clamp::CLAMP_TO_EDGE, m_Settings.BloomSettings.Samples, 1, 200, 200 });
	m_ScreenSpaceAmbientOcclusionTarget	= Texture2D::CreateEXP({ render::Image::Format::RED8UN, render::Image::Usage::Storage, render::Image::Clamp::REPEAT, 1, 1, 200, 200 });
	// TODO: Should it be like some internal part of renderer ?
	m_VisibleSpotLightIndicesBuffer		= StorageBuffer::Create(StorageBuffer::Usage::GPU, RenderAPI::SPOT_LIGHT_INDINCES_BINDING, sizeof(std::uint32_t) * RenderAPI::MAX_SPOT_LIGHTS_COUNT, Renderer::GetFramesCount(), 20);
	m_VisiblePointLightIndicesBuffer	= StorageBuffer::Create(StorageBuffer::Usage::GPU, RenderAPI::POINT_LIGHT_INDINCES_BINDING, sizeof(std::uint32_t) * RenderAPI::MAX_POINT_LIGHTS_COUNT, Renderer::GetFramesCount(), 20);
	m_SSAOSamplesBuffer					= UniformBuffer::Create(UniformBuffer::Usage::CPU_GPU, 4, sizeof(SSAO::RenderBuffer), Renderer::GetFramesCount(), 0);

	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Skeleton-Bone-Visualizing",
			.Shader = ShaderLibrary::Create("Skeleton-Bone-Visualizing", "./resources/assets/shaders/utils/SkeletonVisualizing.glsl"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = skeletonVisualizinglayout,
			.Topology = Pipeline::PrimitiveTopology::Point,
			.BackFalceCull = false,
			.LineWidth = 2.f	
		}));

	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Skeleton-Joint-Visualizing",
			.Shader = ShaderLibrary::Create("Lights-Visualizing", "./resources/assets/shaders/utils/LightsVisualizing.glsl"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.Topology = Pipeline::PrimitiveTopology::Triangle,
			.BackFalceCull = false,
			.LineWidth = 1.5f
		}));

	GetPipeline("Skeleton-Bone-Visualizing")->SetActive(false);

	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name			= "Main-Geometry-Static",
			.Shader			= ShaderLibrary::Create("Main-Geometry-Static", "./resources/assets/shaders/Main-Geometry-Static.glsl"),
			.FrameBuffer	= m_MainTargetFrameBuffer[0],
			.VertexLayout	= mainGeometryVertexlayoutStatic,
		}));

	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Main-Geometry-Animated",
			.Shader = ShaderLibrary::Create("Main-Geometry-Aniamted", "./resources/assets/shaders/Main-Geometry-Animated.glsl"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = mainGeometryVertexlayoutAnimated,
		}));
	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Light-Culling-Pre-Depth",
			.Shader = ShaderLibrary::Create("Light-Culling-Pre-Depth", "./resources/assets/shaders/preprocess/Tiled-Forward-Pre-Depth.glsl"),
			.FrameBuffer = m_LightCullingPreDepthFrameBuffer,
			.VertexLayout = mainGeometryVertexlayoutStatic
		}));

	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Global-Light-Shadow-Pre-Depth",
			.Shader = ShaderLibrary::Create("Global-Light-Shadow-Pre-Depth", "./resources/assets/shaders/preprocess/Global-Light-Shadow-Pre-Depth.glsl"),
			.FrameBuffer = m_GlobalLightShadowFrameBuffer,
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.BackFalceCull = false,
			.DepsBiasConstantFactor = 4.0f,
			.DepthBiasSlopeFactor = 8.0f,
		}));
	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Spot-Light-Shadow-Pre-Depth",
			.Shader = ShaderLibrary::Create("Spot-Light-Shadow-Pre-Depth", "./resources/assets/shaders/preprocess/Spot-Light-Shadow-Pre-Depth.glsl"),
			.FrameBuffer = m_SpotLightShadowFrameBuffer,
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.BackFalceCull = false,
			.DepsBiasConstantFactor = 4.0f,
			.DepthBiasSlopeFactor = 8.0f,
		}));
	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Point-Light-Shadow-Pre-Depth",
			.Shader = ShaderLibrary::Create("Point-Light-Shadow-Pre-Depth", "./resources/assets/shaders/preprocess/Point-Light-Shadow-Pre-Depth.glsl"),
			.FrameBuffer = m_PointLightShadowFrameBuffer,
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.BackFalceCull = false,
			/*.DepsBiasConstantFactor = 4.0f,
			.DepthBiasSlopeFactor = 8.0f,*/
		}));
	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Point-Lights-Visualizing",
			.Shader = ShaderLibrary::Create("Lights-Visualizing", "./resources/assets/shaders/utils/LightsVisualizing.glsl"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.Topology = Pipeline::PrimitiveTopology::Line,
			.BackFalceCull = false
		}));
	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Spot-Lights-Visualizing",
			.Shader = ShaderLibrary::Get("Lights-Visualizing"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.Topology = Pipeline::PrimitiveTopology::LineStrip,
			.BackFalceCull = false
		}));
	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Grid",
			.Shader = ShaderLibrary::Create("Grid", "./resources/assets/shaders/utils/Grid.glsl"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = gridVertexlayout,
			.Topology = Pipeline::PrimitiveTopology::TriangleStrip,
			.BackFalceCull = false
		}));
	RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "AABB_OBB",
			.Shader = ShaderLibrary::Create("StaticFlat", "./resources/assets/shaders/StaticFlat.glsl"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.Topology = Pipeline::PrimitiveTopology::Line,
			//.PolygonMode = Pipeline::PrimitivePolygonMode::Point,
			.LineWidth = 2.f
		}));
	RegisterNewPipeline(shade::ComputePipeline::Create(
		{
			.Name = "Light-Culling",
			.Shader = ShaderLibrary::Create("Light-Culling", "./resources/assets/shaders/preprocess/Tiled-Forward-Light-Culling.glsl"),
		}));
	RegisterNewPipeline(shade::ComputePipeline::Create(
		{
			.Name = "SSAO",
			.Shader = ShaderLibrary::Create("SSAO", "./resources/assets/shaders/postprocess/SSAO/Screen-Sapce-Ambien-Occlusion-Compute.glsl"),
		}));
	RegisterNewPipeline(shade::ComputePipeline::Create(
		{
			.Name = "Color-Correction",
			.Shader = ShaderLibrary::Create("Color-Correction", "./resources/assets/shaders/postprocess/Color-Correction.glsl"),
		}));
	RegisterNewPipeline(shade::ComputePipeline::Create(
		{
			.Name = "Bloom",
			.Shader = ShaderLibrary::Create("Bloom", "./resources/assets/shaders/postprocess/Bloom/Bloom-Compute.glsl"),
		}));

	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("Main-Geometry-Static")->As<RenderPipeline>(),   SceneRenderer, InstancedGeometryPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("Main-Geometry-Animated")->As<RenderPipeline>(), SceneRenderer, InstancedGeometryPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("Light-Culling-Pre-Depth")->As<RenderPipeline>(), SceneRenderer, LightCullingPreDepthPass, this);

	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("Global-Light-Shadow-Pre-Depth")->As<RenderPipeline>(), SceneRenderer, GlobalLightShadowPreDepthPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("Spot-Light-Shadow-Pre-Depth")->As<RenderPipeline>(), SceneRenderer, SpotLightShadowPreDepthPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("Point-Light-Shadow-Pre-Depth")->As<RenderPipeline>(), SceneRenderer, PointLightShadowPreDepthPass, this);
	
	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("Point-Lights-Visualizing")->As<RenderPipeline>(), SceneRenderer, LightVisualizationPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("Spot-Lights-Visualizing")->As<RenderPipeline>(), SceneRenderer, LightVisualizationPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("Grid")->As<RenderPipeline>(), SceneRenderer, GridPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("AABB_OBB")->As<RenderPipeline>(), SceneRenderer, FlatPipeline, this);

	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("Skeleton-Bone-Visualizing")->As<RenderPipeline>(), SceneRenderer, SkeletonVisualizationPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(GetPipeline("Skeleton-Joint-Visualizing")->As<RenderPipeline>(), SceneRenderer, LightVisualizationPass, this);

	BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(GetPipeline("Light-Culling")->As<ComputePipeline>(), SceneRenderer, LightCullingComputePass, this);
	BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(GetPipeline("SSAO")->As<ComputePipeline>(), SceneRenderer, SSAOComputePass, this);
	BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(GetPipeline("Color-Correction")->As<ComputePipeline>(), SceneRenderer, ColorCorrectionComputePass, this);
	BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(GetPipeline("Bloom")->As<ComputePipeline>(), SceneRenderer, BloomComputePass, this);


	m_CollisionPointsMaterial	= SharedPointer<Material>::Create();

	m_AABBMaterial				= SharedPointer<Material>::Create();
	m_AABBMaterial->ColorDiffuse = glm::vec3(0.3, 0.9, 0.2);

	m_OBBMaterial				= SharedPointer<Material>::Create();
	m_OBBMaterial->ColorDiffuse = glm::vec3(0.9, 0.3, 0.2);

	m_ConvexMeshMaterial		= SharedPointer<Material>::Create();

	m_LightVisualizingMaterial	= SharedPointer<Material>::Create();
	m_LightVisualizingMaterial->ColorAmbient = glm::vec3(0.459, 0.773, 1);

	m_JoinVisualizingMaterial   = SharedPointer<Material>::Create();
	m_JoinVisualizingMaterial->ColorAmbient = glm::vec3(0.9, 0.3, 0.3);

	m_Plane  = Plane::Create(10.f, 10.f, 1);
	m_OBB    = Box::Create({ -1.0f ,-1.0f ,-1.0f }, { 1.0f, 1.0f, 1.0f });
	m_Sphere = Sphere::Create(10, 10, 1.0f);
	m_Cone   = Cone::Create(1.f, 1.f, 20, 8.f, glm::vec3(0.0, 0.0, 1.0));
}

void shade::SceneRenderer::OnUpdate(SharedPointer<Scene>& scene, const shade::CameraComponent& camera, const FrameTimer& deltaTime, const ecs::Entity& activeEntity)
{
	m_Statistic.Reset(); const std::uint32_t currentFrame = Renderer::GetCurrentFrameIndex();

	if (camera)
	{
		m_Camera = camera;
	}
	else
	{
		ecs::Entity entity = scene->GetPrimaryCamera();
		if (entity.IsValid())
		{
			m_Camera = entity.GetComponent<CameraComponent>();
		}
	}

	if (m_Camera)
	{
		// Set camera aspect base on render target resolution
		m_Camera->SetAspect((float)m_MainTargetFrameBuffer[currentFrame]->GetWidth() / (float)m_MainTargetFrameBuffer[currentFrame]->GetHeight());

		if (GetPipeline("Light-Culling-Pre-Depth")->IsActive() && GetPipeline("Light-Culling")->IsActive())
		{
			if (m_LightCullingPreDepthFrameBuffer->GetWidth() != m_MainTargetFrameBuffer[currentFrame]->GetWidth() || m_LightCullingPreDepthFrameBuffer->GetHeight() != m_MainTargetFrameBuffer[currentFrame]->GetHeight())
			{
				m_LightCullingPreDepthFrameBuffer->Resize(m_MainTargetFrameBuffer[currentFrame]->GetWidth(), m_MainTargetFrameBuffer[currentFrame]->GetHeight());
			}
		}
		if (GetPipeline("Bloom")->IsActive())
		{
			if (m_BloomTarget->GetWidth() != m_MainTargetFrameBuffer[currentFrame]->GetWidth() ||
				m_BloomTarget->GetHeight() != m_MainTargetFrameBuffer[currentFrame]->GetHeight() ||
				m_BloomTarget->GetImage()->GetSpecification().MipLevels != m_Settings.BloomSettings.Samples)
			{
				m_BloomTarget->Resize(m_MainTargetFrameBuffer[currentFrame]->GetWidth(), m_MainTargetFrameBuffer[currentFrame]->GetHeight(), m_Settings.BloomSettings.Samples);
			}
		}
		if (GetPipeline("SSAO")->IsActive())
		{
			if (m_ScreenSpaceAmbientOcclusionTarget->GetWidth() != m_MainTargetFrameBuffer[currentFrame]->GetWidth() ||
				m_ScreenSpaceAmbientOcclusionTarget->GetHeight() != m_MainTargetFrameBuffer[currentFrame]->GetHeight())
			{
				m_ScreenSpaceAmbientOcclusionTarget->Resize(m_MainTargetFrameBuffer[currentFrame]->GetWidth(), m_MainTargetFrameBuffer[currentFrame]->GetHeight());
			}
		}

		CameraFrustum frustum = m_Camera->GetCameraFrustum();

		scene->View<GlobalLightComponent, TransformComponent>().Each([&](ecs::Entity& entity, GlobalLightComponent& light, TransformComponent& transform)
			{
				Renderer::SubmitLight(light, scene->ComputePCTransform(entity).first, m_Camera);
			});
		scene->View<PointLightComponent, TransformComponent>().Each([&](ecs::Entity& entity, PointLightComponent& light, TransformComponent& transform)
			{
				// Check if point light within camera frustum 
				glm::mat4 pcTransform = scene->ComputePCTransform(entity).first;

				if (frustum.IsInFrustum({ pcTransform[3].x, pcTransform[3].y, pcTransform[3].z }, light->Distance))
				{
					Renderer::SubmitLight(light, pcTransform, m_Camera);  m_Statistic.SubmitedPointLights++;

					if (m_Settings.IsPointLightShow)
					{
						/* In case we want to use point light sphere during instance rendering we need reuse deafult sphere and apply changes only to transform matrix.*/
						pcTransform = glm::scale(pcTransform, glm::vec3(light->Distance));
						Renderer::SubmitStaticMesh(GetPipeline("Point-Lights-Visualizing"), m_Sphere, m_LightVisualizingMaterial, nullptr, pcTransform);
					}
				}
			});
		scene->View<SpotLightComponent, TransformComponent>().Each([&](ecs::Entity& entity, SpotLightComponent& light, TransformComponent& transform)
			{
				glm::mat4 pcTransform = scene->ComputePCTransform(entity).first;
			
				float radius = light->Distance * glm::acos(glm::radians(light->MaxAngle));
	
				if (frustum.IsInFrustum({ pcTransform[3].x, pcTransform[3].y, pcTransform[3].z }, glm::normalize(glm::mat3(pcTransform) * glm::vec3(0.f, 0.f, 1.f)), light->Distance, radius))
				{
					Renderer::SubmitLight(light, pcTransform, m_Camera);
					if (m_Settings.IsSpotLightShow)
					{
						/* In case we want to use spot light cone during instance rendering we need reuse deafult cone and apply changes only to transform matrix.*/
						pcTransform = glm::scale(pcTransform, glm::vec3(radius, radius, light->Distance));
						Renderer::SubmitStaticMesh(GetPipeline("Spot-Lights-Visualizing"), m_Cone, m_LightVisualizingMaterial, nullptr, pcTransform);
					}
				}
			});

		scene->View<Asset<Model>, TransformComponent>().Each([&](ecs::Entity& entity, Asset<Model>& model, TransformComponent& transform)
			{
				auto pcTransform = scene->ComputePCTransform(entity).first; // Frusturm culling need matrix without compensation
				bool isModelInFrustrum = false;

				Asset<animation::AnimationGraph> animationGraph = (entity.HasComponent<AnimationGraphComponent>()) ? entity.GetComponent<AnimationGraphComponent>().AnimationGraph : nullptr;
				animation::Pose* finalPose = (animationGraph) ? animationGraph->GetOutputPose() : nullptr;

				for (const auto& mesh : *model)
				{
					//if (frustum.IsInFrustum(pcTransform, mesh->GetMinHalfExt(), mesh->GetMaxHalfExt()))
					{
						isModelInFrustrum = true;

						if (finalPose && mesh->GetLod(0).Bones.size())
						{
							Renderer::SubmitStaticMesh(GetPipeline("Main-Geometry-Animated"), mesh, mesh->GetMaterial(), model, pcTransform); m_Statistic.SubmitedInstances++;
						}
						else
						{
							Renderer::SubmitStaticMeshDynamicLOD(GetPipeline("Main-Geometry-Static"), mesh, mesh->GetMaterial(), model, pcTransform); m_Statistic.SubmitedInstances++;
						}
						
						if (m_Settings.RenderSettings.LightCulling)
							Renderer::SubmitStaticMesh(GetPipeline("Light-Culling-Pre-Depth"), mesh, nullptr, model, pcTransform);


					}

					// Check if mesh inside point light for shadow pass  
					if (m_Settings.RenderSettings.PointShadowEnabled)
					{
						for (std::uint32_t index = 0; index < Renderer::GetSubmitedPointLightCount(); index++)
						{
							auto& renderData = Renderer::GetSubmitedPointLightRenderData(index);

							if (m_Settings.IsPointLightShadowSplitBySides)
							{
								// Split render passes for each side of cube 
								for (std::uint32_t side = 0; side < 6; side++)
								{
									if (PointLight::IsMeshInside(renderData.Cascades[side].ViewProjectionMatrix, pcTransform, mesh->GetMinHalfExt(), mesh->GetMaxHalfExt()))
									{
										std::size_t seed = index; glm::detail::hash_combine(seed, side);
										Renderer::SubmitStaticMesh(GetPipeline("Point-Light-Shadow-Pre-Depth"), mesh, nullptr, model, pcTransform, seed);
									}
								}
							}
							else
							{
								if (PointLight::IsMeshInside(renderData.Position, renderData.Distance, pcTransform, mesh->GetMinHalfExt(), mesh->GetMaxHalfExt()))
									Renderer::SubmitStaticMesh(GetPipeline("Point-Light-Shadow-Pre-Depth"), mesh, mesh->GetMaterial(), model, pcTransform, index);
							}
						}
					}
					// Check if mesh inside spot light for shadow pass  
					if (m_Settings.RenderSettings.SpotShadowEnabled)
					{
						for (std::uint32_t index = 0; index < Renderer::GetSubmitedSpotLightCount(); index++)
						{
							auto& renderData = Renderer::GetSubmitedSpotLightRenderData(index);

							float radius = glm::acos(glm::radians(renderData.MaxAngle)) * renderData.Distance;
							if (SpotLight::IsMeshInside(renderData.Cascade.ViewProjectionMatrix, pcTransform, mesh->GetMinHalfExt(), mesh->GetMaxHalfExt()))
							{
								Renderer::SubmitStaticMesh(GetPipeline("Spot-Light-Shadow-Pre-Depth"), mesh, mesh->GetMaterial(), model, pcTransform, index);
							}
						}
					}

					if (m_Settings.RenderSettings.GlobalShadowsEnabled)
						Renderer::SubmitStaticMesh(GetPipeline("Global-Light-Shadow-Pre-Depth"), mesh, mesh->GetMaterial(), model, pcTransform);

					// OBB Visualization
					if (m_Settings.IsAABB_OBBShow)
					{
						/* In case we want to use aabb box during instance rendering we need reuse deafult box min and max ext and apply changes only to transform matrix.*/
						// Translate the cpTransform matrix to the center of the mesh
						glm::mat4 permeshTransform = glm::translate(scene->ComputePCTransform(entity).second, (mesh->GetMinHalfExt() + mesh->GetMaxHalfExt()) / 2.f);
						// Scale the cpTransform matrix using the ratio of the half extents of the mesh and the bounding box
						permeshTransform = glm::scale(permeshTransform, (mesh->GetMaxHalfExt() - mesh->GetMinHalfExt()) / (m_OBB->GetMaxHalfExt() - m_OBB->GetMinHalfExt()));

						// Submit aabb for rendering 
						Renderer::SubmitStaticMesh(GetPipeline("AABB_OBB"), m_OBB, m_OBBMaterial, nullptr, permeshTransform);
					}
				}

				if (isModelInFrustrum && finalPose)
				{
					if(activeEntity == entity)
					{
						static SharedPointer<std::vector<animation::Pose::GlobalTransform>> skVisuazlize = SharedPointer<std::vector<animation::Pose::GlobalTransform>>::Create(RenderAPI::MAX_BONES_PER_INSTANCE);

						for (const auto& [name, bone] : finalPose->GetSkeleton()->GetBones())
						{
							auto parrentId = finalPose->GetBoneGlobalTransform(bone.ID).ParentId;

							glm::mat4 boneT = finalPose->GetBoneGlobalTransform(bone.ID).Transform;
							glm::mat4 parentBoneT = (parrentId != ~0) ? finalPose->GetBoneGlobalTransform(parrentId).Transform : glm::mat4(0.0), parentInverseBindPoseT = (parrentId != ~0) ? finalPose->GetSkeleton()->GetBone(parrentId)->InverseBindPose : glm::mat4(0.0);

							if (finalPose->HasInverseBindPose())
							{
								boneT = boneT * glm::inverse(bone.InverseBindPose);
								parentBoneT = parentBoneT * glm::inverse(parentInverseBindPoseT);
							}

							skVisuazlize->at(bone.ID).ParentId = parrentId;
							skVisuazlize->at(bone.ID).Transform = pcTransform * boneT;

							const float scale = glm::distance(boneT * glm::vec4(0, 0, 0, 1), parentBoneT * glm::vec4(0, 0, 0, 1)) * 0.06;

							if (parrentId != ~0)
							{
								Renderer::SubmitStaticMesh(GetPipeline("Skeleton-Joint-Visualizing"), m_Sphere, m_JoinVisualizingMaterial, nullptr, pcTransform * glm::scale(parentBoneT, glm::vec3(scale)));
							}
						}

						Renderer::SubmitStaticMesh(GetPipeline("Skeleton-Bone-Visualizing"), nullptr, nullptr, model, pcTransform);
						Renderer::SubmitBoneTransforms(GetPipeline("Skeleton-Bone-Visualizing"), model, skVisuazlize);
					}

					if (!finalPose->HasInverseBindPose())
					{
						for (auto& [name, bone] : finalPose->GetSkeleton()->GetBones())
						{
							finalPose->GetBoneGlobalTransforms()->at(bone.ID).Transform *= bone.InverseBindPose;
						}

						finalPose->MarkHasInverseBindPose(true);
					}
					
					Renderer::SubmitBoneTransforms(GetPipeline("Main-Geometry-Animated"), model, finalPose->GetBoneGlobalTransforms());
				}

				// AABB Visualization
				if (entity.HasComponent<RigidBodyComponent>())
				{
					auto& rigidBody = entity.GetComponent<RigidBodyComponent>();

					for (const auto& ext : rigidBody.GetExtensions())
					{
						auto minExt = ext.MinHalfExtWorldSpace;
						auto maxExt = ext.MaxHalfExtWorldSpace;

						glm::mat<4, 4, physic::scalar_t> permeshTransform = glm::translate(glm::mat<4, 4, physic::scalar_t>(1.0), (minExt + maxExt) / 2.0);
						permeshTransform = glm::scale(permeshTransform,
							(maxExt - minExt) / glm::vec<3, physic::scalar_t>(m_OBB->GetMaxHalfExt() - m_OBB->GetMinHalfExt())
						);

						//Renderer::SubmitStaticMesh(m_AABB_OBB_Pipeline, m_OBB, m_AABBMaterial, nullptr, permeshTransform);
					}
					
					//for (auto& contactPoint : rigidBody.GetCollisionContacts())
					//{
					//	glm::mat4 pointTransform = glm::translate(pcTransform, static_cast<glm::vec3>(contactPoint));

					//	glm::vec3 p, s; glm::quat r;
					//	math::DecomposeMatrix(pointTransform, p, r, s);
					//	glm::mat4 mat = glm::translate(glm::mat4(1.f), p) * glm::toMat4(r) * glm::scale(glm::mat4(1.f), glm::vec3(0.1f));

					//	pointTransform *= glm::scale(glm::mat4(1.f), glm::vec3(0.1f));

					//	//Renderer::SubmitStaticMesh(m_FlatPipeline, m_Sphere, nullptr, mat);
					//}

					/*for (auto& collider : rigidBody.GetColliders())
					{
						
						if (collider->GetShape() == physic::CollisionShape::Shape::Mesh)
						{
							SharedPointer<Drawable> hullMesh = SharedPointer<Drawable>::Create();

							physic::MeshShape* meshShape = reinterpret_cast<physic::MeshShape*>(
								const_cast<physic::CollisionShape*>(collider.Raw())
								);

							auto& vertices =  meshShape->GetVertices();

							for (std::size_t i = 0; i < vertices.size(); i++)
							{
								hullMesh->AddVertex({ vertices[i] });
								hullMesh->AddIndex(i);
								
								
							}

							pcTransform *= glm::scale(glm::mat4(1.f), glm::vec3(1.0f));
							Renderer::SubmitStaticMesh(m_AABBPipeline, hullMesh, nullptr, pcTransform);
						}
					}*/
					

					/*auto& contactPoint = entity.GetComponent<RigidBodyComponent>().GetLocalContactPoint();
					glm::mat4 pointTransform = glm::translate(pcTransform, static_cast<glm::vec3>(contactPoint));

					glm::vec3 p, s; glm::quat r;
					math::DecomposeMatrix(pointTransform, p, r, s);
					glm::mat4 mat = glm::translate(glm::mat4(1.f), p) * glm::toMat4(r) * glm::scale(glm::mat4(1.f), glm::vec3(0.1f));
					
					pointTransform *= glm::scale(glm::mat4(1.f), glm::vec3(0.1f));
					
					Renderer::SubmitStaticMesh(m_CollisionContanctPointPipline, m_Sphere, nullptr, mat);*/
				}
			});

		// Submit grid for rendering
		if (m_Settings.IsGridShow)
			Renderer::SubmitStaticMesh(GetPipeline("Grid"), m_Plane, nullptr, nullptr, glm::mat4(1.f));
	}
	else
	{
		SHADE_CORE_WARNING("SceneRender::Couldn't find in scene primary camera!");
	   m_Camera = nullptr;
	}
	
}

void shade::SceneRenderer::OnRender(SharedPointer<Scene>& scene, const FrameTimer& deltaTime)
{
	auto curentFrameIndex = Renderer::GetCurrentFrameIndex();

	if (m_Camera != nullptr)
	{
		m_MainCommandBuffer->Begin(curentFrameIndex);

		Renderer::BeginScene(m_Camera, m_Settings.RenderSettings, curentFrameIndex);
		{
			{
				Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Global-Light-Shadow-Pre-Depth"), curentFrameIndex);
				Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Spot-Light-Shadow-Pre-Depth"), curentFrameIndex);
				Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Point-Light-Shadow-Pre-Depth"), curentFrameIndex);
				Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Light-Culling-Pre-Depth"), curentFrameIndex);
				Renderer::ExecuteComputePipeline(GetPipeline("Light-Culling"), curentFrameIndex);
			}

			bool mainPipelineHasExecuted = Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Main-Geometry-Static"), curentFrameIndex, true);
			mainPipelineHasExecuted		+= Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Main-Geometry-Animated"), curentFrameIndex, !mainPipelineHasExecuted);

			{
					//Renderer::ExecuteComputePipeline(GetPipeline("SSAO"), curentFrameIndex);
					Renderer::ExecuteComputePipeline(GetPipeline("Bloom"), curentFrameIndex);
					Renderer::ExecuteComputePipeline(GetPipeline("Color-Correction"), curentFrameIndex);
			}
			{
					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("AABB_OBB"), curentFrameIndex);
					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Grid"), curentFrameIndex, !mainPipelineHasExecuted);
					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Point-Lights-Visualizing"), curentFrameIndex, !mainPipelineHasExecuted);
					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Spot-Lights-Visualizing"), curentFrameIndex, !mainPipelineHasExecuted);
					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Skeleton-Bone-Visualizing"), curentFrameIndex, !mainPipelineHasExecuted);
					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Skeleton-Joint-Visualizing"), curentFrameIndex, !mainPipelineHasExecuted);
			}
		}
		Renderer::EndScene(curentFrameIndex);
		m_MainCommandBuffer->End(curentFrameIndex);
		m_MainCommandBuffer->Submit(curentFrameIndex);
	}
}

void shade::SceneRenderer::OnEvent(SharedPointer<Scene>& scene, const Event& event, const FrameTimer& deltaTime)
{

}

std::vector<shade::SharedPointer<shade::FrameBuffer>>& shade::SceneRenderer::GetMainTargetFrameBuffer()
{
	return m_MainTargetFrameBuffer;
}

shade::SharedPointer<shade::Texture2D>& shade::SceneRenderer::GetBloomRenderTarget()
{
	return m_BloomTarget;
}

shade::SharedPointer<shade::Texture2D>& shade::SceneRenderer::GetSAAORenderTarget()
{
	return m_ScreenSpaceAmbientOcclusionTarget;
}

shade::SceneRenderer::Settings& shade::SceneRenderer::GetSettings()
{
	return m_Settings;
}

const shade::SceneRenderer::Statistic& shade::SceneRenderer::GetStatistic() const
{
	return m_Statistic;
}

shade::SharedPointer<shade::Camera>& shade::SceneRenderer::GetActiveCamera()
{
	return m_Camera;
}

shade::SharedPointer<shade::Pipeline>& shade::SceneRenderer::RegisterNewPipeline(const SharedPointer<Pipeline>& pipeline)
{
	return m_Pipelines.emplace(pipeline->GetSpecification().Name, pipeline).first->second;
}

shade::SharedPointer<shade::Pipeline> shade::SceneRenderer::GetPipeline(const std::string& name)
{
	return (m_Pipelines.find(name) != m_Pipelines.end()) ? m_Pipelines.at(name) : shade::SharedPointer<shade::Pipeline>{};
}

std::unordered_map<std::string, shade::SharedPointer<shade::Pipeline>>& shade::SceneRenderer::GetPipelines()
{
	return m_Pipelines;
}

void shade::SceneRenderer::LightCullingPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, true);
	// Update buffers 
	pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
	// For each instance and its materials in the Instances container
	for (auto& [instance, materials] : instances.Instances)
	{
		for (auto& [lod, material] : materials.Materials)
		{
			// Draw the submitted instance
			Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);
		}
	}
	// End rendering
	Renderer::EndRender(m_MainCommandBuffer, pipeline, frameIndex);

	// Set a memory barrier
	//pipeline->SetBarrier(m_MainCommandBuffer, RenderPipeline::Stage::FragmentShader, RenderPipeline::Stage::ComputeShader, RenderPipeline::Access::ShaderWrite, RenderPipeline::Access::ShaderWrite, frameIndex);
}

void shade::SceneRenderer::LightCullingComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex)
{
	Renderer::BeginCompute(m_MainCommandBuffer, pipeline, frameIndex);
	glm::uvec2 resolution = { m_LightCullingPreDepthFrameBuffer->GetWidth(), m_LightCullingPreDepthFrameBuffer->GetHeight() };
	// set uniforms required by the compute shader
	pipeline->SetUniform(m_MainCommandBuffer, sizeof(glm::uvec2), &resolution, frameIndex);
	// round up the resolution to a multiple of tileSize
	constexpr uint32_t tileSize = 16u;
	resolution += tileSize - resolution % tileSize;
	// divide the resolution by tileSize to get the number of execution groups
	glm::uvec3 executionGroups = { resolution / tileSize, 1 };

	std::uint32_t tilesCountX = executionGroups.x;

	// resize buffers used to store visible indices of lights
	m_VisiblePointLightIndicesBuffer->Resize(executionGroups.x * executionGroups.y * sizeof(std::uint32_t) * RenderAPI::MAX_POINT_LIGHTS_COUNT);
	m_VisibleSpotLightIndicesBuffer->Resize(executionGroups.x * executionGroups.y * sizeof(std::uint32_t) * RenderAPI::MAX_SPOT_LIGHTS_COUNT);

	// set resources needed by the compute shader
	pipeline->SetResource(m_VisiblePointLightIndicesBuffer, Pipeline::Set::PerInstance, frameIndex);
	pipeline->SetResource(m_VisibleSpotLightIndicesBuffer, Pipeline::Set::PerInstance, frameIndex);
	pipeline->SetTexture(m_LightCullingPreDepthFrameBuffer->GetDepthAttachment(), Pipeline::Set::PerInstance, 9, frameIndex); // TODO: 9 need to create constan in RenderAPI

	GetPipeline("Main-Geometry-Static")->As<RenderPipeline>().SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &tilesCountX, frameIndex, Shader::Type::Fragment);

	// update resources, dispatch the compute shader and set a memory barrier
	pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
	pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
	pipeline->SetBarrier(m_MainCommandBuffer, Pipeline::Stage::ComputeShader, Pipeline::Stage::FragmentShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
	Renderer::EndCompute(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::GlobalLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, true);
	// Update buffers 
	pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
	for (auto& [instance, materials] : instances.Instances)
	{
		for (auto& [lod, material] : materials.Materials)
		{
			for (std::uint32_t cascade = 0; cascade < GlobalLight::SHADOW_CASCADES_COUNT; cascade++)
			{
				pipeline->SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &cascade, frameIndex, Shader::Type::Vertex);
				// Draw the submitted instance
				Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);
			}
		}
	}

	// End rendering
	Renderer::EndRender(m_MainCommandBuffer,pipeline, frameIndex);

}

void shade::SceneRenderer::SpotLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, (bool)Renderer::GetSubmitedSpotLightCount(), Renderer::GetSubmitedSpotLightCount());
	// Update buffers 
	pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);

	for (auto& [instance, materials] : instances.Instances)
	{
		for (auto& [lod, material] : materials.Materials)
		{
			for (std::uint32_t index = 0; index < Renderer::GetSubmitedSpotLightCount(); index++)
			{
				pipeline->SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &index, frameIndex, Shader::Type::Vertex);
				// Draw the submitted instance
				Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod, index);
			}
		}
	}
	// End rendering
	Renderer::EndRender(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::PointLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, (bool)Renderer::GetSubmitedPointLightCount(), Renderer::GetSubmitedPointLightCount() * 6);
	// Update buffers 
	pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);

	for (auto& [instance, materials] : instances.Instances)
	{
		for (auto& [lod, material] : materials.Materials)
		{
			for (std::uint32_t index = 0; index < Renderer::GetSubmitedPointLightCount(); index++)
			{
				pipeline->SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &index, frameIndex, Shader::Type::Vertex);

				for (std::uint32_t side = 0; side < 6; side++)
				{
					pipeline->SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &side, frameIndex, sizeof(std::uint32_t));

					if (m_Settings.IsPointLightShadowSplitBySides)
					{
						// Draw the submitted instance
						std::size_t seed = index; glm::detail::hash_combine(seed, side);
						Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod, seed);
					}
					else
					{
						// Draw the submitted instance
						Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod, index);
					}
				}
			}
		}
	}
	// End rendering
	Renderer::EndRender(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::InstancedGeometryPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, isForceClear);
	// Set the visible point light and spot light indices for the pipeline to use during rendering.
	pipeline->SetResource(m_VisiblePointLightIndicesBuffer, Pipeline::Set::PerInstance, frameIndex);
	pipeline->SetResource(m_VisibleSpotLightIndicesBuffer, Pipeline::Set::PerInstance, frameIndex);
	pipeline->SetTexture(m_GlobalLightShadowFrameBuffer->GetDepthAttachment(), Pipeline::Set::PerInstance, RenderAPI::GLOBAL_SHADOW_MAP_BINDING, frameIndex);
	pipeline->SetTexture(m_SpotLightShadowFrameBuffer->GetDepthAttachment(), Pipeline::Set::PerInstance, RenderAPI::SPOT_SHADOW_MAP_BINDING, frameIndex);
	pipeline->SetTexture(m_PointLightShadowFrameBuffer->GetDepthAttachment(), Pipeline::Set::PerInstance, RenderAPI::POINT_SHADOW_MAP_BINDING, frameIndex);
	
	std::uint32_t drawInstane = 0;

	// Loop over the instances and their materials, updating and drawing each submitted material with the rendered instance.
	for (auto& [instance, materials] : instances.Instances)
	{											
		if (GetPipeline("Main-Geometry-Animated").Raw() == pipeline.Raw())
			Renderer::UpdateSubmitedBonesData(m_MainCommandBuffer, GetPipeline("Main-Geometry-Animated"), materials.ModelHash, frameIndex);

		// For each material
		for (auto& [lod, material] : materials.Materials)
		{
			// Update the submitted material
			Renderer::UpdateSubmitedMaterial(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);

			// Draw the submitted instance
			(GetPipeline("Main-Geometry-Animated").Raw() == pipeline.Raw()) ? Renderer::DrawSubmitedInstancedAnimated(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod) : Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);
		}

		++drawInstane;
	}
	// End rendering
	Renderer::EndRender(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::GridPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, isForceClear);
	// Update buffers 
	pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
	// For each instance and its materials in the Instances container
	for (auto& [instance, materials] : instances.Instances)
	{
		// For each material
		for (auto& [lod, material] : materials.Materials)
		{
			// Draw the submitted instance
			Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex);
		}
	}
	// End rendering
	Renderer::EndRender(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::LightVisualizationPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, isForceClear);
	// Update buffers 
	//pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
	// For each instance and its materials in the Instances container
	for (auto& [instance, materials] : instances.Instances)
	{
		for (auto& [lod, material] : materials.Materials)
		{
			Renderer::UpdateSubmitedMaterial(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);
			// Draw the submitted instance
			Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex);
		}
	}
	// End rendering
	Renderer::EndRender(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::SkeletonVisualizationPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, false);
	// Update buffers 
	//pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
	// For each instance and its materials in the Instances container

	int instance = 0;
	for (auto& [instance, materials] : instances.Instances)
	{
		Renderer::UpdateSubmitedBonesData(m_MainCommandBuffer, pipeline, materials.ModelHash, frameIndex);

		for (auto& [lod, material] : materials.Materials)
		{
			// Update the submitted material
			Renderer::UpdateSubmitedMaterial(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);
			// Draw the submitted instance
			Renderer::DummyInvocation(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);
		}
	}
	// End rendering
	Renderer::EndRender(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::FlatPipeline(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, false);
	// Update buffers 
	//pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
	// For each instance and its materials in the Instances container
	for (auto& [instance, materials] : instances.Instances)
	{
		for (auto& [lod, material] : materials.Materials)
		{
			// Update the submitted material
			Renderer::UpdateSubmitedMaterial(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);
			// Draw the submitted instance
			Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance,  material, frameIndex, lod);
		}
	}
	// End rendering
	Renderer::EndRender(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::ColorCorrectionComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex)
{
	Renderer::BeginCompute(m_MainCommandBuffer, pipeline, frameIndex);
	// Get the texture attachment from the main target frame buffer and store in a constant auto reference.
	const auto& texture = m_MainTargetFrameBuffer[frameIndex]->GetTextureAttachment(0);
	// Calculate the number of execution groups required to cover the entire texture.
	glm::uvec3 executionGroups
	{
		std::ceil(static_cast<float>(texture->GetWidth()) / 16.f), std::ceil(static_cast<float>(texture->GetWidth()) / 16.0f), 1
	};
	// Get the render data settings required for color correction.
	ColorCorrection::RenderData renderData = m_Settings.ColorCorrectionSettings.GetRenderData();
	// Set input attachment
	pipeline->SetTexture(texture, Pipeline::Set::PerInstance, 0, frameIndex);
	// Set output attachment
	pipeline->SetTexture(texture, Pipeline::Set::PerInstance, 1, frameIndex);
	// Set the uniform data for the pipeline to use in the rendering process.
	pipeline->SetUniform(m_MainCommandBuffer, sizeof(ColorCorrection::RenderData), &renderData, frameIndex);
	// Update the resources
	pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
	// Dispatch the pipeline with the set execution groups.
	pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
	// Set a barrier to signal the end of the computation process.
	pipeline->SetBarrier(m_MainCommandBuffer, texture, Pipeline::Stage::ColorAttachmentOutput, Pipeline::Stage::ComputeShader, Pipeline::Access::ColorAttachmentWrite, Pipeline::Access::ShaderRead, frameIndex);
	Renderer::EndCompute(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::BloomComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex)
{
	const auto& mainTarget = m_MainTargetFrameBuffer[frameIndex]->GetTextureAttachment(0);
	const std::uint32_t SAMPLES = m_Settings.BloomSettings.Samples;

	auto grups = ImGuiLayer::GetGlobalValue<glm::ivec2>("Work groups");

	glm::uvec3 executionGroups { std::ceil(static_cast<float>(mainTarget->GetWidth()) / (16.f + grups->x)), std::ceil(static_cast<float>(mainTarget->GetHeight()) / (16.0f + grups->y)), 1 };
	Bloom::RenderData bloomData = m_Settings.BloomSettings.GetRenderData();

	

	Renderer::BeginCompute(m_MainCommandBuffer, pipeline, frameIndex);
	{
		/////////////////////////HDR////////////////////////////////
		bloomData.Stage = Bloom::Stage::HDR;
		pipeline->SetTexture(mainTarget, Pipeline::Set::PerInstance, 1, frameIndex);
		pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 2, frameIndex, 0);
		//Proxy 
		pipeline->SetTexture(m_BloomTarget, Pipeline::Set::PerInstance, 0, frameIndex);

		pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);

		pipeline->SetUniform(m_MainCommandBuffer, sizeof(SceneRenderer::Bloom::RenderData), &bloomData, frameIndex);

		pipeline->SetBarrier(m_MainCommandBuffer, mainTarget, Pipeline::Stage::ColorAttachmentOutput, Pipeline::Stage::ComputeShader, Pipeline::Access::ColorAttachmentWrite, Pipeline::Access::ShaderRead, frameIndex);
		pipeline->Dispatch(m_MainCommandBuffer, (std::uint32_t)executionGroups.x, (std::uint32_t)executionGroups.y, executionGroups.z, frameIndex);
		pipeline->SetBarrier(m_MainCommandBuffer, m_BloomTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
	}
	{
		/////////////////////////Downsample//////////////////////////
		bloomData.Stage = Bloom::Stage::DownSample;
		for (std::uint32_t mip = 0; mip < SAMPLES - 1; mip++)
		{
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 0, frameIndex, mip);
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 2, frameIndex, mip + 1);
			//Proxy 
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 1, frameIndex, mip + 1);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);

			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SceneRenderer::Bloom::RenderData), &bloomData, frameIndex);

			pipeline->Dispatch(m_MainCommandBuffer, (std::uint32_t)std::ceil(executionGroups.x / float(mip + 1.f)), (std::uint32_t)std::ceil(executionGroups.y / float(mip + 1.f)), executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, m_BloomTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
	}
	{
		/////////////////////////////Upsample//////////////////////////////
		bloomData.Stage = Bloom::Stage::UpSample;
		for (std::uint32_t mip = SAMPLES - 1; mip > 0; mip--)
		{
			bloomData.Lod = mip;
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 0, frameIndex, mip);
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 2, frameIndex, mip - 1);
			//Proxy 
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 1, frameIndex, mip - 1);

			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SceneRenderer::Bloom::RenderData), &bloomData, frameIndex);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			pipeline->Dispatch(m_MainCommandBuffer, (std::uint32_t)std::ceil(executionGroups.x / float(mip)), (std::uint32_t)std::ceil(executionGroups.y / float(mip)), executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, m_BloomTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
	}
	{
		/////////////////////////////Combine//////////////////////////////
		bloomData.Stage = Bloom::Stage::Combine;
		pipeline->SetTexture(m_BloomTarget,	Pipeline::Set::PerInstance, 1, frameIndex);
		pipeline->SetTexture(mainTarget,								Pipeline::Set::PerInstance, 2, frameIndex);
		// Proxy
		pipeline->SetTexture(m_BloomTarget,   Pipeline::Set::PerInstance, 0, frameIndex);

		pipeline->SetUniform(m_MainCommandBuffer, sizeof(SceneRenderer::Bloom::RenderData), &bloomData, frameIndex);

		pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
		pipeline->Dispatch(m_MainCommandBuffer, (std::uint32_t)executionGroups.x, (std::uint32_t)executionGroups.y, executionGroups.z, frameIndex);
		pipeline->SetBarrier(m_MainCommandBuffer, mainTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
	}
	Renderer::EndCompute(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::SSAOComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex)
{
	const auto& mainTarget		= m_MainTargetFrameBuffer[frameIndex]->GetTextureAttachment(0);
	const auto& positionTexture	= m_MainTargetFrameBuffer[frameIndex]->GetTextureAttachment(1);
	const auto& normalTexture	= m_MainTargetFrameBuffer[frameIndex]->GetTextureAttachment(2);

	glm::uvec3 executionGroups { std::ceil(static_cast<float>(mainTarget->GetWidth()) / 16.f), std::ceil(static_cast<float>(mainTarget->GetHeight()) / 16.0f), 1 };

	SSAO::RenderData	renderData = m_Settings.SSAOSettings.GetRenderData();
	SSAO::RenderBuffer	renderBuffer = m_Settings.SSAOSettings.GetRenderBuffer();

	m_SSAOSamplesBuffer->SetData(sizeof(SSAO::RenderBuffer), &renderBuffer, frameIndex);

	Renderer::BeginCompute(m_MainCommandBuffer, pipeline, frameIndex);
	{
		pipeline->SetResource(m_SSAOSamplesBuffer, Pipeline::Set::PerInstance, frameIndex);
		{
			////////////////////////////GENERATE////////////////////////////////////////////
			renderData.Stage = SSAO::Stage::Generate;
			pipeline->SetTexture(positionTexture, Pipeline::Set::PerInstance, 0, frameIndex);
			pipeline->SetTexture(normalTexture, Pipeline::Set::PerInstance, 1, frameIndex);
			pipeline->SetTexture(m_ScreenSpaceAmbientOcclusionTarget, Pipeline::Set::PerInstance, 2, frameIndex);

			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SSAO::RenderData), &renderData, frameIndex);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, m_ScreenSpaceAmbientOcclusionTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
		{
			////////////////////////////BLUR H////////////////////////////////////////////
			renderData.Stage = SSAO::Stage::BlurHorizontal;
			pipeline->SetTexture(m_ScreenSpaceAmbientOcclusionTarget, Pipeline::Set::PerInstance, 0, frameIndex);
			pipeline->SetTexture(m_ScreenSpaceAmbientOcclusionTarget, Pipeline::Set::PerInstance, 1, frameIndex);

			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SSAO::RenderData), &renderData, frameIndex);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
		{
			////////////////////////////BLUR V////////////////////////////////////////////
			renderData.Stage = SSAO::Stage::BlurVertical;
			pipeline->SetTexture(m_ScreenSpaceAmbientOcclusionTarget, Pipeline::Set::PerInstance, 0, frameIndex);
			pipeline->SetTexture(m_ScreenSpaceAmbientOcclusionTarget, Pipeline::Set::PerInstance, 1, frameIndex);
			
			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SSAO::RenderData), &renderData, frameIndex);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
		{
			////////////////////////////ZOOM////////////////////////////////////////////
			renderData.Stage = SSAO::Stage::Zoom;
			pipeline->SetTexture(m_ScreenSpaceAmbientOcclusionTarget, Pipeline::Set::PerInstance, 0, frameIndex);
			pipeline->SetTexture(m_ScreenSpaceAmbientOcclusionTarget, Pipeline::Set::PerInstance, 1, frameIndex);

			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SSAO::RenderData), &renderData, frameIndex);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
		{
			////////////////////////////COMBINE////////////////////////////////////////////
			renderData.Stage = SSAO::Stage::Combine;
			pipeline->SetTexture(m_ScreenSpaceAmbientOcclusionTarget, Pipeline::Set::PerInstance, 0, frameIndex);
			pipeline->SetTexture(mainTarget, Pipeline::Set::PerInstance, 1, frameIndex);
			pipeline->SetTexture(m_ScreenSpaceAmbientOcclusionTarget, Pipeline::Set::PerInstance, 2, frameIndex);

			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SSAO::RenderData), &renderData, frameIndex);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, mainTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
	}
	Renderer::EndCompute(m_MainCommandBuffer, pipeline, frameIndex);
}
