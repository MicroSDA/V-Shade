#include "shade_pch.h"
#include "Scene.h"
#include <shade/core/asset/AssetManager.h>
#include <shade/core/scripting/ScriptManager.h>
// TODO: TEMPORARY
#include <shade/core/camera/Camera.h>
#include <ctti/type_id.hpp>
#include <ctti/nameof.hpp>

std::unordered_map<std::string, shade::SharedPointer<shade::Scene>> shade::Scene::m_sScenes;
shade::SharedPointer<shade::Scene> shade::Scene::m_sActiveScene;

shade::Scene::Scene(const std::string& name)
{
	m_Name = name;
}

shade::SharedPointer<shade::Scene> shade::Scene::Create(const std::string& name)
{
	if (m_sScenes.find(name) != m_sScenes.end())
		SHADE_CORE_ERROR("Scene '{0}' already exist.", name)
	else
	{
		m_sScenes.insert({ name, SharedPointer<Scene>::Create(name) });
		return m_sScenes.at(name);
	}
}

shade::SharedPointer<shade::Scene>& shade::Scene::GetScene(const std::string& name)
{
	if (m_sScenes.find(name) == m_sScenes.end())
		SHADE_CORE_ERROR("Requested '{0}' has not been found!", name)
	else
		return m_sScenes.at(name);
}

shade::SharedPointer<shade::Scene>& shade::Scene::GetActiveScene()
{
	return m_sActiveScene;
}

shade::SharedPointer<shade::Scene>& shade::Scene::SetActiveScene(const std::string& name)
{
	if (m_sScenes.find(name) == m_sScenes.end())
		SHADE_CORE_ERROR("Requested '{0}' has not been found!", name)
	else
		return m_sActiveScene = m_sScenes.at(name);
}

void shade::Scene::NativeScriptsUpdate(const shade::FrameTimer& deltaTime)
{
	View<NativeScriptComponent>().Each([=](ecs::Entity& entity, NativeScriptComponent& script)
		{
			if (script.InstantiateScript)
			{
				if (!script.m_pInstance)
				{
					script.m_pInstance = script.InstantiateScript();
					script.m_pInstance->m_Entity = entity;
					script.m_pInstance->OnCreate();
				}
				else if (script.m_pInstance->IsUpdate())
					script.m_pInstance->OnUpdate(deltaTime);
			}
		});
}

void shade::Scene::GraphsUpdate(const shade::FrameTimer& deltaTime)
{
	View<shade::AnimationGraphComponent>().Each([&](shade::ecs::Entity& entity, shade::AnimationGraphComponent& graph)
		{
			if (graph.AnimationGraph) graph.AnimationGraph->ProcessBranch(deltaTime);
		});
}

shade::ecs::Entity shade::Scene::GetPrimaryCamera()
{
	ecs::Entity _entity;

	View<CameraComponent>().Each([&_entity](ecs::Entity& entity, CameraComponent& camera)
		{
			if (camera->IsPrimary())
				_entity = entity;
		});

	return _entity; // Make sure that entity is valid !
}

void shade::Scene::OnPlayStart()
{
}

void shade::Scene::OnPlaying(const shade::FrameTimer& deltaTime)
{
	NativeScriptsUpdate(deltaTime);
	GraphsUpdate(deltaTime);
}

void shade::Scene::OnPlayStop()
{
}

const bool& shade::Scene::IsPlaying() const
{
	return m_IsPlaying;
}
void shade::Scene::SetPlaying(const bool& play)
{
	m_IsPlaying = play;
}

void shade::Scene::SetAsActiveScene()
{
	SetActiveScene(m_Name);
}
std::pair<glm::mat4, glm::mat4> shade::Scene::ComputePCTransform(ecs::Entity& entity)
{
	std::pair<glm::mat4, glm::mat4> tComplMat = { glm::identity<glm::mat4>(), glm::identity<glm::mat4>() };

	if (entity.HasParent())
	{
		ecs::Entity parent = entity.GetParent();
		tComplMat = ComputePCTransform(parent);
	}

	if (entity.HasComponent<shade::TransformComponent>())
	{
		auto& transform = entity.GetComponent<shade::TransformComponent>();
		glm::mat4 tMat = transform.GetModelMatrix(); 

		glm::mat4 tMatWithRootMotion = tMat; 

		if (entity.HasComponent<shade::AnimationGraphComponent>())
		{
			if (const animation::Pose* pose = entity.GetComponent<shade::AnimationGraphComponent>().AnimationGraph->GetOutputPose())
			{
				if (pose->HasRootMotion())
				{
					glm::mat4 dif = glm::toMat4(glm::conjugate(pose->GetRootMotion().Rotation.Current)) * glm::translate(glm::identity<glm::mat4>(), -pose->GetRootMotion().Translation.Current);
					tMatWithRootMotion = tMat * dif;
				}
			}
		}
		return { tComplMat.second * tMatWithRootMotion, tComplMat.second * tMat };
	}
	else
	{
		return tComplMat;
	}
}

