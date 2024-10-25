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
	m_MainCommandBuffer = (swapChainAsMainTarget) ? RenderCommandBuffer::CreateFromSwapChain() : RenderCommandBuffer::Create(RenderCommandBuffer::Type::Primary, RenderCommandBuffer::Family::Graphic, Renderer::GetFramesCount());
	// Main geometry vertex layout static
	VertexBuffer::Layout MGVLS =
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
	// Main geometry vertex layout animated
	VertexBuffer::Layout MGVLA =
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
	// Greed vertex layout
	VertexBuffer::Layout GVL =
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
	//------------------------------------------------------------------------
	// Frame buffers and render targets                    
	//------------------------------------------------------------------------
	m_MainTargetFrameBuffer =
		FrameBuffer::Create(
			FrameBuffer::Specification
			{
				.Width = 1000, .Height = 1000,
				.ClearColor = {0.05f, 0.05f, 0.05f, 1.f},
				.DepthClearValue = 1.f,
				.Attachments = { { render::Image::Format::RGBA32F,  render::Image::Usage::Attachment, render::Image::Clamp::Repeat },
								 { render::Image::Format::RGBA32F,  render::Image::Usage::Attachment, render::Image::Clamp::Repeat },
								 { render::Image::Format::RGBA32F,  render::Image::Usage::Attachment, render::Image::Clamp::Repeat },
								 { render::Image::Format::DEPTH32F, render::Image::Usage::Attachment, render::Image::Clamp::Repeat } } });
	m_GlobalLightShadowFrameBuffer = FrameBuffer::Create(
		FrameBuffer::Specification
		{
			.Width = 4096, .Height = 4096,
			.ClearColor = { 0.0f, 0.0f, 0.0f, 1.f },
			.DepthClearValue = 1.0,
			.Attachments = {{ render::Image::Format::Depth, render::Image::Usage::Attachment, render::Image::Clamp::Repeat, 1, DirectionalLight::SHADOW_CASCADES_COUNT  }}
		});
	m_SpotLightShadowFrameBuffer = FrameBuffer::Create(
		FrameBuffer::Specification
		{
			.Width = 1024, .Height = 1024,
			.ClearColor = { 0.0f, 0.0f, 0.0f, 1.f },
			.DepthClearValue = 1.0,
			.Attachments = {{ render::Image::Format::Depth, render::Image::Usage::Attachment, render::Image::Clamp::Repeat, 1,  RenderAPI::MAX_SPOT_SHADOW_CASTERS  }}
		});
	m_PointLightShadowFrameBuffer = FrameBuffer::Create(
		FrameBuffer::Specification
		{
			.Width = 1024, .Height = 1024,
			.ClearColor = { 0.0f, 0.0f, 0.0f, 1.f },
			.DepthClearValue = 1.0,
			.Attachments = {{ render::Image::Format::Depth, render::Image::Usage::Attachment, render::Image::Clamp::ClampToEdge, 1,  RenderAPI::MAX_POINT_SHADOW_CASTERS * 6, 1, 1, true  }}
		});
	m_LightCullingPreDepthFrameBuffer = FrameBuffer::Create(
			FrameBuffer::Specification
			{
				.Width = 1, .Height = 1,
				.ClearColor = { 0.0f, 0.0f, 0.0f, 1.f },
				.DepthClearValue = 1.f,
				.Attachments = { { render::Image::Format::DEPTH24STENCIL8 }} 
			});
	m_BloomTarget = Texture2D::CreateEXP(
		render::Image::Specification 
		{ 
			.Format		= render::Image::Format::RGBA32F,
			.Usage		= render::Image::Usage::Storage,
			.Clamp		= render::Image::Clamp::ClampToEdge, 
			.MipLevels	= m_Settings.Bloom.Samples,
			.Layers		= 1,
			.Width		= 1, .Height = 1
		}
	);
	m_SSAOTarget = Texture2D::CreateEXP(
		render::Image::Specification
		{
			.Format		= render::Image::Format::RED8UN,
			.Usage		= render::Image::Usage::Storage,
			.Clamp		= render::Image::Clamp::Repeat,
			.MipLevels	= m_Settings.Bloom.Samples, // TODO: Add normal settings ?
			.Layers		= 1,
			.Width		= 1, .Height = 1
		}
	);
	//------------------------------------------------------------------------
	// Uniforms and storage buffers                   
	//------------------------------------------------------------------------
	// TODO: Should it be like some internal part of renderer ?
	m_VisibleSpotLightIndicesBuffer		= StorageBuffer::Create(StorageBuffer::Usage::GPU, RenderAPI::SPOT_LIGHT_INDINCES_BINDING, sizeof(std::uint32_t) * RenderAPI::MAX_SPOT_LIGHTS_COUNT, Renderer::GetFramesCount(), 20);
	m_VisiblePointLightIndicesBuffer	= StorageBuffer::Create(StorageBuffer::Usage::GPU, RenderAPI::POINT_LIGHT_INDINCES_BINDING, sizeof(std::uint32_t) * RenderAPI::MAX_POINT_LIGHTS_COUNT, Renderer::GetFramesCount(), 20);
	m_SSAOSamplesBuffer					= UniformBuffer::Create(UniformBuffer::Usage::CPU_GPU, 4, sizeof(SSAO::RenderBuffer), Renderer::GetFramesCount(), 0);
	//------------------------------------------------------------------------
	// Main geometry                     
	//------------------------------------------------------------------------
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Main-Geometry-Static",
			.Shader = ShaderLibrary::Create({.Name = "Main-Geometry-Static", .FilePath = "./resources/assets/shaders/Main-Geometry.glsl"}),
			.FrameBuffer = m_MainTargetFrameBuffer,
			.VertexLayout = MGVLS,
			.BackFalceCull = true,
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, InstancedGeometryPass, this);
	}
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Main-Geometry-Animated",
			.Shader = ShaderLibrary::Create({.Name = "Main-Geometry-Aniamted", .FilePath = "./resources/assets/shaders/Main-Geometry.glsl", .MacroDefinitions = {"VS_SHADER_ANIMATED"}}),
			.FrameBuffer = m_MainTargetFrameBuffer,
			.VertexLayout = MGVLA,
			.BackFalceCull = true,
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, InstancedGeometryPass, this);
	}
	//------------------------------------------------------------------------
	// Main geometry global light shadows                  
	//------------------------------------------------------------------------
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create( // Static
		{
			.Name = "Global-Light-Shadow-Pre-Depth-Static",
			.Shader = ShaderLibrary::Create({.Name = "Global-Light-Shadow-Pre-Depth-Static", .FilePath = "./resources/assets/shaders/preprocess/Global-Light-Shadow-Pre-Depth.glsl"}),
			.FrameBuffer = m_GlobalLightShadowFrameBuffer,
			.VertexLayout = MGVLS,
			.BackFalceCull = false,
			.DepsBiasConstantFactor = 4.0f,
			.DepthBiasSlopeFactor = 8.0f,
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, GlobalLightShadowPreDepthPass, this);
	}
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create( // Animated
		{
			.Name = "Global-Light-Shadow-Pre-Depth-Animated",
			.Shader = ShaderLibrary::Create({.Name = "Global-Light-Shadow-Pre-Depth-Animated", .FilePath = "./resources/assets/shaders/preprocess/Global-Light-Shadow-Pre-Depth.glsl", .MacroDefinitions = {"VS_SHADER_ANIMATED"}}),
			.FrameBuffer = m_GlobalLightShadowFrameBuffer,
			.VertexLayout = MGVLA,
			.BackFalceCull = false,
			.DepsBiasConstantFactor = 4.0f,
			.DepthBiasSlopeFactor = 8.0f,
		}))) 
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, GlobalLightShadowPreDepthPass, this);
	}
	//------------------------------------------------------------------------
	// Main geometry omnidirectional light shadows                    
	//------------------------------------------------------------------------
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Point-Light-Shadow-Pre-Depth-Static",
			.Shader = ShaderLibrary::Create({.Name = "Point-Light-Shadow-Pre-Depth-Static", .FilePath = "./resources/assets/shaders/preprocess/Point-Light-Shadow-Pre-Depth.glsl"}),
			.FrameBuffer = m_PointLightShadowFrameBuffer,
			.VertexLayout = MGVLS,
			.BackFalceCull = false,
			.DepsBiasConstantFactor = 4.0f,
			.DepthBiasSlopeFactor = 8.0f,
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, PointLightShadowPreDepthPass, this);
	}
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Point-Light-Shadow-Pre-Depth-Animated",
			.Shader = ShaderLibrary::Create({.Name = "Point-Light-Shadow-Pre-Depth-Animated", .FilePath = "./resources/assets/shaders/preprocess/Point-Light-Shadow-Pre-Depth.glsl", .MacroDefinitions = {"VS_SHADER_ANIMATED"}}),
			.FrameBuffer = m_PointLightShadowFrameBuffer,
			.VertexLayout = MGVLA,
			.BackFalceCull = false,
			.DepsBiasConstantFactor = 4.0f,
			.DepthBiasSlopeFactor = 8.0f,
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, PointLightShadowPreDepthPass, this);
	}
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Point-Lights-Visualizing",
			.Shader = ShaderLibrary::Create({.Name = "Lights-Visualizing", .FilePath = "./resources/assets/shaders/utils/LightsVisualizing.glsl"}),
			.FrameBuffer = m_MainTargetFrameBuffer,
			.VertexLayout = MGVLS,
			.Topology = Pipeline::PrimitiveTopology::Line,
			.BackFalceCull = false
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, LightVisualizationPass, this);
		pipeline->SetActive(false);
	}
	//------------------------------------------------------------------------
	// Main geometry spot light shadows                    
	//------------------------------------------------------------------------
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Spot-Light-Shadow-Pre-Depth",
			.Shader = ShaderLibrary::Create({.Name = "Spot-Light-Shadow-Pre-Depth", .FilePath = "./resources/assets/shaders/preprocess/Spot-Light-Shadow-Pre-Depth.glsl"}),
			.FrameBuffer = m_SpotLightShadowFrameBuffer,
			.VertexLayout = MGVLS,
			.BackFalceCull = false,
			.DepsBiasConstantFactor = 4.0f,
			.DepthBiasSlopeFactor = 8.0f,
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, SpotLightShadowPreDepthPass, this);
	}
	// TODO: Aniamted !
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Spot-Lights-Visualizing",
			.Shader = ShaderLibrary::Get("Lights-Visualizing"),
			.FrameBuffer = m_MainTargetFrameBuffer,
			.VertexLayout = MGVLS,
			.Topology = Pipeline::PrimitiveTopology::LineStrip,
			.BackFalceCull = false
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, LightVisualizationPass, this);
		pipeline->SetActive(false);
	}
	//------------------------------------------------------------------------
	// Main geometry light culling pre depth and compute                    
	//------------------------------------------------------------------------
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Light-Culling-Pre-Depth",
			.Shader = ShaderLibrary::Create({.Name = "Light-Culling-Pre-Depth", .FilePath = "./resources/assets/shaders/preprocess/Tiled-Forward-Pre-Depth.glsl"}),
			.FrameBuffer = m_LightCullingPreDepthFrameBuffer,
			.VertexLayout = MGVLS
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, LightCullingPreDepthPass, this);
		pipeline->SetActive(false);
	}
	if (auto pipeline = RegisterNewPipeline(shade::ComputePipeline::Create(
		{
			.Name = "Light-Culling",
			.Shader = ShaderLibrary::Create({.Name = "Light-Culling", .FilePath = "./resources/assets/shaders/preprocess/Tiled-Forward-Light-Culling.glsl"}),
		})))
	{
		BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(pipeline->As<ComputePipeline>(), SceneRenderer, LightCullingComputePass, this);
		pipeline->SetActive(false);
	}
	//------------------------------------------------------------------------
	// Main geometry SSAO, Bloom, Color correction                    
	//------------------------------------------------------------------------
	if (auto pipeline = RegisterNewPipeline(shade::ComputePipeline::Create(
		{
			.Name = "SSAO",
			.Shader = ShaderLibrary::Create({.Name = "SSAO", .FilePath = "./resources/assets/shaders/postprocess/SSAO/Screen-Sapce-Ambien-Occlusion-Compute.glsl"}),
		})))
	{
		BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(pipeline->As<ComputePipeline>(), SceneRenderer, SSAOComputePass, this);
	}
	if (auto pipeline = RegisterNewPipeline(shade::ComputePipeline::Create(
		{
			.Name = "Bloom",
			.Shader = ShaderLibrary::Create({.Name = "Bloom", .FilePath = "./resources/assets/shaders/postprocess/Bloom/Bloom-Compute.glsl"}),
		})))
	{
		BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(pipeline->As<ComputePipeline>(), SceneRenderer, BloomComputePass, this);
	}
	if (auto pipeline = RegisterNewPipeline(shade::ComputePipeline::Create(
		{
			.Name = "Color-Correction",
			.Shader = ShaderLibrary::Create({.Name = "Color-Correction", .FilePath = "./resources/assets/shaders/postprocess/Color-Correction.glsl"}),
		})))
	{
		BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(pipeline->As<ComputePipeline>(), SceneRenderer, ColorCorrectionComputePass, this);
	}
	//------------------------------------------------------------------------
	// Debug visualizing                   
	//------------------------------------------------------------------------
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "AABB-OBB",
			.Shader = ShaderLibrary::Create({.Name = "StaticFlat", .FilePath = "./resources/assets/shaders/StaticFlat.glsl"}),
			.FrameBuffer = m_MainTargetFrameBuffer,
			.VertexLayout = MGVLS,
			.Topology = Pipeline::PrimitiveTopology::Line,
			.LineWidth = 2.f
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, FlatPipeline, this);
		pipeline->SetActive(false);
	}
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Skeleton-Bone-Visualizing",
			.Shader = ShaderLibrary::Create({.Name = "Skeleton-Bone-Visualizing", .FilePath = "./resources/assets/shaders/utils/SkeletonVisualizing.glsl" }),
			.FrameBuffer = m_MainTargetFrameBuffer,
			.Topology = Pipeline::PrimitiveTopology::Point,
			.BackFalceCull = false,
			.LineWidth = 2.f
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, SkeletonVisualizationPass, this);
		pipeline->SetActive(false);
	}
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Skeleton-Joint-Visualizing",
			.Shader = ShaderLibrary::Create({.Name = "Lights-Visualizing", .FilePath = "./resources/assets/shaders/utils/LightsVisualizing.glsl"}),
			.FrameBuffer = m_MainTargetFrameBuffer,
			.VertexLayout = MGVLS,
			.Topology = Pipeline::PrimitiveTopology::Triangle,
			.BackFalceCull = false,
			.LineWidth = 1.5f
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, LightVisualizationPass, this);
		pipeline->SetActive(false);
	}
	if (auto pipeline = RegisterNewPipeline(shade::RenderPipeline::Create(
		{
			.Name = "Grid",
			.Shader = ShaderLibrary::Create({.Name = "Grid", .FilePath = "./resources/assets/shaders/utils/Grid.glsl"}),
			.FrameBuffer = m_MainTargetFrameBuffer,
			.VertexLayout = GVL,
			.Topology = Pipeline::PrimitiveTopology::TriangleStrip,
			.BackFalceCull = false,
			.LineWidth = 15.f
		})))
	{
		BIND_PIPELINE_PROCESS_FUNCTION(pipeline->As<RenderPipeline>(), SceneRenderer, GridPass, this);
	}
	//------------------------------------------------------------------------
	// !Debug visualizing                   
	//------------------------------------------------------------------------

	m_AABBMaterial				= SharedPointer<Material>::Create();
	m_AABBMaterial->ColorDiffuse = glm::vec3(0.3, 0.9, 0.2);

	m_OBBMaterial				= SharedPointer<Material>::Create();
	m_OBBMaterial->ColorDiffuse = glm::vec3(0.9, 0.3, 0.2);

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
		// Set camera aspect base on the render target resolution
		m_Camera->SetAspect((float)m_MainTargetFrameBuffer->GetWidth() / (float)m_MainTargetFrameBuffer->GetHeight());

		CameraFrustum frustum = m_Camera->GetCameraFrustum();

		if (GetPipeline("Light-Culling-Pre-Depth")->IsActive() && GetPipeline("Light-Culling")->IsActive())
		{
			if (m_LightCullingPreDepthFrameBuffer->GetWidth() != m_MainTargetFrameBuffer->GetWidth() || m_LightCullingPreDepthFrameBuffer->GetHeight() != m_MainTargetFrameBuffer->GetHeight())
			{
				m_LightCullingPreDepthFrameBuffer->Resize(m_MainTargetFrameBuffer->GetWidth(), m_MainTargetFrameBuffer->GetHeight());
			}
		}
		if (GetPipeline("Bloom")->IsActive())
		{
			if (m_BloomTarget->GetWidth() != m_MainTargetFrameBuffer->GetWidth() ||
				m_BloomTarget->GetHeight() != m_MainTargetFrameBuffer->GetHeight() ||
				m_BloomTarget->GetImage()->GetSpecification().MipLevels != m_Settings.Bloom.Samples)
			{
				m_BloomTarget->Resize(m_MainTargetFrameBuffer->GetWidth(), m_MainTargetFrameBuffer->GetHeight(), m_Settings.Bloom.Samples);
			}
		}
		if (GetPipeline("SSAO")->IsActive())
		{
			if (m_SSAOTarget->GetWidth() != m_MainTargetFrameBuffer->GetWidth() ||
				m_SSAOTarget->GetHeight() != m_MainTargetFrameBuffer->GetHeight())
			{
				m_SSAOTarget->Resize(m_MainTargetFrameBuffer->GetWidth(), m_MainTargetFrameBuffer->GetHeight());
			}
		}

		scene->View<DirectionalLightComponent, TransformComponent>().Each([&](ecs::Entity& entity, DirectionalLightComponent& light, TransformComponent& transform)
			{
				//transformRenderer::SubmitLight(light, scene->ComputePCTransform(entity).first, m_Camera); 
				Renderer::SubmitLight(light, transform.GetForwardDirection(), m_Camera);
			});

		scene->View<OmnidirectionalLightComponent, TransformComponent>().Each([&](ecs::Entity& entity, OmnidirectionalLightComponent& light, TransformComponent& transform)
			{
				// Check if point light within camera frustum 
				glm::mat4 pcTransform = scene->ComputePCTransform(entity).first;

				if (frustum.IsInFrustum({ pcTransform[3].x, pcTransform[3].y, pcTransform[3].z }, light->Distance))
				{
					Renderer::SubmitLight(light, pcTransform, m_Camera); 

					/* In case we want to use point light sphere during instance rendering we need reuse deafult sphere and apply changes only to transform matrix.*/
					pcTransform = glm::scale(pcTransform, glm::vec3(light->Distance));
					Renderer::SubmitStaticMesh(GetPipeline("Point-Lights-Visualizing"), m_Sphere, m_LightVisualizingMaterial, nullptr, pcTransform);
				}
			});

		scene->View<SpotLightComponent, TransformComponent>().Each([&](ecs::Entity& entity, SpotLightComponent& light, TransformComponent& transform)
			{
				glm::mat4 pcTransform = scene->ComputePCTransform(entity).first;
			
				float radius = light->Distance * glm::acos(glm::radians(light->MaxAngle));
	
				if (frustum.IsInFrustum({ pcTransform[3].x, pcTransform[3].y, pcTransform[3].z }, glm::normalize(glm::mat3(pcTransform) * glm::vec3(0.f, 0.f, 1.f)), light->Distance, radius))
				{
					Renderer::SubmitLight(light, pcTransform, m_Camera);
					/* In case we want to use spot light cone during instance rendering we need reuse deafult cone and apply changes only to transform matrix.*/
					pcTransform = glm::scale(pcTransform, glm::vec3(radius, radius, light->Distance));
					Renderer::SubmitStaticMesh(GetPipeline("Spot-Lights-Visualizing"), m_Cone, m_LightVisualizingMaterial, nullptr, pcTransform);
				}
			});

		scene->View<Asset<Model>, TransformComponent>().Each([&](ecs::Entity& entity, Asset<Model>& model, TransformComponent& transform)
			{
				auto pcTransform = scene->ComputePCTransform(entity).first; // Frusturm culling need matrix without compensation
				bool isModelInFrustrum = true;

				Asset<animation::AnimationGraph> animationGraph = (entity.HasComponent<AnimationGraphComponent>()) ? entity.GetComponent<AnimationGraphComponent>().AnimationGraph : nullptr;
				animation::Pose* finalPose = (animationGraph) ? animationGraph->GetOutputPose() : nullptr;

				for (const auto& mesh : *model)
				{
					//if (frustum.IsInFrustum(pcTransform, mesh->GetMinHalfExt(), mesh->GetMaxHalfExt()))
					{
						isModelInFrustrum = true;

						if (finalPose && mesh->GetLod(0).Bones.size())
						{
							Renderer::SubmitStaticMesh(GetPipeline("Main-Geometry-Animated"), mesh, mesh->GetMaterial(), model, pcTransform);
							Renderer::SubmitStaticMesh(GetPipeline("Global-Light-Shadow-Pre-Depth-Animated"), mesh, mesh->GetMaterial(), model, pcTransform);
						}
						else
						{
							Renderer::SubmitStaticMesh(GetPipeline("Main-Geometry-Static"), mesh, mesh->GetMaterial(), model, pcTransform);
							Renderer::SubmitStaticMesh(GetPipeline("Global-Light-Shadow-Pre-Depth-Static"), mesh, mesh->GetMaterial(), model, pcTransform);
						}
					
						Renderer::SubmitStaticMesh(GetPipeline("Light-Culling-Pre-Depth"), mesh, nullptr, model, pcTransform);
					}

					if (GetPipeline("Point-Light-Shadow-Pre-Depth-Static")->IsActive() || GetPipeline("Point-Light-Shadow-Pre-Depth-Animated")->IsActive())
					{
						for (std::uint32_t index = 0; index < Renderer::GetSubmitedPointLightCount(); index++)
						{
							auto& renderData = Renderer::GetSubmitedOmnidirectionalLightRenderData(index);

							if (OmnidirectionalLight::GetRenderSettings().SplitBySides)
							{
								// Split render passes for each side of cube 
								for (std::uint32_t side = 0; side < 6; side++)
								{
									// Check if mesh inside point light for shadow pass  
									if (OmnidirectionalLight::IsMeshInside(renderData.Cascades[side].ViewProjectionMatrix, pcTransform, mesh->GetMinHalfExt(), mesh->GetMaxHalfExt()))
									{
										std::size_t seed = index; glm::detail::hash_combine(seed, side);
										if (finalPose && mesh->GetLod(0).Bones.size())
										{
											Renderer::SubmitStaticMesh(GetPipeline("Point-Light-Shadow-Pre-Depth-Animated"), mesh, nullptr, model, pcTransform, seed);
										}
										else
										{
											Renderer::SubmitStaticMesh(GetPipeline("Point-Light-Shadow-Pre-Depth-Static"), mesh, nullptr, model, pcTransform, seed);
										}
									}
								}
							}
							else
							{
								if (OmnidirectionalLight::IsMeshInside(renderData.Position, renderData.Distance, pcTransform, mesh->GetMinHalfExt(), mesh->GetMaxHalfExt()))
								{
									if (finalPose && mesh->GetLod(0).Bones.size())
									{
										Renderer::SubmitStaticMesh(GetPipeline("Point-Light-Shadow-Pre-Depth-Animated"), mesh, mesh->GetMaterial(), model, pcTransform, index);
									}
									else
									{
										Renderer::SubmitStaticMesh(GetPipeline("Point-Light-Shadow-Pre-Depth-Static"), mesh, mesh->GetMaterial(), model, pcTransform, index);
									}
									
								}	
							}
						}
					}
					 // Check if mesh inside spot light for shadow pass  
					if (GetPipeline("Spot-Light-Shadow-Pre-Depth"))
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

					// OBB Visualization
					if (GetPipeline("AABB-OBB")->IsActive())
					{
						/* In case we want to use aabb box during instance rendering we need reuse deafult box min and max ext and apply changes only to transform matrix.*/
						// Translate the cpTransform matrix to the center of the mesh
						glm::mat4 permeshTransform = glm::translate(scene->ComputePCTransform(entity).second, (mesh->GetMinHalfExt() + mesh->GetMaxHalfExt()) / 2.f);
						// Scale the cpTransform matrix using the ratio of the half extents of the mesh and the bounding box
						permeshTransform = glm::scale(permeshTransform, (mesh->GetMaxHalfExt() - mesh->GetMinHalfExt()) / (m_OBB->GetMaxHalfExt() - m_OBB->GetMinHalfExt()));
						// Submit aabb for rendering 
						Renderer::SubmitStaticMesh(GetPipeline("AABB-OBB"), m_OBB, m_OBBMaterial, nullptr, permeshTransform);
					}
				}

				if (isModelInFrustrum && finalPose)
				{
					// Only for the selected entity 
					if (activeEntity == entity) // TODO: check if pipelines are enabled to avoid using this part of the code 
					{
						// Create a copy of skeleton transforms
						static SharedPointer<std::vector<animation::Pose::GlobalTransform>> skVisualize = SharedPointer<std::vector<animation::Pose::GlobalTransform>>::Create(RenderAPI::MAX_BONES_PER_INSTANCE);

						for (const auto& [name, bone] : finalPose->GetSkeleton()->GetBones())
						{
							auto parentId = finalPose->GetBoneGlobalTransform(bone.ID).ParentId;

							glm::mat4 boneT = finalPose->GetBoneGlobalTransform(bone.ID).Transform;
							glm::mat4 parentBoneT = (parentId != ~0) ? finalPose->GetBoneGlobalTransform(parentId).Transform : glm::mat4(0.0);
							glm::mat4 parentInverseBindPoseT = (parentId != ~0) ? finalPose->GetSkeleton()->GetBone(parentId)->InverseBindPose : glm::mat4(0.0);

							if (finalPose->HasInverseBindPose())
							{
								// If the inverse bind pose was applied earlier, we need to remove it 
								boneT = boneT * glm::inverse(bone.InverseBindPose);
								parentBoneT = parentBoneT * glm::inverse(parentInverseBindPoseT);
							}

							skVisualize->at(bone.ID).ParentId = parentId;
							// Set the global bone transform 
							skVisualize->at(bone.ID).Transform = pcTransform * boneT;

							// Calculate the distance between parent and child bones to determine scale factor for the spheres
							const float scale = glm::distance(boneT * glm::vec4(0, 0, 0, 1), parentBoneT * glm::vec4(0, 0, 0, 1)) * 0.06f;

							if (parentId != ~0)
							{
								// Submit only those joints that have parent bones 
								Renderer::SubmitStaticMesh(GetPipeline("Skeleton-Joint-Visualizing"), m_Sphere, m_JoinVisualizingMaterial, nullptr, pcTransform * glm::scale(parentBoneT, glm::vec3(scale)));
							}
						}

						// Submit bones visualization, dummy invocation for the geometry shader 
						Renderer::SubmitStaticMesh(GetPipeline("Skeleton-Bone-Visualizing"), nullptr, nullptr, model, pcTransform);
						// Submit bone matrices for visualization 
						Renderer::SubmitBoneTransforms(GetPipeline("Skeleton-Bone-Visualizing"), model, skVisualize);
					}


					if (!finalPose->HasInverseBindPose())
					{
						for (auto& [name, bone] : finalPose->GetSkeleton()->GetBones())
						{
							finalPose->GetBoneGlobalTransforms()->at(bone.ID).Transform *= bone.InverseBindPose;
						}

						finalPose->MarkHasInverseBindPose(true);
					}
					
					Renderer::SubmitBoneTransforms(GetPipeline("Global-Light-Shadow-Pre-Depth-Animated"), model, finalPose->GetBoneGlobalTransforms());
					Renderer::SubmitBoneTransforms(GetPipeline("Point-Light-Shadow-Pre-Depth-Animated"), model, finalPose->GetBoneGlobalTransforms());
					Renderer::SubmitBoneTransforms(GetPipeline("Main-Geometry-Animated"), model, finalPose->GetBoneGlobalTransforms());
				}

				// AABB Visualization
				//if (entity.HasComponent<RigidBodyComponent>())
				//{
				//	auto& rigidBody = entity.GetComponent<RigidBodyComponent>();

				//	for (const auto& ext : rigidBody.GetExtensions())
				//	{
				//		auto minExt = ext.MinHalfExtWorldSpace;
				//		auto maxExt = ext.MaxHalfExtWorldSpace;

				//		glm::mat<4, 4, physic::scalar_t> permeshTransform = glm::translate(glm::mat<4, 4, physic::scalar_t>(1.0), (minExt + maxExt) / 2.0);
				//		permeshTransform = glm::scale(permeshTransform,
				//			(maxExt - minExt) / glm::vec<3, physic::scalar_t>(m_OBB->GetMaxHalfExt() - m_OBB->GetMinHalfExt())
				//		);

				//		//Renderer::SubmitStaticMesh(m_AABB_OBB_Pipeline, m_OBB, m_AABBMaterial, nullptr, permeshTransform);
				//	}
				//	
				//	//for (auto& contactPoint : rigidBody.GetCollisionContacts())
				//	//{
				//	//	glm::mat4 pointTransform = glm::translate(pcTransform, static_cast<glm::vec3>(contactPoint));

				//	//	glm::vec3 p, s; glm::quat r;
				//	//	math::DecomposeMatrix(pointTransform, p, r, s);
				//	//	glm::mat4 mat = glm::translate(glm::mat4(1.f), p) * glm::toMat4(r) * glm::scale(glm::mat4(1.f), glm::vec3(0.1f));

				//	//	pointTransform *= glm::scale(glm::mat4(1.f), glm::vec3(0.1f));

				//	//	//Renderer::SubmitStaticMesh(m_FlatPipeline, m_Sphere, nullptr, mat);
				//	//}

				//	/*for (auto& collider : rigidBody.GetColliders())
				//	{
				//		
				//		if (collider->GetShape() == physic::CollisionShape::Shape::Mesh)
				//		{
				//			SharedPointer<Drawable> hullMesh = SharedPointer<Drawable>::Create();

				//			physic::MeshShape* meshShape = reinterpret_cast<physic::MeshShape*>(
				//				const_cast<physic::CollisionShape*>(collider.Raw())
				//				);

				//			auto& vertices =  meshShape->GetVertices();

				//			for (std::size_t i = 0; i < vertices.size(); i++)
				//			{
				//				hullMesh->AddVertex({ vertices[i] });
				//				hullMesh->AddIndex(i);
				//				
				//				
				//			}

				//			pcTransform *= glm::scale(glm::mat4(1.f), glm::vec3(1.0f));
				//			Renderer::SubmitStaticMesh(m_AABBPipeline, hullMesh, nullptr, pcTransform);
				//		}
				//	}*/
				//	

				//	/*auto& contactPoint = entity.GetComponent<RigidBodyComponent>().GetLocalContactPoint();
				//	glm::mat4 pointTransform = glm::translate(pcTransform, static_cast<glm::vec3>(contactPoint));

				//	glm::vec3 p, s; glm::quat r;
				//	math::DecomposeMatrix(pointTransform, p, r, s);
				//	glm::mat4 mat = glm::translate(glm::mat4(1.f), p) * glm::toMat4(r) * glm::scale(glm::mat4(1.f), glm::vec3(0.1f));
				//	
				//	pointTransform *= glm::scale(glm::mat4(1.f), glm::vec3(0.1f));
				//	
				//	Renderer::SubmitStaticMesh(m_CollisionContanctPointPipline, m_Sphere, nullptr, mat);*/
				//}
			});

		// Submit grid for rendering
		
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

	m_Settings.RenderSettings.SSAOEnabled					= GetPipeline("SSAO")->IsActive();
	m_Settings.RenderSettings.LightCulling					= GetPipeline("Light-Culling-Pre-Depth")->IsActive() && GetPipeline("Light-Culling")->IsActive();
	m_Settings.RenderSettings.DirectionalLightShadows		= GetPipeline("Global-Light-Shadow-Pre-Depth-Static")->IsActive() || GetPipeline("Global-Light-Shadow-Pre-Depth-Animated")->IsActive();
	m_Settings.RenderSettings.OmnidirectionalLightShadows	= GetPipeline("Point-Light-Shadow-Pre-Depth-Static")->IsActive() || GetPipeline("Point-Light-Shadow-Pre-Depth-Animated")->IsActive();
	m_Settings.RenderSettings.SpotLightShadows				= GetPipeline("Spot-Light-Shadow-Pre-Depth")->IsActive();

	if (m_Camera != nullptr)
	{
		m_MainCommandBuffer->Begin(curentFrameIndex);

		Renderer::BeginScene(m_Camera, m_Settings.RenderSettings, curentFrameIndex);
		{
			{
				Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Light-Culling-Pre-Depth"), curentFrameIndex);
				Renderer::ExecuteComputePipeline(GetPipeline("Light-Culling"), curentFrameIndex);
			}
			{
				bool gClear		 = Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Global-Light-Shadow-Pre-Depth-Static"), curentFrameIndex, true);
				gClear			+= Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Global-Light-Shadow-Pre-Depth-Animated"), curentFrameIndex, !gClear);
				
				bool pClear		 = Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Point-Light-Shadow-Pre-Depth-Static"), curentFrameIndex, true);
				pClear			+= Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Point-Light-Shadow-Pre-Depth-Animated"), curentFrameIndex, !pClear);

				bool sClear		 = Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Spot-Light-Shadow-Pre-Depth"), curentFrameIndex, true);
				
				
			}

			bool mClear			 = Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Main-Geometry-Static"), curentFrameIndex, true);
			mClear				+=Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Main-Geometry-Animated"), curentFrameIndex, !mClear);

			{
					Renderer::ExecuteComputePipeline(GetPipeline("SSAO"), curentFrameIndex);
					Renderer::ExecuteComputePipeline(GetPipeline("Bloom"), curentFrameIndex);
					Renderer::ExecuteComputePipeline(GetPipeline("Color-Correction"), curentFrameIndex);
			}
			{
					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("AABB-OBB"), curentFrameIndex, !mClear);
					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Point-Lights-Visualizing"), curentFrameIndex, !mClear);
					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Spot-Lights-Visualizing"), curentFrameIndex, !mClear);
					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Skeleton-Bone-Visualizing"), curentFrameIndex, !mClear);
					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Skeleton-Joint-Visualizing"), curentFrameIndex, !mClear);

					Renderer::ExecuteSubmitedRenderPipeline(GetPipeline("Grid"), curentFrameIndex, !mClear);
			}
		}
		Renderer::EndScene(curentFrameIndex);
		m_MainCommandBuffer->End(curentFrameIndex);
		m_MainCommandBuffer->Submit(curentFrameIndex);

		//Renderer::QueryResults(curentFrameIndex);
	}
}

