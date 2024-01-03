#include "shade_pch.h"
#include "Scene.h"
#include <shade/core/asset/AssetManager.h>
#include <shade/core/scripting/ScriptManager.h>
// TODO: TEMPORARY
#include <shade/core/camera/Camera.h>

std::unordered_map<std::string, shade::SharedPointer<shade::Scene>> shade::Scene::m_sScenes;
shade::SharedPointer<shade::Scene> shade::Scene::m_sActiveScene;

#ifndef SERIALIZE_COMPONENT
    #define SERIALIZE_COMPONENT(stream, count, Type, entity) \
        if (HasComponent<Type>(entity)) { \
           SHADE_CORE_INFO("       Serrializing: {}", typeid(Type).name());\
            std::uint32_t hash = static_cast<std::uint32_t>(std::hash<std::string>()(typeid(Type).name()));\
            shade::Serializer::Serialize(stream, hash); \
            shade::SceneComponentSerializer::Serialize(stream, GetComponent<Type>(entity)); \
            count++;\
        }
#endif // SERIALIZE_COMPONENT

// TODO: Change to return nullptr and check if not null then dessirialize, or keep like this
#ifndef DESERIALIZE_COMPONENT
#define DESERIALIZE_COMPONENT(cHhash, stream, Type, entity, callback) \
   if (cHhash == static_cast<std::uint32_t>(std::hash<std::string>()(typeid(Type).name()))) { \
        Type& component = callback(entity); \
        if(&component != nullptr) {\
           shade::SceneComponentSerializer::Deserialize(stream, component); \
        }\
   }\

#endif // DESERIALIZE_COMPONENT

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


shade::Scene::~Scene()
{
   
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
            if (!script.m_pInstance)
            {
                script.m_pInstance = script.InstantiateScript();
                script.m_pInstance->m_Entity = entity;
                script.m_pInstance->OnCreate();
            }
            else if (script.m_pInstance->IsUpdate())
                script.m_pInstance->OnUpdate(deltaTime);
        });
}

void shade::Scene::AnimationsUpdate(const shade::FrameTimer& deltaTime)
{
   View<AnimationGraphComponent>().Each([&](ecs::Entity& entity, AnimationGraphComponent& graph)
        {
            if (graph)
            {
                graph->Evaluate(deltaTime);
            }
        });
}