glm::mat4 shade::Scene::ComputePCTransformWithoutRootMotion(ecs::Entity& entity)
{
	glm::mat4 tComplMat = glm::identity<glm::mat4>();

	if (entity.HasParent())
	{
		ecs::Entity parent = entity.GetParent();
		tComplMat = ComputePCTransformWithoutRootMotion(parent);
	}

	if (entity.HasComponent<shade::TransformComponent>())
	{
		return tComplMat * entity.GetComponent<shade::TransformComponent>().GetModelMatrix();
	}
	else
	{
		return tComplMat;
	}
}

void shade::Scene::Serialize(std::ostream& stream) const
{
	SHADE_CORE_INFO("******************************************************************");
	SHADE_CORE_INFO("Start to serialize scene ->: {}", m_Name);
	SHADE_CORE_INFO("******************************************************************");
	SHADE_CORE_INFO("Entities count : {}", EntitiesCount());
	serialize::Serializer::Serialize(stream, m_Name);
	serialize::Serializer::Serialize(stream, static_cast<std::uint32_t>(EntitiesCount()));
	// See Components.h
	for (const auto& entity : *this)
	{
		SerrializeEntity(stream, ecs::Entity(entity, this));
	}
}

void shade::Scene::Deserialize(std::istream& stream)
{
	serialize::Serializer::Deserialize(stream, m_Name);
	SHADE_CORE_INFO("******************************************************************");
	SHADE_CORE_INFO("Start to deserialize scene ->: {}", m_Name);
	SHADE_CORE_INFO("******************************************************************");

	std::uint32_t entitiesCount = 0u; serialize::Serializer::Deserialize(stream, entitiesCount);

	SHADE_CORE_INFO("Entities count : {}", entitiesCount);
	for (std::uint32_t entIndex = 0u; entIndex < entitiesCount; entIndex++)
	{
		ecs::Entity entity = CreateEntity();
		DeserrializeEntity(stream, entity, entIndex);
	}

}

