#include "shade_pch.h"
#include "SceneRenderer.h"
#include <shade/core/event/Input.h>
#include <shade/core/application/Application.h>

#include <glm/glm/gtx/hash.hpp>

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
	m_GlobalLightShadowFrameBuffer		= FrameBuffer::Create({ 6000, 6000, {0.0f, 0.0f, 0.0f, 1.f}, 1.f, { { render::Image::Format::Depth, render::Image::Usage::Attachment, 1, GlobalLight::SHADOW_CASCADES_COUNT } } });
	m_SpotLightShadowFrameBuffer		= FrameBuffer::Create({ 2000, 2000, {0.0f, 0.0f, 0.0f, 1.f}, 1.f, { { render::Image::Format::Depth, render::Image::Usage::Attachment,  1,  RenderAPI::MAX_SPOT_SHADOW_CASTERS }} });
	m_PointLightShadowFrameBuffer		= FrameBuffer::Create({ 1000, 1000, {0.0f, 0.0f, 0.0f, 1.f}, 1.f, { { render::Image::Format::Depth, render::Image::Usage::Attachment, 1,  RenderAPI::MAX_POINT_SHADOW_CASTERS * 6, 1, 1, true}} });
	m_BloomTarget						= Texture2D::CreateEXP(render::Image::Specification{ render::Image::Format::RGBA32F, render::Image::Usage::Storage, m_Settings.BloomSettings.Samples, 1, 200, 200 });
	m_ScreenSpaceAmbientOcclusionTarget	= Texture2D::CreateEXP(render::Image::Specification{ render::Image::Format::RED8UN, render::Image::Usage::Storage, 1, 1, 200, 200 });


	m_MainGeometryPipelineStatic = shade::RenderPipeline::Create(
		{
			.Shader = ShaderLibrary::Create("Main", "./resources/assets/shaders/Main-Geometry-Static.glsl"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = mainGeometryVertexlayoutStatic,
		});

	m_MainGeometryPipelineAnimated = shade::RenderPipeline::Create(
		{
			.Shader = ShaderLibrary::Create("MainAnimated", "./resources/assets/shaders/Main-Geometry-Animated.glsl"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = mainGeometryVertexlayoutAnimated,
		});

	m_LightCullingPreDepthPipeline = shade::RenderPipeline::Create(
		{
			.Shader = ShaderLibrary::Create("PreDepth", "./resources/assets/shaders/preprocess/Tiled-Forward-Pre-Depth.glsl"),
			.FrameBuffer = m_LightCullingPreDepthFrameBuffer,
			.VertexLayout = mainGeometryVertexlayoutStatic
		});
	m_LightCullingPipeline = shade::ComputePipeline::Create(
		{
			.Shader = ShaderLibrary::Create("LightCulling", "./resources/assets/shaders/preprocess/Tiled-Forward-Light-Culling.glsl"),
		});
	// TODO: Should it be like some internal part of renderer ?
	m_VisibleSpotLightIndicesBuffer  = StorageBuffer::Create(StorageBuffer::Usage::GPU, RenderAPI::SPOT_LIGHT_INDINCES_BINDING, sizeof(std::uint32_t) * RenderAPI::MAX_SPOT_LIGHTS_COUNT, Renderer::GetFramesCount(), 20);
	m_VisiblePointLightIndicesBuffer = StorageBuffer::Create(StorageBuffer::Usage::GPU, RenderAPI::POINT_LIGHT_INDINCES_BINDING, sizeof(std::uint32_t) * RenderAPI::MAX_POINT_LIGHTS_COUNT, Renderer::GetFramesCount(), 20);
	m_GlobalLightShadowDepthPipeline = shade::RenderPipeline::Create(
		{
			.Shader = ShaderLibrary::Create("GlobalShadowPreDepth", "./resources/assets/shaders/preprocess/Global-Light-Shadow-Pre-Depth.glsl"),
			.FrameBuffer = m_GlobalLightShadowFrameBuffer,
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.BackFalceCull = false,
			.DepsBiasConstantFactor = 4.0f,
			.DepthBiasSlopeFactor = 8.0f,
		});
	m_SpotLightShadowDepthPipeline = shade::RenderPipeline::Create(
		{
			.Shader = ShaderLibrary::Create("SpotShadowPreDepth", "./resources/assets/shaders/preprocess/Spot-Light-Shadow-Pre-Depth.glsl"),
			.FrameBuffer = m_SpotLightShadowFrameBuffer,
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.BackFalceCull = false,
			.DepsBiasConstantFactor = 4.0f,
			.DepthBiasSlopeFactor = 8.0f,
		});
	m_PointLightShadowDepthPipeline = shade::RenderPipeline::Create(
		{
			.Shader = ShaderLibrary::Create("PointShadowPreDepth", "./resources/assets/shaders/preprocess/Point-Light-Shadow-Pre-Depth.glsl"),
			.FrameBuffer = m_PointLightShadowFrameBuffer,
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.BackFalceCull = false,
			/*.DepsBiasConstantFactor = 4.0f,
			.DepthBiasSlopeFactor = 8.0f,*/
		});
	m_PointLightVisualizationPipeline = shade::RenderPipeline::Create(
		{
			.Shader = ShaderLibrary::Create("LightsVisualizing", "./resources/assets/shaders/utils/LightsVisualizing.glsl"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.Topology = Pipeline::PrimitiveTopology::Line,
			.BackFalceCull = false
		});
	m_SpotLightVisualizationPipeline = shade::RenderPipeline::Create(
		{
			.Shader = ShaderLibrary::Get("LightsVisualizing"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.Topology = Pipeline::PrimitiveTopology::LineStrip,
			.BackFalceCull = false
		});
	m_GridPipeline = shade::RenderPipeline::Create(
		{
			.Shader = ShaderLibrary::Create("Grid", "./resources/assets/shaders/utils/Grid.glsl"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = gridVertexlayout,
			.Topology = Pipeline::PrimitiveTopology::TriangleStrip,
			.BackFalceCull = false
		});
	m_AABB_OBB_Pipeline = shade::RenderPipeline::Create(
		{
			.Shader = ShaderLibrary::Create("StaticFlat", "./resources/assets/shaders/StaticFlat.glsl"),
			.FrameBuffer = m_MainTargetFrameBuffer[0],
			.VertexLayout = mainGeometryVertexlayoutStatic,
			.Topology = Pipeline::PrimitiveTopology::Line,
			//.PolygonMode = Pipeline::PrimitivePolygonMode::Point,
			.LineWidth = 2.f
		});
	m_ScreenSpaceAmbientOcclusionPipeline = shade::ComputePipeline::Create(
		{
			.Shader = ShaderLibrary::Create("SSAO", "./resources/assets/shaders/postprocess/SSAO/Screen-Sapce-Ambien-Occlusion-Compute.glsl"),
		});
	m_SSAOSamplesBuffer = UniformBuffer::Create(UniformBuffer::Usage::CPU_GPU, 4, sizeof(SSAO::RenderBuffer), Renderer::GetFramesCount(), 0);
	m_ColorCorrectionPipeline = shade::ComputePipeline::Create(
		{
			.Shader = ShaderLibrary::Create("ColorCorrection", "./resources/assets/shaders/postprocess/Color-Correction.glsl"),
		});
	m_BloomPipeline = shade::ComputePipeline::Create(
		{
			.Shader = ShaderLibrary::Create("Bloom", "./resources/assets/shaders/postprocess/Bloom/Bloom-Compute.glsl"),
		});

	
	BIND_PIPELINE_PROCESS_FUNCTION(m_MainGeometryPipelineStatic, SceneRenderer, InstancedGeometryPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(m_MainGeometryPipelineAnimated, SceneRenderer, InstancedGeometryPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(m_LightCullingPreDepthPipeline, SceneRenderer, LightCullingPreDepthPass, this);
	BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(m_LightCullingPipeline, SceneRenderer, LightCullingComputePass, this);

	BIND_PIPELINE_PROCESS_FUNCTION(m_GlobalLightShadowDepthPipeline, SceneRenderer, GlobalLightShadowPreDepthPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(m_SpotLightShadowDepthPipeline, SceneRenderer, SpotLightShadowPreDepthPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(m_PointLightShadowDepthPipeline, SceneRenderer, PointLightShadowPreDepthPass, this);

	BIND_PIPELINE_PROCESS_FUNCTION(m_GridPipeline, SceneRenderer, GridPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(m_AABB_OBB_Pipeline, SceneRenderer, FlatPipeline, this);
	
	BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(m_ScreenSpaceAmbientOcclusionPipeline, SceneRenderer, SSAOComputePass, this);
	BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(m_ColorCorrectionPipeline, SceneRenderer, ColorCorrectionComputePass, this);
	BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(m_BloomPipeline, SceneRenderer, BloomComputePass, this);
	
	BIND_PIPELINE_PROCESS_FUNCTION(m_PointLightVisualizationPipeline, SceneRenderer, LightVisualizationPass, this);
	BIND_PIPELINE_PROCESS_FUNCTION(m_SpotLightVisualizationPipeline, SceneRenderer, LightVisualizationPass, this);

	m_CollisionPointsMaterial	= SharedPointer<Material>::Create();

	m_AABBMaterial				= SharedPointer<Material>::Create();
	m_AABBMaterial->ColorDiffuse = glm::vec3(0.3, 0.9, 0.2);

	m_OBBMaterial				= SharedPointer<Material>::Create();
	m_OBBMaterial->ColorDiffuse = glm::vec3(0.9, 0.3, 0.2);

	m_ConvexMeshMaterial		= SharedPointer<Material>::Create();
	m_LightVisualizingMaterial	= SharedPointer<Material>::Create();

	m_Plane  = Plane::Create(10.f, 10.f, 1);
	m_OBB    = Box::Create({ -1.0f ,-1.0f ,-1.0f }, { 1.0f, 1.0f, 1.0f });
	m_Sphere = Sphere::Create(10, 10, 1.0f);
	m_Cone   = Cone::Create(1.f, 1.f, 20, 8.f, glm::vec3(0.0, 0.0, 1.0));
}

void shade::SceneRenderer::OnUpdate(SharedPointer<Scene>& scene, const shade::CameraComponent& camera, const FrameTimer& deltaTime)
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

		if (m_Settings.RenderSettings.LightCulling)
		{
			if (m_LightCullingPreDepthFrameBuffer->GetWidth() != m_MainTargetFrameBuffer[currentFrame]->GetWidth() || m_LightCullingPreDepthFrameBuffer->GetHeight() != m_MainTargetFrameBuffer[currentFrame]->GetHeight())
			{
				m_LightCullingPreDepthFrameBuffer->Resize(m_MainTargetFrameBuffer[currentFrame]->GetWidth(), m_MainTargetFrameBuffer[currentFrame]->GetHeight());
			}
		}
		if (m_Settings.BloomSettings.Enabled)
		{
			if (m_BloomTarget->GetWidth() != m_MainTargetFrameBuffer[currentFrame]->GetWidth() ||
				m_BloomTarget->GetHeight() != m_MainTargetFrameBuffer[currentFrame]->GetHeight() ||
				m_BloomTarget->GetImage()->GetSpecification().MipLevels != m_Settings.BloomSettings.Samples)
			{
				m_BloomTarget->Resize(m_MainTargetFrameBuffer[currentFrame]->GetWidth(), m_MainTargetFrameBuffer[currentFrame]->GetHeight(), m_Settings.BloomSettings.Samples);
			}
		}
		if (m_Settings.RenderSettings.SSAOEnabled)
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
				Renderer::SubmitLight(light, scene->ComputePCTransform(entity), m_Camera);
			});
		scene->View<PointLightComponent, TransformComponent>().Each([&](ecs::Entity& entity, PointLightComponent& light, TransformComponent& transform)
			{
				// Check if point light within camera frustum 
				glm::mat4 pcTransform = scene->ComputePCTransform(entity);

				if (frustum.IsInFrustum({ pcTransform[3].x, pcTransform[3].y, pcTransform[3].z }, light->Distance))
				{
					Renderer::SubmitLight(light, pcTransform, m_Camera);  m_Statistic.SubmitedPointLights++;

					if (m_Settings.IsPointLightShow)
					{
						/* In case we want to use point light sphere during instance rendering we need reuse deafult sphere and apply changes only to transform matrix.*/
						pcTransform = glm::scale(pcTransform, glm::vec3(light->Distance));
						Renderer::SubmitStaticMesh(m_PointLightVisualizationPipeline, m_Sphere, nullptr, nullptr, pcTransform);
					}
				}
			});
		scene->View<SpotLightComponent, TransformComponent>().Each([&](ecs::Entity& entity, SpotLightComponent& light, TransformComponent& transform)
			{
				glm::mat4 pcTransform = scene->ComputePCTransform(entity);
				float radius = glm::acos(glm::radians(light->MaxAngle)) * light->Distance + 3.f;
				if (frustum.IsInFrustum({ pcTransform[3].x, pcTransform[3].y, pcTransform[3].z }, glm::normalize(glm::mat3(pcTransform) * glm::vec3(0.f, 0.f, 1.f)), light->Distance, radius))
				{
					Renderer::SubmitLight(light, pcTransform, m_Camera);
					if (m_Settings.IsSpotLightShow)
					{
						/* In case we want to use spot light cone during instance rendering we need reuse deafult cone and apply changes only to transform matrix.*/
						pcTransform = glm::scale(pcTransform, glm::vec3(radius, radius, light->Distance));
						Renderer::SubmitStaticMesh(m_SpotLightVisualizationPipeline, m_Cone, nullptr, nullptr, pcTransform);
					}
				}
			});

		scene->View<Asset<Model>, TransformComponent>().Each([&](ecs::Entity& entity, Asset<Model>& model, TransformComponent& transform)
			{
				auto pcTransform = scene->ComputePCTransform(entity);

				Asset<animation::AnimationGraph> animationGraph = (entity.HasComponent<AnimationGraphComponent>()) ? entity.GetComponent<AnimationGraphComponent>().AnimationGraph : nullptr;
				const animation::Pose* finalPose = (animationGraph) ? animationGraph->GetOutputPose() : nullptr;


				bool isModelInFrustrum = false;
				
				for (const auto& mesh : *model)
				{
					if (frustum.IsInFrustum(pcTransform, mesh->GetMinHalfExt(), mesh->GetMaxHalfExt()))
					{
						isModelInFrustrum = true;

						if (animationGraph && finalPose && mesh->GetLod(0).Bones.size())
						{
							Renderer::SubmitStaticMesh(m_MainGeometryPipelineAnimated, mesh, mesh->GetMaterial(), model, pcTransform); m_Statistic.SubmitedInstances++;
						}
						else
						{
							Renderer::SubmitStaticMesh(m_MainGeometryPipelineStatic, mesh, mesh->GetMaterial(), model, pcTransform); m_Statistic.SubmitedInstances++;
						}
						
						if (m_Settings.RenderSettings.LightCulling)
							Renderer::SubmitStaticMesh(m_LightCullingPreDepthPipeline, mesh, nullptr, model, pcTransform);
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
										Renderer::SubmitStaticMesh(m_PointLightShadowDepthPipeline, mesh, nullptr, model, pcTransform, seed);
									}
								}
							}
							else
							{
								if (PointLight::IsMeshInside(renderData.Position, renderData.Distance, pcTransform, mesh->GetMinHalfExt(), mesh->GetMaxHalfExt()))
									Renderer::SubmitStaticMesh(m_PointLightShadowDepthPipeline, mesh, mesh->GetMaterial(), model, pcTransform, index);
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
								Renderer::SubmitStaticMesh(m_SpotLightShadowDepthPipeline, mesh, mesh->GetMaterial(), model, pcTransform, index);
							}
						}
					}

					if (m_Settings.RenderSettings.GlobalShadowsEnabled)
						Renderer::SubmitStaticMesh(m_GlobalLightShadowDepthPipeline, mesh, mesh->GetMaterial(), model, pcTransform);

					// OBB Visualization
					if (m_Settings.IsAABB_OBBShow)
					{
						/* In case we want to use aabb box during instance rendering we need reuse deafult box min and max ext and apply changes only to transform matrix.*/
						// Translate the cpTransform matrix to the center of the mesh
						glm::mat4 permeshTransform = glm::translate(pcTransform, (mesh->GetMinHalfExt() + mesh->GetMaxHalfExt()) / 2.f);
						// Scale the cpTransform matrix using the ratio of the half extents of the mesh and the bounding box
						permeshTransform = glm::scale(permeshTransform, (mesh->GetMaxHalfExt() - mesh->GetMinHalfExt()) / (m_OBB->GetMaxHalfExt() - m_OBB->GetMinHalfExt()));
						// Submit aabb for rendering 
						Renderer::SubmitStaticMesh(m_AABB_OBB_Pipeline, m_OBB, m_OBBMaterial, nullptr, permeshTransform);
					}
				}

				
				if (isModelInFrustrum && animationGraph && finalPose)
				{
					 Renderer::SubmitBoneTransforms(m_MainGeometryPipelineAnimated, model, finalPose->GetBoneGlobalTransforms()); 		
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

						Renderer::SubmitStaticMesh(m_AABB_OBB_Pipeline, m_OBB, m_AABBMaterial, nullptr, permeshTransform);
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
			Renderer::SubmitStaticMesh(m_GridPipeline, m_Plane, nullptr, nullptr, glm::mat4(1.f));
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
			//Global lighting shadow pass
			if (m_Settings.RenderSettings.GlobalShadowsEnabled)
				Renderer::ExecuteSubmitedRenderPipeline(m_GlobalLightShadowDepthPipeline, curentFrameIndex);
			// Spot lighting shadow pass
			if(m_Settings.RenderSettings.SpotShadowEnabled)
				Renderer::ExecuteSubmitedRenderPipeline(m_SpotLightShadowDepthPipeline, curentFrameIndex);
			// Point lighting shadow pass
			if (m_Settings.RenderSettings.PointShadowEnabled)
				Renderer::ExecuteSubmitedRenderPipeline(m_PointLightShadowDepthPipeline, curentFrameIndex);

			// Cull point and spot light
			if (m_Settings.RenderSettings.LightCulling)
			{
				Renderer::ExecuteSubmitedRenderPipeline(m_LightCullingPreDepthPipeline, curentFrameIndex);
				Renderer::ExecuteComputePipeline(m_LightCullingPipeline, curentFrameIndex);
			}
			//Main geometry pass 
			bool mainPipelineHasExecuted = Renderer::ExecuteSubmitedRenderPipeline(m_MainGeometryPipelineStatic, curentFrameIndex, true);
			mainPipelineHasExecuted		+= Renderer::ExecuteSubmitedRenderPipeline(m_MainGeometryPipelineAnimated, curentFrameIndex, !mainPipelineHasExecuted);

			{
				if (m_Settings.RenderSettings.SSAOEnabled)
					Renderer::ExecuteComputePipeline(m_ScreenSpaceAmbientOcclusionPipeline, curentFrameIndex);
				if (m_Settings.BloomSettings.Enabled)
					Renderer::ExecuteComputePipeline(m_BloomPipeline, curentFrameIndex);
				if (m_Settings.ColorCorrectionSettings.Enabled)
					Renderer::ExecuteComputePipeline(m_ColorCorrectionPipeline, curentFrameIndex);
			}
			{
				if (m_Settings.IsAABB_OBBShow)
					Renderer::ExecuteSubmitedRenderPipeline(m_AABB_OBB_Pipeline, curentFrameIndex);

				if (m_Settings.IsGridShow)
					Renderer::ExecuteSubmitedRenderPipeline(m_GridPipeline, curentFrameIndex, !mainPipelineHasExecuted);

				if (m_Settings.IsPointLightShow)
					Renderer::ExecuteSubmitedRenderPipeline(m_PointLightVisualizationPipeline, curentFrameIndex, !mainPipelineHasExecuted);

				if (m_Settings.IsSpotLightShow)
					Renderer::ExecuteSubmitedRenderPipeline(m_SpotLightVisualizationPipeline, curentFrameIndex, !mainPipelineHasExecuted);

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

void shade::SceneRenderer::RecompileAllPipelines()
{
	m_MainGeometryPipelineStatic->Recompile();
	m_MainGeometryPipelineAnimated->Recompile();
	m_GridPipeline->Recompile();
	/*m_ScreenSpaceAmbientOcclusionPipeline->Recompile();
	m_BloomPipeline->Recompile();
	m_GridPipeline->Recompile();
	m_AABB_OBB_Pipeline->Recompile();*/
}

void shade::SceneRenderer::LightCullingPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	Renderer::BeginTimestamp(m_MainCommandBuffer, "Light-Culling-Pre-Depth");
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
	Renderer::EndRender(m_MainCommandBuffer, frameIndex);

	m_Statistic.LightCullingPreDepth = Renderer::EndTimestamp(m_MainCommandBuffer, "Light-Culling-Pre-Depth");
	// Set a memory barrier
	//pipeline->SetBarrier(m_MainCommandBuffer, RenderPipeline::Stage::FragmentShader, RenderPipeline::Stage::ComputeShader, RenderPipeline::Access::ShaderWrite, RenderPipeline::Access::ShaderWrite, frameIndex);
}

void shade::SceneRenderer::LightCullingComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex)
{
	Renderer::BeginTimestamp(m_MainCommandBuffer, "Light-Culling-Compute");
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

	m_MainGeometryPipelineStatic->SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &tilesCountX, frameIndex, Shader::Type::Fragment);

	// update resources, dispatch the compute shader and set a memory barrier
	pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
	pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
	pipeline->SetBarrier(m_MainCommandBuffer, Pipeline::Stage::ComputeShader, Pipeline::Stage::FragmentShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
	Renderer::EndCompute(m_MainCommandBuffer, pipeline, frameIndex);
	m_Statistic.LightCullingCompute = Renderer::EndTimestamp(m_MainCommandBuffer, "Light-Culling-Compute");
}

void shade::SceneRenderer::GlobalLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	Renderer::BeginTimestamp(m_MainCommandBuffer, "Global-Light-Pre-Depth");
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
	Renderer::EndRender(m_MainCommandBuffer, frameIndex);

	m_Statistic.GlobalLightPreDepth = Renderer::EndTimestamp(m_MainCommandBuffer, "Global-Light-Pre-Depth");
}

void shade::SceneRenderer::SpotLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	Renderer::BeginTimestamp(m_MainCommandBuffer, "Spot-Light-Pre-Depth");
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
	Renderer::EndRender(m_MainCommandBuffer, frameIndex);

	m_Statistic.SpotLightPreDepth = Renderer::EndTimestamp(m_MainCommandBuffer, "Spot-Light-Pre-Depth");
}

void shade::SceneRenderer::PointLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	Renderer::BeginTimestamp(m_MainCommandBuffer, "Point-Light-Pre-Depth");
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
	Renderer::EndRender(m_MainCommandBuffer, frameIndex);

	m_Statistic.PointLightPreDepth = Renderer::EndTimestamp(m_MainCommandBuffer, "Point-Light-Pre-Depth");
}

void shade::SceneRenderer::InstancedGeometryPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	Renderer::BeginTimestamp(m_MainCommandBuffer, "Instanced-Geometry");
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
		if (m_MainGeometryPipelineAnimated.Raw() == pipeline.Raw())
			Renderer::UpdateSubmitedBonesData(m_MainCommandBuffer, m_MainGeometryPipelineAnimated, materials.ModelHash, frameIndex);

		// For each material
		for (auto& [lod, material] : materials.Materials)
		{
			// Update the submitted material
			Renderer::UpdateSubmitedMaterial(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);

			// Draw the submitted instance
			(m_MainGeometryPipelineAnimated.Raw() == pipeline.Raw()) ? Renderer::DrawSubmitedInstancedAnimated(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod) : Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);			
		}

		++drawInstane;
	}
	// End rendering
	Renderer::EndRender(m_MainCommandBuffer, frameIndex);
	
	m_Statistic.InstanceGeometry = Renderer::EndTimestamp(m_MainCommandBuffer, "Instanced-Geometry");
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
	Renderer::EndRender(m_MainCommandBuffer, frameIndex);
}