shade::ecs::Entity shade::Scene::GetPrimaryCamera()
{
   ecs::Entity _entity;

    View<CameraComponent>().Each([&_entity](ecs::Entity& entity, CameraComponent& camera)
        {
            if(camera->IsPrimary())
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
    AnimationsUpdate(deltaTime);
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

glm::mat4 shade::Scene::ComputePCTransform(ecs::Entity& entity)
{
    glm::mat4 transform(1.0f);

    if (entity.HasParent())
    {
        ecs::Entity parent = entity.GetParent();
        transform = ComputePCTransform(parent);
    }

    if (entity.HasComponent<shade::TransformComponent>())
    {
        // Not user do we need to use * with transform ?
        return transform * entity.GetComponent<shade::TransformComponent>().GetModelMatrix();
    }
    else
        return transform;
}

std::size_t shade::Scene::Serialize(std::ostream& stream) const
{
    SHADE_CORE_INFO("******************************************************************");
    SHADE_CORE_INFO("Start to serrialize scene ->: {}", "PATH");
    SHADE_CORE_INFO("******************************************************************");
    SHADE_CORE_INFO("Entities count : {}", EntitiesCount());
    Serializer::Serialize(stream, static_cast<std::uint32_t>(EntitiesCount()));
    // See Components.h
    for (const auto& entity : *this)
    {
        SerrializeEntity(stream, ecs::Entity(entity, this));
    }
    return std::size_t();
}

std::size_t shade::Scene::Deserialize(std::istream& stream)
{ 
    std::uint32_t entitiesCount = 0u;
    std::size_t size = Serializer::Deserialize(stream, entitiesCount);

    for (std::uint32_t entIndex = 0u; entIndex < entitiesCount; entIndex++)
    {
        ecs::Entity entity = CreateEntity();
        DeserrializeEntity(stream, entity, entIndex);
    }

    return std::size_t();
}

void shade::Scene::SerrializeEntity(std::ostream& stream, ecs::Entity entity, bool isParentCall) const
{
    if (entity.HasParent() && !isParentCall)
        return;

    SHADE_CORE_INFO("   Serrializing entity id: {}", entity.GetID());
    SHADE_CORE_INFO("------------------------------------------------------------------");

    std::uint32_t componentsCount = 0u, childrenCount = entity.GetChildrensCount();
    std::size_t  componentsCountPosition = stream.tellp();
    // Write how many components has to be serrialized ;
    Serializer::Serialize(stream, componentsCount);
    // Write how many childrens has to be serrialized ;
    Serializer::Serialize(stream, childrenCount);
    SHADE_CORE_INFO("       Childs count: {}", childrenCount);

    SERIALIZE_COMPONENT(stream, componentsCount, TagComponent, entity);
    SERIALIZE_COMPONENT(stream, componentsCount, CameraComponent, entity);
    SERIALIZE_COMPONENT(stream, componentsCount, TransformComponent, entity);
    SERIALIZE_COMPONENT(stream, componentsCount, RigidBodyComponent, entity);
    SERIALIZE_COMPONENT(stream, componentsCount, ModelComponent, entity);
    SERIALIZE_COMPONENT(stream, componentsCount, GlobalLightComponent, entity);
    SERIALIZE_COMPONENT(stream, componentsCount, SpotLightComponent, entity);
    SERIALIZE_COMPONENT(stream, componentsCount, PointLightComponent, entity);
    SERIALIZE_COMPONENT(stream, componentsCount, NativeScriptComponent, entity);
  
    // Set confirmed comonents count 
    std::size_t endPosition = stream.tellp();
    stream.seekp(componentsCountPosition);
    Serializer::Serialize(stream, componentsCount);
    stream.seekp(endPosition);

    SHADE_CORE_INFO("------------------------------------------------------------------");
    // Serrialize childrens
    for (const ecs::Entity& child : entity)
        SerrializeEntity(stream, child, true); 
}

void shade::Scene::DeserrializeEntity(std::istream& stream, ecs::Entity entity, std::uint32_t& index)
{
    std::uint32_t componentsCount = 0u, childrenCount = 0u;
    Serializer::Deserialize(stream, componentsCount);
    Serializer::Deserialize(stream, childrenCount);

    for (std::uint32_t compIndex = 0u; compIndex < componentsCount; compIndex++)
    {
        std::uint32_t compHeaderHash;
        shade::Serializer::Deserialize(stream, compHeaderHash);

        // TODO: If no one of components was recogninzed we need to stop desirialize scene !!
        DESERIALIZE_COMPONENT(compHeaderHash, stream, TagComponent, entity, [&](ecs::Entity& entity)->TagComponent& { return entity.AddComponent<TagComponent>(); });
        DESERIALIZE_COMPONENT(compHeaderHash, stream, CameraComponent, entity, [&](ecs::Entity& entity)->CameraComponent& { return entity.AddComponent<CameraComponent>(CameraComponent::Create()); });
        DESERIALIZE_COMPONENT(compHeaderHash, stream, TransformComponent, entity, [&](ecs::Entity& entity)->TransformComponent& { return entity.AddComponent<TransformComponent>(); });
        DESERIALIZE_COMPONENT(compHeaderHash, stream, RigidBodyComponent, entity, [&](ecs::Entity& entity)->RigidBodyComponent& { return entity.AddComponent<RigidBodyComponent>(); });

        DESERIALIZE_COMPONENT(compHeaderHash, stream, ModelComponent, entity, [&](ecs::Entity& entity)->ModelComponent& 
            {
                std::string id;  Serializer::Deserialize(stream, id);
                AssetManager::GetAsset<Model>(id, AssetMeta::Category::Primary, BaseAsset::LifeTime::KeepAlive, [=](auto& model) mutable
                    {
                        entity.AddComponent<ModelComponent>(model);
                    });
                // We don't need to call DesirialzieAsComponent function for this component
                return *static_cast<ModelComponent*>(nullptr);
            });

        DESERIALIZE_COMPONENT(compHeaderHash, stream, GlobalLightComponent, entity, [&](ecs::Entity& enityt)->GlobalLightComponent& { return entity.AddComponent<GlobalLightComponent>(GlobalLightComponent::Create()); });
        DESERIALIZE_COMPONENT(compHeaderHash, stream, SpotLightComponent, entity, [&](ecs::Entity& enityt)->SpotLightComponent& { return entity.AddComponent<SpotLightComponent>(SpotLightComponent::Create()); });
        DESERIALIZE_COMPONENT(compHeaderHash, stream, PointLightComponent, entity, [&](ecs::Entity& enityt)->PointLightComponent& { return entity.AddComponent<PointLightComponent>(PointLightComponent::Create()); });
        DESERIALIZE_COMPONENT(compHeaderHash, stream, NativeScriptComponent, entity, [&](ecs::Entity& enityt)->NativeScriptComponent&
            {
                std::string module; std::string name;
                Serializer::Deserialize(stream, module);  Serializer::Deserialize(stream, name);
                ecs::ScriptableEntity* script = shade::ScriptManager::InstantiateScript<shade::ecs::ScriptableEntity*>(module, name);
                if (script)
                    entity.AddComponent<shade::NativeScriptComponent>().Bind(script);
                // We don't need to call DesirialzieAsComponent function for this component
                return *static_cast<NativeScriptComponent*>(nullptr);
            });
    }
    // Deserrialize childrens
    for (std::uint32_t childIndex = 0u; childIndex < childrenCount; childIndex++, index++)
    {
        ecs::Entity child = CreateEntity();
        DeserrializeEntity(stream, child, index);
        entity.AddChild(child);
    }
}