void shade::Scene::SerrializeEntity(std::ostream& stream, ecs::Entity entity, bool isParentCall) const
{
	//------------------------------------------------------------------------
	// Check Parent Condition
	//------------------------------------------------------------------------
	// Check if the entity has a parent and this is not a recursive call for a parent entity.
	// If so, skip serialization as we only want to serialize top-level entities first.
	if (entity.HasParent() && !isParentCall) return;

	// Logging the start of entity serialization.
	SHADE_CORE_INFO("   Serializing entity id: {}", entity.GetID());
	SHADE_CORE_INFO("------------------------------------------------------------------");

	//------------------------------------------------------------------------
	// Entity Metadata Serialization
	//------------------------------------------------------------------------
	// Serialization of entity metadata
	std::uint32_t componentsCount = 0u;                // Initialize the components count to zero
	std::uint32_t childrenCount = entity.GetChildrensCount();  // Get the count of child entities
	std::size_t componentsCountPosition = stream.tellp();      // Store the position to overwrite component count later

	// Write the initial counts of components and children to the stream
	serialize::Serializer::Serialize(stream, componentsCount);  // Placeholder count for components (to be updated later)
	serialize::Serializer::Serialize(stream, childrenCount);    // Write the number of children
	SHADE_CORE_INFO("       Childs count: {}", childrenCount);

	//------------------------------------------------------------------------
	// Components Serialization
	//------------------------------------------------------------------------
	// Serialize each component using a lambda function to handle the specific serialization logic

	// Serialize TagComponent if present
	if (bool is = entity.SerializeComponent<TagComponent>(stream, [](std::ostream& stream, const TagComponent& tag)
		{
			return serialize::Serializer::Serialize(stream, tag);
		}))
	{
		componentsCount++;  // Increment components count if serialization was successful
	}

	// Serialize CameraComponent if present
	if (bool is = entity.SerializeComponent<CameraComponent>(stream, [](std::ostream& stream, const CameraComponent& camera)
		{
			serialize::Serializer::Serialize(stream, camera);
		}))
	{
		componentsCount++;
	}

	// Serialize TransformComponent if present
	if (bool is = entity.SerializeComponent<TransformComponent>(stream, [](std::ostream& stream, const TransformComponent& transform)
		{
			serialize::Serializer::Serialize(stream, transform);
		}))
	{
		componentsCount++;
	}

	// Serialize ModelComponent if present, using the model's ID if available
	if (bool is = entity.SerializeComponent<ModelComponent>(stream, [](std::ostream& stream, const ModelComponent& model)
		{
			serialize::Serializer::Serialize(stream, (model && model->GetAssetData()) ? model->GetAssetData()->GetId() : "");
		}))
	{
		componentsCount++;
	}

	// Serialize GlobalLightComponent if present
	if (bool is = entity.SerializeComponent<OmnidirectionalLightComponent>(stream, [](std::ostream& stream, const OmnidirectionalLightComponent& light)
		{
			serialize::Serializer::Serialize(stream, light);
		}))
	{
		componentsCount++;
	}

	// Serialize SpotLightComponent if present
	if (bool is = entity.SerializeComponent<SpotLightComponent>(stream, [](std::ostream& stream, const SpotLightComponent& light)
		{
			serialize::Serializer::Serialize(stream, light);
		}))
	{
		componentsCount++;
	}

	// Serialize PointLightComponent if present
	if (bool is = entity.SerializeComponent<OmnidirectionalLightComponent>(stream, [](std::ostream& stream, const OmnidirectionalLightComponent& light)
		{
			serialize::Serializer::Serialize(stream, light);
		}))
	{
		componentsCount++;
	}

	// Serialize NativeScriptComponent if present
	if (bool is = entity.SerializeComponent<NativeScriptComponent>(stream, [](std::ostream& stream, const NativeScriptComponent& script)
		{
			serialize::Serializer::Serialize(stream, script);
		}))
	{
		componentsCount++;
	}

	// Serialize AnimationGraphComponent if present, using the graph's ID if available
	if (bool is = entity.SerializeComponent<AnimationGraphComponent>(stream, [](std::ostream& stream, const AnimationGraphComponent& graph)
		{
			serialize::Serializer::Serialize(stream, (graph.AnimationGraph && graph.AnimationGraph->GetAssetData()) ? graph.AnimationGraph->GetAssetData()->GetId() : "");
		}))
	{
		componentsCount++;
	}

	//------------------------------------------------------------------------
	// Update Component Count
	//------------------------------------------------------------------------
	// Update component count in the stream
	// Move back to the position where the component count was initially written
	std::size_t endPosition = stream.tellp();         // Save the current stream position
	stream.seekp(componentsCountPosition);            // Go back to where the component count was written
	serialize::Serializer::Serialize(stream, componentsCount);   // Update the component count with the actual value
	stream.seekp(endPosition);                        // Return to the end of the stream for further writing

	SHADE_CORE_INFO("------------------------------------------------------------------");

	//------------------------------------------------------------------------
	// Child Entities Serialization
	//------------------------------------------------------------------------
	// Recursively serialize all child entities of the current entity
	for (const ecs::Entity& child : entity)
		SerrializeEntity(stream, child, true);  // Recursively call with `isParentCall = true` to handle children correctly
}