void shade::SceneRenderer::OnEvent(SharedPointer<Scene>& scene, const Event& event, const FrameTimer& deltaTime)
{

}

shade::SharedPointer<shade::FrameBuffer>& shade::SceneRenderer::GetMainTargetFrameBuffer()
{
	return m_MainTargetFrameBuffer;
}

shade::SharedPointer<shade::Texture2D>& shade::SceneRenderer::GetBloomRenderTarget()
{
	return m_BloomTarget;
}

shade::SharedPointer<shade::Texture2D>& shade::SceneRenderer::GetSAAORenderTarget()
{
	return m_SSAOTarget;
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

std::map<std::string, shade::SharedPointer<shade::Pipeline>>& shade::SceneRenderer::GetPipelines()
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
	pipeline->SetUniform(m_MainCommandBuffer, sizeof(glm::uvec2), glm::value_ptr(resolution), frameIndex);
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

	// Не поулчается пушить от сюда в эти пайплайны, нужно сделать промежуточное значение, хранит как настройки лайт кулинга !!
	GetPipeline("Main-Geometry-Static")->As<RenderPipeline>().SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &tilesCountX, frameIndex, Shader::Type::Fragment);
	GetPipeline("Main-Geometry-Animated")->As<RenderPipeline>().SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &tilesCountX, frameIndex, Shader::Type::Fragment );

	// update resources, dispatch the compute shader and set a memory barrier
	pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
	pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
	pipeline->SetBarrier(m_MainCommandBuffer, Pipeline::Stage::ComputeShader, Pipeline::Stage::FragmentShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
	Renderer::EndCompute(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::GlobalLightShadowPreDepthPass(SharedPointer<RenderPipeline>& pipeline, const render::SubmitedInstances& instances, const render::SubmitedSceneRenderData& data, std::uint32_t frameIndex, bool isForceClear)
{
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, isForceClear);

	for (auto& [instance, materials] : instances.Instances)
	{
		if (GetPipeline("Global-Light-Shadow-Pre-Depth-Animated").Raw() == pipeline.Raw())
			Renderer::UpdateSubmitedBonesData(m_MainCommandBuffer, pipeline, materials.ModelHash, frameIndex);

		for (auto& [lod, material] : materials.Materials)
		{
			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			// Update the submitted material
			//Renderer::UpdateSubmitedMaterial(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);

			for (std::uint32_t cascade = 0; cascade < DirectionalLight::SHADOW_CASCADES_COUNT; cascade++)
			{
				pipeline->SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &cascade, frameIndex, Shader::Type::Vertex);

				if (GetPipeline("Global-Light-Shadow-Pre-Depth-Animated").Raw() == pipeline.Raw())
				{
					Renderer::DrawSubmitedInstancedAnimated(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);
				}
				else
				{
					Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);
				}
			}
		}
	}

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
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, (isForceClear) && (bool)Renderer::GetSubmitedPointLightCount(), Renderer::GetSubmitedPointLightCount() * 6);
	// Update buffers 
	//pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);

	for (auto& [instance, materials] : instances.Instances)
	{
		if (GetPipeline("Point-Light-Shadow-Pre-Depth-Animated").Raw() == pipeline.Raw())
			Renderer::UpdateSubmitedBonesData(m_MainCommandBuffer, pipeline, materials.ModelHash, frameIndex);

		for (auto& [lod, material] : materials.Materials)
		{
			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);

			for (std::uint32_t index = 0; index < Renderer::GetSubmitedPointLightCount(); index++)
			{
				pipeline->SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &index, frameIndex, Shader::Type::Vertex);

				for (std::uint32_t side = 0; side < 6; side++)
				{
					pipeline->SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &side, frameIndex, Shader::Type::Vertex, sizeof(std::uint32_t));

					if (OmnidirectionalLight::GetRenderSettings().SplitBySides)
					{
						// Draw the submitted instance
						std::size_t seed = index; glm::detail::hash_combine(seed, side);
						
						if (GetPipeline("Point-Light-Shadow-Pre-Depth-Animated").Raw() == pipeline.Raw())
						{
							Renderer::DrawSubmitedInstancedAnimated(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod, seed);
						}
						else
						{
							Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod);
						}
					}
					else
					{
						if (GetPipeline("Point-Light-Shadow-Pre-Depth-Animated").Raw() == pipeline.Raw())
						{
							Renderer::DrawSubmitedInstancedAnimated(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod, index);
						}
						else
						{
							Renderer::DrawSubmitedInstanced(m_MainCommandBuffer, pipeline, instance, material, frameIndex, lod, index);
						}
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
	std::uint32_t tilesCountX = 95;
	// Begin rendering
	Renderer::BeginRender(m_MainCommandBuffer, pipeline, frameIndex, isForceClear);
	// Set the visible point light and spot light indices for the pipeline to use during rendering.
	pipeline->SetResource(m_VisiblePointLightIndicesBuffer, Pipeline::Set::PerInstance, frameIndex);
	pipeline->SetResource(m_VisibleSpotLightIndicesBuffer, Pipeline::Set::PerInstance, frameIndex);
	pipeline->SetTexture(m_GlobalLightShadowFrameBuffer->GetDepthAttachment(), Pipeline::Set::PerInstance, RenderAPI::GLOBAL_SHADOW_MAP_BINDING, frameIndex);
	pipeline->SetTexture(m_SpotLightShadowFrameBuffer->GetDepthAttachment(), Pipeline::Set::PerInstance, RenderAPI::SPOT_SHADOW_MAP_BINDING, frameIndex);
	pipeline->SetTexture(m_PointLightShadowFrameBuffer->GetDepthAttachment(), Pipeline::Set::PerInstance, RenderAPI::POINT_SHADOW_MAP_BINDING, frameIndex);
	
	//GetPipeline("Main-Geometry-Static")->As<RenderPipeline>().SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &tilesCountX, frameIndex, Shader::Type::Fragment);
	//GetPipeline("Main-Geometry-Animated")->As<RenderPipeline>().SetUniform(m_MainCommandBuffer, sizeof(std::uint32_t), &tilesCountX, frameIndex, Shader::Type::Fragment);

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
	const auto& texture = m_MainTargetFrameBuffer->GetTextureAttachment(0);
	// Calculate the number of execution groups required to cover the entire texture.
	glm::uvec3 executionGroups
	{
		std::ceil(static_cast<float>(texture->GetWidth()) / 16.f), std::ceil(static_cast<float>(texture->GetWidth()) / 16.0f), 1
	};
	// Get the render data settings required for color correction.
	ColorCorrection::RenderData renderData = m_Settings.ColorCorrection.GetRenderData();
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
	const auto& mainTarget = m_MainTargetFrameBuffer->GetTextureAttachment(0);
	const std::uint32_t SAMPLES = m_Settings.Bloom.Samples;

	glm::uvec3 executionGroups { std::ceil(static_cast<float>(mainTarget->GetWidth()) / (16.f)), std::ceil(static_cast<float>(mainTarget->GetHeight()) / (16.0f)), 1 };
	BloomSettings::RenderData bloomData = m_Settings.Bloom.GetRenderData();

	Renderer::BeginCompute(m_MainCommandBuffer, pipeline, frameIndex);
	{
		/////////////////////////HDR////////////////////////////////
		bloomData.Stage = BloomSettings::Stage::HDR;
		pipeline->SetTexture(mainTarget, Pipeline::Set::PerInstance, 1, frameIndex);
		pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 2, frameIndex, 0);
		//Proxy 
		pipeline->SetTexture(m_BloomTarget, Pipeline::Set::PerInstance, 0, frameIndex);

		pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);

		pipeline->SetUniform(m_MainCommandBuffer, sizeof(SceneRenderer::BloomSettings::RenderData), &bloomData, frameIndex);

		pipeline->SetBarrier(m_MainCommandBuffer, mainTarget, Pipeline::Stage::ColorAttachmentOutput, Pipeline::Stage::ComputeShader, Pipeline::Access::ColorAttachmentWrite, Pipeline::Access::ShaderRead, frameIndex);
		pipeline->Dispatch(m_MainCommandBuffer, (std::uint32_t)executionGroups.x, (std::uint32_t)executionGroups.y, executionGroups.z, frameIndex);
		pipeline->SetBarrier(m_MainCommandBuffer, m_BloomTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
	}
	{
		/////////////////////////Downsample//////////////////////////
		bloomData.Stage = BloomSettings::Stage::DownSample;
		for (std::uint32_t mip = 0; mip < SAMPLES - 1; mip++)
		{
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 0, frameIndex, mip);
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 2, frameIndex, mip + 1);
			//Proxy 
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 1, frameIndex, mip + 1);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);

			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SceneRenderer::BloomSettings::RenderData), &bloomData, frameIndex);

			pipeline->Dispatch(m_MainCommandBuffer, (std::uint32_t)std::ceil(executionGroups.x / float(mip + 1.f)), (std::uint32_t)std::ceil(executionGroups.y / float(mip + 1.f)), executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, m_BloomTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
	}
	{
		/////////////////////////////Upsample//////////////////////////////
		bloomData.Stage = BloomSettings::Stage::UpSample;
		for (std::uint32_t mip = SAMPLES - 1; mip > 0; mip--)
		{
			bloomData.Lod = mip;
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 0, frameIndex, mip);
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 2, frameIndex, mip - 1);
			//Proxy 
			pipeline->SetTexturePerMipLevel(m_BloomTarget, Pipeline::Set::PerInstance, 1, frameIndex, mip - 1);

			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SceneRenderer::BloomSettings::RenderData), &bloomData, frameIndex);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			pipeline->Dispatch(m_MainCommandBuffer, (std::uint32_t)std::ceil(executionGroups.x / float(mip)), (std::uint32_t)std::ceil(executionGroups.y / float(mip)), executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, m_BloomTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
	}
	{
		/////////////////////////////Combine//////////////////////////////
		bloomData.Stage = BloomSettings::Stage::Combine;
		pipeline->SetTexture(m_BloomTarget,	Pipeline::Set::PerInstance, 1, frameIndex);
		pipeline->SetTexture(mainTarget,								Pipeline::Set::PerInstance, 2, frameIndex);
		// Proxy
		pipeline->SetTexture(m_BloomTarget,   Pipeline::Set::PerInstance, 0, frameIndex);

		pipeline->SetUniform(m_MainCommandBuffer, sizeof(SceneRenderer::BloomSettings::RenderData), &bloomData, frameIndex);

		pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
		pipeline->Dispatch(m_MainCommandBuffer, (std::uint32_t)executionGroups.x, (std::uint32_t)executionGroups.y, executionGroups.z, frameIndex);
		pipeline->SetBarrier(m_MainCommandBuffer, mainTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
	}
	Renderer::EndCompute(m_MainCommandBuffer, pipeline, frameIndex);
}

