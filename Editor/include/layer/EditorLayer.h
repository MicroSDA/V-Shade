#pragma once
#include <shade/core/layer/Layer.h>
#include <shade/core/layer/imgui/ImGuiLayer.h>
#include <shade/core/system/FileDialog.h>
#include <import/IModel.h>
#include <animation graphs/EditorAnimGraph.h>

class EditorLayer : public shade::ImGuiLayer
{
public:
	EditorLayer();
	virtual ~EditorLayer() = default;

	virtual void OnCreate() override;
	virtual void OnUpdate(shade::SharedPointer<shade::Scene>& scene, const shade::FrameTimer& deltaTime) override;
	virtual void OnRender(shade::SharedPointer<shade::Scene>& scene, const shade::FrameTimer& deltaTime) override;
	virtual void OnEvent(shade::SharedPointer<shade::Scene>& scene, const shade::Event& event, const shade::FrameTimer& deltaTime) override;
	virtual void OnDestroy() override;
private:
	std::uint32_t				m_ImGuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	shade::ecs::Entity			m_SelectedEntity;
	shade::MaterialComponent	m_SelectedMaterial;
	shade::MeshComponent		m_SelectedMesh;

	shade::SharedPointer<shade::Model> m_ImportedModel;
	shade::ecs::Entity m_ImportedEntity;
	shade::SharedPointer<shade::SceneRenderer>  m_SceneRenderer;


	editor_anim_grap_nodes::Graph m_AnimationGraphEditor;

	bool m_IsAddNewAssetModalOpen = false;
	bool m_IsAddNewAttributeModalOpen = false;
	bool m_IsCreateNewRawAssetModalOpen = false;
	bool m_IsAddCollisionShapeModal = false;
	bool m_ImportModelModal = false;
	bool m_PackFilesModal = false;
	bool m_IsAddSkeletalAnimationModal = false;

	// TEST !
	bool m_IsScenePlaying = false;

	void MainMenu(shade::SharedPointer<shade::Scene>& scene);
	void Scene(shade::SharedPointer<shade::Scene>& scene);
	void EntitiesList(const char* search, shade::SharedPointer<shade::Scene>& scene);
	void EntitiesList(const char* search, shade::ecs::Entity& entity);
	void Entities(shade::SharedPointer<shade::Scene>& scene);
	void AssetsExplorer();
	void Creator();
	void EditAsset(shade::SharedPointer<shade::AssetData>& assetData);
	void RenderSettings(shade::SharedPointer<shade::SceneRenderer>& renderer);
	void EntityInspector(shade::ecs::Entity& entity);
	void AddNewAsset();
	/////////
	void TagComponent(shade::ecs::Entity& entity);
	void TransformComponent(shade::ecs::Entity& entity);
	void GlobalLightComponent(shade::ecs::Entity& entity);
	void PointLightComponent(shade::ecs::Entity& entity);
	void SpotLightComponent(shade::ecs::Entity& entity);
	void CameraComponent(shade::ecs::Entity& entity);
	void ModelComponent(shade::ecs::Entity& entity);
	void RgidBodyComponent(shade::ecs::Entity& entity);
	void AnimationGraphComponent(shade::ecs::Entity& entity);
	///////
	void MaterialEdit(shade::Material& material);
	void BoneMaskEdnitor(const shade::Skeleton::BoneNode* node, shade::animation::BoneMask& boneMask);
	void CreateMaterial();
	void Material(shade::Material& material);
	void AnimationSequencer();
	//////
	void CreateCollisionShapes();
};