void shade::Scene::DeserrializeEntity(std::istream& stream, ecs::Entity entity, std::uint32_t& index)
{
	//------------------------------------------------------------------------
	// Deserialize Entity Components and Children
	//------------------------------------------------------------------------

	SHADE_CORE_INFO("   Deserializing entity id: {}", entity.GetID());
	SHADE_CORE_INFO("------------------------------------------------------------------");

	// Read the number of components and children associated with the entity
	std::uint32_t componentsCount = 0u, childrenCount = 0u;
	serialize::Serializer::Deserialize(stream, componentsCount); serialize::Serializer::Deserialize(stream, childrenCount);
	SHADE_CORE_INFO("       Childs count: {}", childrenCount);

	//------------------------------------------------------------------------
	// Deserialize Components
	//------------------------------------------------------------------------

	for (std::uint32_t compIndex = 0u; compIndex < componentsCount; compIndex++)
	{
		std::uint32_t cSize = false; std::uint32_t compTypeHash; serialize::Serializer::Deserialize(stream, compTypeHash);

		// Deserialize TagComponent
		cSize += entity.DeserializeComponent<TagComponent>(stream, compTypeHash, [](std::istream& stream, TagComponent& tag)
			{
				serialize::Serializer::Deserialize(stream, tag);
			});

		// Deserialize CameraComponent
		cSize += entity.DeserializeComponent<CameraComponent>(stream, compTypeHash, [](std::istream& stream, CameraComponent& camera)
			{
				camera = CameraComponent::Create();
				serialize::Serializer::Deserialize(stream, camera);
			});

		// Deserialize TransformComponent
		cSize += entity.DeserializeComponent<TransformComponent>(stream, compTypeHash, [](std::istream& stream, TransformComponent& transform)
			{
				serialize::Serializer::Deserialize(stream, transform);
			});

		// Deserialize ModelComponent
		cSize += entity.DeserializeComponent<ModelComponent>(stream, compTypeHash, [](std::istream& stream, ModelComponent& model)
			{
				std::string assetId;
				serialize::Serializer::Deserialize(stream, assetId);
				AssetManager::GetAsset<Model>(assetId, AssetMeta::Category::Primary, BaseAsset::LifeTime::KeepAlive, [&](auto& asset) mutable { model = asset; });
			});

		// Deserialize GlobalLightComponent
		cSize += entity.DeserializeComponent<DirectionalLightComponent>(stream, compTypeHash, [](std::istream& stream, DirectionalLightComponent& light)
			{
				light = DirectionalLightComponent::Create();
				serialize::Serializer::Deserialize(stream, light);
			});

		// Deserialize SpotLightComponent
		cSize += entity.DeserializeComponent<SpotLightComponent>(stream, compTypeHash, [](std::istream& stream, SpotLightComponent& light)
			{
				light = SpotLightComponent::Create();
				serialize::Serializer::Deserialize(stream, light);
			});

		// Deserialize PointLightComponent
		cSize += entity.DeserializeComponent<OmnidirectionalLightComponent>(stream, compTypeHash, [](std::istream& stream, OmnidirectionalLightComponent& light)
			{
				light = OmnidirectionalLightComponent::Create();
				serialize::Serializer::Deserialize(stream, light);
			});

		// Deserialize NativeScriptComponent
		cSize += entity.DeserializeComponent<NativeScriptComponent>(stream, compTypeHash, [](std::istream& stream, NativeScriptComponent& script)
			{
				serialize::Serializer::Deserialize(stream, script);
			});

		// Deserialize AnimationGraphComponent
		cSize += entity.DeserializeComponent<AnimationGraphComponent>(stream, compTypeHash, [](std::istream& stream, AnimationGraphComponent& graph)
			{
				std::string assetId; serialize::Serializer::Deserialize(stream, assetId);
				graph.GraphContext.Controller = shade::animation::AnimationController::Create();
				AssetManager::GetAsset<animation::AnimationGraph, BaseAsset::InstantiationBehaviour::Aynchronous>(assetId, AssetMeta::Category::Secondary, BaseAsset::LifeTime::KeepAlive, 
					[&](auto& asset) mutable { 
						graph.AnimationGraph = asset;
					}, &graph.GraphContext);
			});

		if (!cSize)
		{
			// Should be throw !
			SHADE_CORE_ERROR("Component was not recognized during the deserialization !");
			return;
		}
	}

	SHADE_CORE_INFO("------------------------------------------------------------------");

	//------------------------------------------------------------------------
	// Deserialize Children Entities
	//------------------------------------------------------------------------

	for (std::uint32_t childIndex = 0u; childIndex < childrenCount; childIndex++, index++)
	{
		ecs::Entity child = CreateEntity();			// Create a new child entity
		DeserrializeEntity(stream, child, index);	// Deserialize the child entity recursively
		entity.AddChild(child);						// Add the child entity to the current entity's children list
	}
}
