#pragma once
#include <shade/core/layer/Layer.h>
#include <shade/core/layer/imgui/ImGuiLayer.h>
#include <shade/core/system/FileDialog.h>
#include <import/IModel.h>
//#include <animation graphs/EditorAnimGraph.h>
#include <animation graphs/EditorAnimGraph.h>

class EditorLayer : public shade::ImGuiLayer
{
public:
	class EditorCamera : public shade::Camera
	{
	public:
		EditorCamera();
		void OnUpdate(const shade::FrameTimer& deltaTime);
		void SetUpdate(bool set) { m_IsUpdate = set; }
	private:
		float m_RotationSpeed = 0.3f;
		float m_MovementSpeed = 0.005f;
		bool m_IsUpdate = false;
	}; 
public:
	EditorLayer();
	virtual ~EditorLayer() = default;

	virtual void OnCreate() override;
	virtual void OnUpdate(shade::SharedPointer<shade::Scene>& scene, const shade::FrameTimer& deltaTime) override;
	virtual void OnRender(shade::SharedPointer<shade::Scene>& scene, const shade::FrameTimer& deltaTime) override;
	virtual void OnEvent(shade::SharedPointer<shade::Scene>& scene, const shade::Event& event, const shade::FrameTimer& deltaTime) override;
	virtual void OnDestroy() override;
private:
	std::uint32_t				m_ImGuizmoOperation			= ImGuizmo::OPERATION::TRANSLATE;
	std::uint32_t				m_ImGuizmoAllowedOperation	= 0;
	shade::ecs::Entity			m_SelectedEntity;
	shade::MaterialComponent	m_SelectedMaterial;
	shade::MeshComponent		m_SelectedMesh;
	shade::AnimationGraphComponent m_SelectedAnimationGraphComponent;

	shade::SharedPointer<shade::Model> m_ImportedModel;
	std::unordered_map<std::string, shade::SharedPointer<shade::Animation>> m_ImportedAnimations;
	shade::ecs::Entity m_ImportedEntity;
	shade::SharedPointer<shade::SceneRenderer>  m_SceneRenderer;

	shade::animation::AnimationGraphContext	graphContext;
	shade::SharedPointer<shade::animation::AnimationGraph> m_Graph;
	graph_editor::GraphEditor m_GraphEditor;

	shade::SharedPointer<EditorCamera> m_EditorCamera;

	/*shade::SharedPointer<editor_animation_graph::GraphDeligate>			m_AnimationGraphEditor;
	shade::SharedPointer<editor_state_machine::StateMachineDeligate>	m_StateMachineEditor;*/

	bool m_IsAddNewAssetModalOpen = false;
	bool m_IsAddNewAttributeModalOpen = false;
	bool m_IsCreateNewRawAssetModalOpen = false;
	bool m_IsAddCollisionShapeModal = false;
	bool m_ImportModelModal = false;
	bool m_PackFilesModal = false;
	bool m_IsAddSkeletalAnimationModal = false;
	bool m_IsSceneFullScreen = false;
	// TEST !
	bool m_IsScenePlaying = false;

	void MainMenu(shade::SharedPointer<shade::Scene>& scene);
	void Scene(shade::SharedPointer<shade::Scene>& scene, const shade::FrameTimer& deltaTime);
	bool EntitiesList(const char* search, shade::SharedPointer<shade::Scene>& scene);
	void EntitiesList(const char* search, shade::ecs::Entity& entity);
	void Entities(shade::SharedPointer<shade::Scene>& scene);
	void AssetsExplorer();
	void Creator();
	void EditAsset(shade::SharedPointer<shade::AssetData>& assetData);
	void RenderSettings(shade::SharedPointer<shade::SceneRenderer>& renderer, const shade::FrameTimer& deltaTime);
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
	void NativeScriptComponent(shade::ecs::Entity& entity);
	///////
	void MaterialEdit(shade::Material& material);
	void BoneMaskEdnitor(const shade::Skeleton::BoneNode* node, shade::animation::BoneMask& boneMask);
	void CreateMaterial();
	void Material(shade::Material& material);
	void AnimationSequencer();
	//////
	void CreateCollisionShapes();

	template<typename Callback>
	void DrawRenderMenuItem(const std::string& title, Callback callback)
	{
		//ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { ImGui::GetStyle().FramePadding.x , ImGui::GetStyle().FramePadding.y * 2.0f });
		bool isOpen = ImGui::TreeNodeEx(title.c_str(), ImGuiTreeNodeFlags_Framed);
		ImGui::PopStyleVar(1);
		if (isOpen) { std::invoke(callback); ImGui::TreePop();} 
	}
};