void shade::SceneRenderer::LightVisualizationPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, isForceClear);
	// Update buffers 
	pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
	// For each instance and its materials in the Instances container
	for (auto& [instance, materials] : instances.Instances)
	{
		for (auto& [lod, material] : materials.Materials)
		{
			// Draw the submitted instance
			Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex);
		}
	}
	// End rendering
	Renderer::EndRender(m_MainCommandBuffer, frameIndex);
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
	Renderer::EndRender(m_MainCommandBuffer, frameIndex);
}

void shade::SceneRenderer::ColorCorrectionComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex)
{
	Renderer::BeginTimestamp(m_MainCommandBuffer, "Color-Correction");
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
	m_Statistic.ColorCorrection = Renderer::EndTimestamp(m_MainCommandBuffer, "Color-Correction");
}

void shade::SceneRenderer::BloomComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex)
{
	const auto& mainTarget = m_MainTargetFrameBuffer[frameIndex]->GetTextureAttachment(0);
	const std::uint32_t SAMPLES = m_Settings.BloomSettings.Samples;

	glm::uvec3 executionGroups { std::ceil(static_cast<float>(mainTarget->GetWidth()) / 16.f), std::ceil(static_cast<float>(mainTarget->GetHeight()) / 16.0f), 1 };
	Bloom::RenderData bloomData = m_Settings.BloomSettings.GetRenderData();

	Renderer::BeginTimestamp(m_MainCommandBuffer, "Bloom");
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
	m_Statistic.Bloom = Renderer::EndTimestamp(m_MainCommandBuffer, "Bloom");
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