void shade::SceneRenderer::SSAOComputePass(SharedPointer<ComputePipeline>& pipeline, std::uint32_t frameIndex)
{
	const auto& mainTarget		= m_MainTargetFrameBuffer->GetTextureAttachment(0);
	const auto& positionTexture	= m_MainTargetFrameBuffer->GetTextureAttachment(1);
	const auto& normalTexture	= m_MainTargetFrameBuffer->GetTextureAttachment(2);

	glm::uvec3 executionGroups { std::ceil(static_cast<float>(mainTarget->GetWidth()) / 16.f), std::ceil(static_cast<float>(mainTarget->GetHeight()) / 16.0f), 1 };

	SSAO::RenderData	renderData = m_Settings.SSAO.GetRenderData();
	SSAO::RenderBuffer	renderBuffer = m_Settings.SSAO.GetRenderBuffer();

	m_SSAOSamplesBuffer->SetData(sizeof(SSAO::RenderBuffer), &renderBuffer, frameIndex);

	Renderer::BeginCompute(m_MainCommandBuffer, pipeline, frameIndex);
	{
		pipeline->SetResource(m_SSAOSamplesBuffer, Pipeline::Set::PerInstance, frameIndex);
		{
			////////////////////////////GENERATE////////////////////////////////////////////
			renderData.Stage = SSAO::Stage::Generate;
			pipeline->SetTexture(positionTexture, Pipeline::Set::PerInstance, 0, frameIndex);
			pipeline->SetTexture(normalTexture, Pipeline::Set::PerInstance, 1, frameIndex);
			pipeline->SetTexture(m_SSAOTarget, Pipeline::Set::PerInstance, 2, frameIndex);

			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SSAO::RenderData), &renderData, frameIndex);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, m_SSAOTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
		{
			////////////////////////////BLUR H////////////////////////////////////////////
			renderData.Stage = SSAO::Stage::BlurHorizontal;
			pipeline->SetTexture(m_SSAOTarget, Pipeline::Set::PerInstance, 0, frameIndex);
			pipeline->SetTexture(m_SSAOTarget, Pipeline::Set::PerInstance, 1, frameIndex);

			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SSAO::RenderData), &renderData, frameIndex);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
		{
			////////////////////////////BLUR V////////////////////////////////////////////
			renderData.Stage = SSAO::Stage::BlurVertical;
			pipeline->SetTexture(m_SSAOTarget, Pipeline::Set::PerInstance, 0, frameIndex);
			pipeline->SetTexture(m_SSAOTarget, Pipeline::Set::PerInstance, 1, frameIndex);
			
			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SSAO::RenderData), &renderData, frameIndex);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
		{
			////////////////////////////COMBINE////////////////////////////////////////////
			renderData.Stage = SSAO::Stage::Combine;
			pipeline->SetTexture(m_SSAOTarget, Pipeline::Set::PerInstance, 0, frameIndex);
			pipeline->SetTexture(mainTarget, Pipeline::Set::PerInstance, 1, frameIndex);
			pipeline->SetTexture(m_SSAOTarget, Pipeline::Set::PerInstance, 2, frameIndex);

			pipeline->SetUniform(m_MainCommandBuffer, sizeof(SSAO::RenderData), &renderData, frameIndex);

			pipeline->UpdateResources(m_MainCommandBuffer, frameIndex);
			pipeline->Dispatch(m_MainCommandBuffer, executionGroups.x, executionGroups.y, executionGroups.z, frameIndex);
			pipeline->SetBarrier(m_MainCommandBuffer, mainTarget, Pipeline::Stage::ComputeShader, Pipeline::Stage::ComputeShader, Pipeline::Access::ShaderWrite, Pipeline::Access::ShaderRead, frameIndex);
		}
	}
	Renderer::EndCompute(m_MainCommandBuffer, pipeline, frameIndex);
}
