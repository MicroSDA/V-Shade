#include "shade_pch.h"
#include "EditorLayer.h"
#include <shade/core/event/Input.h>
#include <shade/core/application/Application.h>

// TODO: Temporary

EditorLayer::EditorCamera::EditorCamera()
{
	SetDirection(glm::vec3(-0.011512465, -0.33462766, 0.94228005));
	SetPosition(glm::vec3(0, 10, -20));
}

void EditorLayer::EditorCamera::OnUpdate(const shade::FrameTimer& deltaTime)
{
	if (m_IsUpdate)
	{
		{
			// Movment
			if (shade::Input::IsKeyPressed(shade::Key::W))
				MoveForward(m_MovementSpeed * deltaTime);
			if (shade::Input::IsKeyPressed(shade::Key::S))
				MoveBackward(m_MovementSpeed * deltaTime);

			if (shade::Input::IsKeyPressed(shade::Key::A))
				MoveLeft(m_MovementSpeed * deltaTime);
			if (shade::Input::IsKeyPressed(shade::Key::D))
				MoveRight(m_MovementSpeed * deltaTime);

			if (shade::Input::IsKeyPressed(shade::Key::Q))
				RotateZ(m_RotationSpeed / 100.f * deltaTime);
			if (shade::Input::IsKeyPressed(shade::Key::E))
				RotateZ(-m_RotationSpeed / 100.f * deltaTime);

		}
		{
			if (shade::Input::IsMouseButtonPressed(shade::Mouse::ButtonRight))
			{
				shade::Input::ShowMouseCursor(false);
				glm::vec2 screenCenter(shade::Application::GetWindow()->GetWidth() / 2, shade::Application::GetWindow()->GetHeight() / 2);
				glm::vec2 mousePosition(shade::Input::GetMousePosition() - screenCenter);

				Rotate(glm::vec3(mousePosition, 0.0f) * m_RotationSpeed / 1000.f);

				shade::Input::SetMousePosition(screenCenter.x, screenCenter.y);
			}
			else
			{
				shade::Input::ShowMouseCursor(true);
			}
		}
	}
}

EditorLayer::EditorLayer() : ImGuiLayer()
{
	ImGui::SetCurrentContext(GetImGuiContext());
	shade::ImGuiThemeEditor::SetColors(0x202020FF, 0xFAFFFDFF, 0x505050FF, 0x9C1938CC, 0xFFC307B1);
	shade::ImGuiThemeEditor::SetColors(0x202020FF, 0xFAFFFDFF, 0x505050FF, 0x9C1938CC, 0xFFC307B1);
	shade::ImGuiThemeEditor::ApplyTheme();
}

void EditorLayer::OnCreate()
{
	m_SceneRenderer = shade::SceneRenderer::Create();
	m_EditorCamera = shade::SharedPointer<EditorCamera>::Create();
}

void EditorLayer::OnUpdate(shade::SharedPointer<shade::Scene>& scene, const shade::FrameTimer& deltaTime)
{

	(m_IsScenePlaying) ? scene->SetPlaying(true) : scene->SetPlaying(false);
	if (!m_IsScenePlaying) m_EditorCamera->OnUpdate(deltaTime);

	shade::physic::PhysicsManager::Step(scene, deltaTime);
	// TODO: m_SceneRenderer->OnUpdate using render functions so for logic we need to use on update scene render only when we need to draw
	// When window is minizied we don't need to update m_SceneRenderer->OnUpdate but layer should be updated instead !
	// Для того что бы смочь менимизировать окно нам нужно m_SceneRenderer->OnUpdate до EditorLayer::OnRender но не в EditorLayer::OnUpdate!!!!
	m_SceneRenderer->OnUpdate(scene, (m_IsScenePlaying) ? nullptr : m_EditorCamera, deltaTime);
}

void EditorLayer::OnRender(shade::SharedPointer<shade::Scene>& scene, const shade::FrameTimer& deltaTime)
{

	m_SceneRenderer->OnRender(scene, deltaTime);

	ImGui::SetCurrentContext(GetImGuiContext());
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	if (ImGui::Begin("Shade", nullptr, m_WindowFlags))
	{
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspaceId = ImGui::GetID("Shade");
			ImGui::DockSpace(dockspaceId, ImVec2(0.f, 0.f), m_DockSpaceFlags);
		}


		MainMenu(scene);
		ShowWindowBar("Entities", NULL, &EditorLayer::Entities, this, scene);
		ShowWindowBar("Creator", NULL, &EditorLayer::Creator, this);
		ShowWindowBar("Assets", NULL, &EditorLayer::AssetsExplorer, this);
		ShowWindowBar("Inspector", NULL, &EditorLayer::EntityInspector, this, m_SelectedEntity);
		ShowWindowBar("Material", NULL, &EditorLayer::MaterialEdit, this, (m_SelectedMaterial != nullptr) ? *m_SelectedMaterial : *shade::Renderer::GetDefaultMaterial());
		ShowWindowBar("Render settings", NULL, &EditorLayer::RenderSettings, this, m_SceneRenderer);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1));
		ShowWindowBar("Scene", NULL, &EditorLayer::Scene, this, scene);
		ImGui::PopStyleColor();
		//ImGui::ShowDemoWindow();
		ImGui::End();
		/*	enum States : std::uint8_t
			{
				Idle,
				Walk
			};
			shade::PlayerStateMachineComponent stateMachine = shade::PlayerStateMachineComponent::Create();
			stateMachine->AddState<Idle, shade::state_machine::PlayerStateMachine>("Idle");


			stateMachine->GetState(Idle, Idle)->Evaluate(deltaTime);*/

	}
}

void EditorLayer::OnEvent(shade::SharedPointer<shade::Scene>& scene, const shade::Event& event, const shade::FrameTimer& deltaTime)
{
	m_SceneRenderer->OnEvent(scene, event, deltaTime);

	if (shade::Input::IsKeyPressed(shade::Key::R))
		m_ImGuizmoOperation = ImGuizmo::OPERATION::ROTATE;
	if (shade::Input::IsKeyPressed(shade::Key::T))
		m_ImGuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
	if (shade::Input::IsKeyPressed(shade::Key::Y))
		m_ImGuizmoOperation = ImGuizmo::OPERATION::SCALE;
}

void EditorLayer::OnDestroy()
{
}

void EditorLayer::MainMenu(shade::SharedPointer<shade::Scene>& scene)
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("Scene"))
			{
				if (ImGui::MenuItem("Open scene"))
				{
					auto path = shade::FileDialog::OpenFile("Shade scene(*.scene) \0*.scene\0");
					if (!path.empty())
					{
						shade::File file(path.string(), shade::File::In, "@s_scene", shade::File::VERSION(0, 0, 1));
						if (file.IsOpen())
						{
							scene->DestroyAllEntites();
							file.Read(scene);
						}
						else
						{
							SHADE_CORE_WARNING("Couldn't open scene file, path ={0}", path);
						}
					}
				}

				if (ImGui::MenuItem("Save scene"))
				{
					auto path = shade::FileDialog::SaveFile("Shade scene(*.scene) \0*.scene\0");
					if (!path.empty())
					{
						shade::File file(path.string(), shade::File::Out, "@s_scene", shade::File::VERSION(0, 0, 1));
						if (file.IsOpen())
						{
							file.Write(scene);
						}
						else
						{
							SHADE_CORE_WARNING("Couldn't open scene file, path ={0}", path);
						}
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Import"))
			{
				if (ImGui::MenuItem("Model"))
				{
					m_ImportModelModal = true;
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Pack files"))
			{
				m_PackFilesModal = true;
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGui::SetNextWindowSize(ImGui::GetContentRegionMax() / 2.f);
	DrawModal("Import model", m_ImportModelModal, [&]()
		{
			static std::string from;
			static std::string to;

			static bool importModels = true,
				importMeshes = true,
				importSkeleton = true,
				bakeBones = true,
				importAnimations = true,
				importMaterials = true,
				validateAnimationChannels = false,
				triangulate = true,
				flipUvs = true,
				joinVerties = true,
				calcNormals = true,
				calcTangents = true,
				genSmoothNormals = true,
				useScale = false;

			//if (!m_ImportedModel)
			{

				if (ImGui::BeginTable("ImportModelTable", 2, ImGuiTableFlags_SizingStretchProp))
				{
					ImGui::TableSetupColumn("Import settings");
					ImGui::TableSetupColumn("Post process");
					ImGui::TableHeadersRow();

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					{

						if (ImGui::BeginTable("ImportSettingsTable", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg, { 0, 0 }))
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("Import Model"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##ImprotModelsCheckBox", &importModels);  HelpMarker("(?)", "TODO"); }

							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("Import Meshes"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##ImprotMeshesCheckBox", &importMeshes); HelpMarker("(?)", "Import and convert meshes into valid engine file format."); }

							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("Import Materials"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##ImprotMaterialsCheckBox", &importMaterials); HelpMarker("(?)", "Import and convert materials into valid engine file format."); }

							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("Try to import Skeleton"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##ImprotSkeletonCheckBox", &importSkeleton); HelpMarker("(?)", "Try to find and convert skeleton into valid engine file format if skeleton is present."); }

							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("Try to import Animations"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##ImprotAnimationsCheckBox", &importAnimations); HelpMarker("(?)", "Try to find and convert animations into valid engine file format if animations are present."); }

							ImGui::TableNextRow();
							(!importSkeleton || !importAnimations) ? ImGui::BeginDisabled() : void();
							ImGui::TableNextColumn(); { ImGui::Text("	Validate animation channels"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##ValidateAnimationsChannels", &validateAnimationChannels); HelpMarker("(?)", "Remove animation channels if there are no specific bones present."); }
							(!importSkeleton || !importAnimations) ? ImGui::EndDisabled() : void();

							ImGui::EndTable();
						}
					}
					ImGui::TableNextColumn();
					{
						if (ImGui::BeginTable("PostProcessTable", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg, { 0, 0 }))
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("Global scale"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##GlobalScale", &useScale); HelpMarker("(?)", "This step will perform a global scale of the model."); }

							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("Triangulate"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##TriangulateCheckBox", &triangulate); HelpMarker("(?)", "Triangulates all faces of all meshes."); }

							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("FlipUVs"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##FlipUVsCheckBox", &flipUvs); HelpMarker("(?)", "Flips all UV coordinates along the y-axis and adjusts material settings and bitangents accordingly."); }

							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("Join identical vertices"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##JoinVertiesCheckBox", &joinVerties);  HelpMarker("(?)", "Identifies and joins identical vertex data sets within all imported meshes."); }

							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("Calculate normals"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##CalculateNormalsCheckBox", &calcNormals); }

							(!calcNormals) ? ImGui::BeginDisabled() : void();
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("	Generate smooth normals"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##GenerateSmoothNormalsCheckBox", &genSmoothNormals); }
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("	Calculate tangents"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##CalculateTangentsCheckBox", &calcTangents); }
							(!calcNormals) ? ImGui::EndDisabled() : void();

							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("Bake bones into mesh data"); }
							ImGui::TableNextColumn(); { ImGui::Checkbox("##BakeNormalsCheckBox", &bakeBones); }

							ImGui::EndTable();
						}
					}

					ImGui::EndTable();
				}

				//ImGui::SameLine();
				//ImGui::Text("Post process:"); ImGui::Separator();



				if (ImGui::BeginTable("SelectFrom", 2, ImGuiTableFlags_SizingStretchProp, { ImGui::GetContentRegionAvail().x, 0 }))
				{
					ImGui::TableNextColumn();
					{
						InputTextCol("Select from", from);
					}
					ImGui::TableNextColumn();
					{
						if (ImGui::Button("...##From"))
						{
							auto selectedPath = shade::FileDialog::OpenFile("Supported formats(*.obj, *.fbx, *.dae) \0*.obj;*.fbx;*.dae\0");
							if (!selectedPath.empty())
							{
								IImportFlag importFlags =
									((importModels) ? IImportFlags::ImportModel : 0) |
									((importMeshes) ? IImportFlags::ImportMeshes : 0) |
									((importMaterials) ? IImportFlags::ImportMaterials : 0) |
									((importAnimations) ? IImportFlags::ImportAnimation : 0) |
									((importSkeleton) ? IImportFlags::TryToImportSkeleton : 0) |
									((validateAnimationChannels) ? IImportFlags::TryValidateAnimationChannels : 0) |

									((triangulate) ? IImportFlags::Triangulate : 0) |
									((bakeBones) ? IImportFlags::BakeBoneIdsWeightsIntoMesh : 0) |
									((flipUvs) ? IImportFlags::FlipUVs : 0) |
									((joinVerties) ? IImportFlags::JoinIdenticalVertices : 0) |
									((calcNormals) ? IImportFlags::CalcNormals : 0) |
									((genSmoothNormals) ? IImportFlags::GenSmoothNormals : 0) |
									((calcTangents) ? IImportFlags::CalcTangentSpace : 0) |
									((useScale) ? IImportFlags::UseScale : 0);

								auto [model, animation] = IModel::Import(selectedPath.string(), importFlags);

								m_ImportedModel = model;
								m_ImportedAnimations = animation;

								if (m_ImportedModel)
								{
									// Temorary and actually wrong bcs we need to create imported model entity as part of editor layer!

									m_ImportedEntity = scene->CreateEntity();
									// TODO: MAKE PROPARE NAME 
									m_ImportedEntity.AddComponent<shade::TagComponent>("Improted model 1");
									m_ImportedEntity.AddComponent<shade::TransformComponent>();
									m_ImportedEntity.AddComponent<shade::ModelComponent>(m_ImportedModel);

									/*if (animation)
									{
										m_ImportedEntity.AddComponent<shade::AnimationGraphComponent>(animation);
									}*/
								}

								from = selectedPath.string();
							}

						}
					}
					ImGui::EndTable();
				}
			}

			if (m_ImportedModel && m_ImportedEntity.IsValid())
			{

				ModelComponent(m_ImportedEntity);

				//for (auto& mesh : *m_ImportedModel)
				//{
				//	const int maxFaces = mesh->GetLod(0).Indices.size() / 3;
				//	static int faces = maxFaces;
				//	static float lambda = 0.1;

				//	if (ImGui::TreeNodeEx(std::format("Mesh: {}", mesh->GetAssetData()->GetId()).c_str(), ImGuiTreeNodeFlags_Framed))
				//	{
				//		ImGui::Text("Set globally :"); ImGui::Separator();
				//		if (ImGui::BeginTable("_title.c_str()", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp, { 0, 0 }))
				//		{
				//			ImGui::TableNextColumn();
				//			{
				//				if (DragInt("Faces", &faces, 1, 1, maxFaces))
				//					mesh->RecalculateAllLods(shade::Drawable::MAX_LEVEL_OF_DETAIL, maxFaces, faces, lambda);
				//			}
				//			ImGui::TableNextColumn();
				//			{
				//				if (DragFloat("Split power", &lambda, 0.01, 0.01, 1.0))
				//					mesh->RecalculateAllLods(shade::Drawable::MAX_LEVEL_OF_DETAIL, maxFaces, faces, lambda);
				//			}
				//			ImGui::EndTable();
				//		}

				//		ImGui::Text("Set manually :"); ImGui::Separator();
				//		for (int i = 1; i < mesh->GetLods().size(); i++)
				//		{
				//			int faces = mesh->GetLod(i).Indices.size() / 3;

				//			if (DragInt(std::format("Lod level #:{}, faces :", i).c_str(), &faces, 1, 1, maxFaces))
				//			{
				//				mesh->RecalculateLod(i, faces);
				//			}
				//		}

				//		/*if (ImGui::Button("Save"))
				//		{
				//			auto path = mesh->GetAssetData()->GetReference()->GetAttribute<std::string>("Path");

				//			if (!path.empty())
				//			{
				//				std::ofstream file(path, std::ios::binary);
				//				shade::Serializer::Serialize(file, mesh);
				//				file.close();
				//			}
				//			else
				//			{
				//				SHADE_CORE_WARNING("Couldn't save mesh, asset path is empty!");
				//			}

				//		}*/
				//		ImGui::TreePop();
				//	}
				//}

				if (ImGui::BeginTable("Save to", 2, ImGuiTableFlags_SizingStretchProp, { ImGui::GetContentRegionAvail().x, 0 }))
				{
					ImGui::TableNextColumn();
					{
						InputTextCol("Save into", to);
					}
					ImGui::TableNextColumn();
					{
						if (ImGui::Button("...##To"))
						{
							auto selectedPath = shade::FileDialog::SelectFolder("");
							if (!selectedPath.empty())
								to = selectedPath.string();
						}
					}

					ImGui::EndTable();
				}

				if (!to.empty())
				{
					if (ImGui::Button("Save", { ImGui::GetContentRegionAvail().x, 0.f }))
					{
						if (m_ImportedModel->GetSkeleton())
						{
							const std::string path(to + m_ImportedModel->GetSkeleton()->GetAssetData()->GetId() + ".s_skel");
							shade::File file(path, shade::File::Out, "@s_skel", shade::File::VERSION(0, 0, 1));
							if (file.IsOpen())
							{
								file.Write(m_ImportedModel->GetSkeleton());
							}
							else
							{
								SHADE_CORE_WARNING("Failed to save skeleton, path = {}", path);
							}
						}

						for (const auto& mesh : *m_ImportedModel)
						{
							const std::string path(to + mesh->GetAssetData()->GetId() + ".s_mesh");
							shade::File file(path, shade::File::Out, "@s_mesh", shade::File::VERSION(0, 0, 1));
							if (file.IsOpen())
							{
								file.Write(mesh);
							}
							else
							{
								SHADE_CORE_WARNING("Failed to save mesh, path = {}", path);
							}
						}

						for (const auto& [name, animation] : m_ImportedAnimations)
						{
							const std::string path(to + animation->GetAssetData()->GetId() + ".s_anim");

							shade::File file(path, shade::File::Out, "@s_anim", shade::File::VERSION(0, 0, 1));

							if (file.IsOpen())
								file.Write(animation);
							else
								SHADE_CORE_WARNING("Failed to save animation, path = {}", path);
						}
						//if (m_ImportedEntity.HasComponent<shade::AnimationGraphComponent>())
						//{
						//	/*for (const auto& [name, animation] : *m_ImportedEntity.GetComponent<shade::AnimationControllerComponent>())
						//	{
						//		const std::string path(to + animation.Animation->GetAssetData()->GetId() + ".s_sanim");
						//		shade::File file(path, shade::File::Out, "@s_sanim", shade::File::VERSION(0, 0, 1));

						//		if (file.IsOpen())
						//			file.Write(animation.Animation);
						//		else
						//			SHADE_CORE_WARNING("Failed to save animation, path = {}", path);
						//	}*/
						//}

						m_ImportedModel = nullptr; m_ImportedAnimations.clear();
						m_ImportModelModal = false;
					}
				}
			}

			/*if(ImGui::Button(""))
			auto path = shade::FileDialog::OpenFile("Supported formats(*.obj, *.fbx, *.dae) \0*.obj;*.fbx;*.dae\0");
			if (!path.empty())
			{


				m_ImportedModel = IModel::Import(path.string());
			}*/

		});

	ImGui::SetNextWindowSize(ImVec2{ 600, 400 });
	DrawModal("Pack files", m_PackFilesModal, [&]()
		{
			static std::string rootPath = std::filesystem::current_path().generic_string();
			static std::unordered_map < std::string, std::vector<std::string>> from;
			static std::unordered_map < std::string, std::string> to;

			if (ImGui::BeginTable("Search where", 2, ImGuiTableFlags_SizingStretchProp, { ImGui::GetContentRegionAvail().x, 0 }))
			{
				ImGui::TableNextColumn();
				{
					InputTextCol("Root", rootPath);
				}
				ImGui::TableNextColumn();
				{
					if (ImGui::Button("...##Root"))
					{
						rootPath = shade::FileDialog::SelectFolder("").string();
					}
				}
				ImGui::EndTable();
			}

			if (!rootPath.empty())
			{
				if (ImGui::Button("Scan", { ImGui::GetContentRegionAvail().x, 0 }))
				{
					from = shade::File::FindFilesWithExtensionExclude(rootPath, { ".dds", ".dll", ".glsl" });
				}
			}

			if (ImGui::BeginTable("table1", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
			{

				ImGui::TableSetupColumn("Extension");
				ImGui::TableSetupColumn("Path");

				ImGui::TableHeadersRow();

				auto interator = from.begin();

				for (auto interator = from.begin(); interator != from.end();)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					{
						ImGui::Text(interator->first.c_str());
					}
					ImGui::TableSetColumnIndex(1);
					{
						ImGui::PushItemWidth(175.f);
						InputTextD(interator->first.c_str(), to[interator->first]);
						ImGui::PopItemWidth();

					}
					ImGui::TableSetColumnIndex(2);
					{
						if (ImGui::Button(std::string("Select##" + interator->first).c_str()))
						{
							auto path = shade::FileDialog::SaveFile("Shade pakcet(*.bin) \0*.bin\0");
							if (!path.empty())
							{
								to[interator->first] = path.string();
							}
						}
					}
					ImGui::TableSetColumnIndex(3);
					{
						if (ImGui::Button(std::string("Remove##" + interator->first).c_str()))
						{
							auto toIter = to.find(interator->first);
							if (toIter != to.end())
							{
								to.erase(toIter);
							}

							interator = from.erase(interator);
						}
						else
							interator++;
					}
				}
				ImGui::EndTable();
			}

			if (!to.empty() && !from.empty())
			{
				if (ImGui::Button("Pack", { ImGui::GetContentRegionAvail().x, 0 }))
				{
					shade::File::Specification spec;
					shade::File::PackFiles({ from, to });

					from.clear(); to.clear();
					//m_PackFilesModal = false; 
				}
			}
		});

}

void DrawGizmoOperationButton(const char* id, const char8_t* icon, ImGuizmo::OPERATION operation, uint32_t& currentOperation, uint32_t allowedOperation)
{
	ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_Button];

	if (currentOperation & operation) color = ImVec4(0.995f, 0.857f, 0.420f, 1.000f);
		
	ImGui::PushStyleColor(ImGuiCol_Button, color);
	
	if (shade::ImGuiLayer::IconButton(id, icon, 1, 1.0f)) 
		if (allowedOperation & operation) { currentOperation = operation; }

	ImGui::PopStyleColor(); 
}

void EditorLayer::Scene(shade::SharedPointer<shade::Scene>& scene)
{
	ImGui::ShowDemoWindow();
	// Bool shit
	const std::uint32_t frameIndex = shade::Renderer::GetCurrentFrameIndex();

	static ImVec4 focusColor = { 0, 0, 0, 1 };

	/*if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
		ImGui::SetWindowFocus(NULL);*/

		// TODO: Need to rewrite this logic !
	if (ImGui::IsWindowFocused())
	{
		focusColor = { 0.995f, 0.857f, 0.420f, 1.000f };
		m_EditorCamera->SetUpdate(true);
	}
	else
	{
		focusColor = { 0, 0, 0, 1 };
		m_EditorCamera->SetUpdate(false);
	}

	if (m_SceneViewPort.ViewPort.z != ImGui::GetContentRegionAvail().x || m_SceneViewPort.ViewPort.w != ImGui::GetContentRegionAvail().y)
	{
		m_SceneViewPort.ViewPort.z = ImGui::GetContentRegionAvail().x;
		m_SceneViewPort.ViewPort.w = ImGui::GetContentRegionAvail().y;

		/*	auto camera = scene->GetPrimaryCamera();
			if (camera.IsValid())
				camera.GetComponent<shade::CameraComponent>()->Resize((float)m_SceneViewPort.ViewPort.z / (float)m_SceneViewPort.ViewPort.w);*/

		if (m_SceneViewPort.ViewPort.z && m_SceneViewPort.ViewPort.w)
			m_SceneRenderer->GetMainTargetFrameBuffer()[frameIndex]->Resize(m_SceneViewPort.ViewPort.z, m_SceneViewPort.ViewPort.w);
	}

	m_SceneViewPort.ViewPort.y = ImGui::GetWindowPos().x + ImGui::GetCursorPos().x; // With tab size
	m_SceneViewPort.ViewPort.x = ImGui::GetWindowPos().y + ImGui::GetCursorPos().y; // With tab size

	DrawImage(m_SceneRenderer->GetMainTargetFrameBuffer()[frameIndex]->GetTextureAttachment(0), { m_SceneViewPort.ViewPort.z, m_SceneViewPort.ViewPort.w }, focusColor);

	ImGui::SetNextWindowSize(ImVec2{ ImGui::GetWindowSize().x - 50.0f,0 }, ImGuiCond_Always);
	ShowWindowBarOverlay("Overlay", ImGui::GetWindowViewport(), [&]()
		{
			if (ImGui::BeginTable("##OverlayTable", 3, ImGuiTableFlags_SizingStretchProp))
			{
				ImGui::TableNextRow();
				{
					ImGui::TableNextColumn();
					{
						if (ImGui::BeginTable("##GuizmoEditMode", 4, ImGuiTableFlags_SizingFixedFit))
						{
							ImGui::TableNextColumn();
							{
								// Select

								/*ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_Button];

								if (m_ImGuizmoOperation == 0) color = ImVec4(0.995f, 0.857f, 0.420f, 1.000f);

								ImGui::PushStyleColor(ImGuiCol_Button, color);

								if (shade::ImGuiLayer::IconButton("##Select", u8"\xe9d9", 1, 1.0f))
								{
									m_ImGuizmoOperation = 0;
								}
									
								ImGui::PopStyleColor();*/

								DrawGizmoOperationButton("##Select", u8"\xe9d9", ImGuizmo::BOUNDS, m_ImGuizmoOperation, m_ImGuizmoAllowedOperation);
							}
							ImGui::TableNextColumn();
							{
								// Translate
								DrawGizmoOperationButton("##Translate", u8"\xea93", ImGuizmo::TRANSLATE, m_ImGuizmoOperation, m_ImGuizmoAllowedOperation);
							}
							ImGui::TableNextColumn();
							{
								// Rotation
								DrawGizmoOperationButton("##Rotate", u8"\xea98", ImGuizmo::ROTATE, m_ImGuizmoOperation, m_ImGuizmoAllowedOperation);
							}
							ImGui::TableNextColumn();
							{
								// Scale
								DrawGizmoOperationButton("##Scale", u8"\xea91", ImGuizmo::SCALE, m_ImGuizmoOperation, m_ImGuizmoAllowedOperation);
							}

							ImGui::EndTable();
						}

						ImGui::Dummy({ ImGui::GetWindowSize().x / 3.f, 0.f }); 

					}
					// Play, Puse, Stop scene
					ImGui::TableNextColumn();
					{
						if (ImGuiLayer::IconButton("##PlayButton", u8"\xe88b", 1, 1.f))
						{
							m_IsScenePlaying = true;
						}
						ImGui::SameLine(); ImGui::Dummy({ 10.f, 0 }); ImGui::SameLine();
						if (ImGuiLayer::IconButton("##PaueButton", u8"\xe88e", 1, 1.f) || shade::Input::IsKeyPressed(shade::Key::Escape))
						{
							m_IsScenePlaying = false;
							shade::Input::ShowMouseCursor(true);
						}
						ImGui::SameLine(); ImGui::Dummy({ 10.f, 0 }); ImGui::SameLine();
						if (ImGuiLayer::IconButton("##StopButton", u8"\xe88d", 1, 1.f) || shade::Input::IsKeyPressed(shade::Key::Escape))
						{
							m_IsScenePlaying = false;
							shade::Input::ShowMouseCursor(true);
						}

					}
					ImGui::TableNextColumn();
					{
						ImGuiIO& io = ImGui::GetIO();
						ImGui::Text("Application average %.1f ms/frame (%.0f FPS)", 1000.0f / io.Framerate, io.Framerate);
					}
				}

				ImGui::EndTable();
			}
		});

	// Geometry
	{
		//ImSequencer::Sequencer()
		if (m_SelectedEntity && m_SelectedEntity.HasComponent<shade::TransformComponent>())
		{
			auto& transform = m_SelectedEntity.GetComponent<shade::TransformComponent>();
			auto pcTransform = scene->ComputePCTransform(m_SelectedEntity);

			if (m_SelectedEntity.HasComponent<shade::GlobalLightComponent>())
				m_ImGuizmoAllowedOperation = ImGuizmo::ROTATE | ImGuizmo::BOUNDS;
			else if (m_SelectedEntity.HasComponent<shade::PointLightComponent>())
				m_ImGuizmoAllowedOperation = ImGuizmo::TRANSLATE | ImGuizmo::BOUNDS;
			else if (m_SelectedEntity.HasComponent<shade::SpotLightComponent>())
				m_ImGuizmoAllowedOperation = ImGuizmo::TRANSLATE | ImGuizmo::ROTATE | ImGuizmo::BOUNDS;
			else
				m_ImGuizmoAllowedOperation = ImGuizmo::TRANSLATE | ImGuizmo::ROTATE | ImGuizmo::SCALE | ImGuizmo::BOUNDS;

			if (!(m_ImGuizmoAllowedOperation & m_ImGuizmoOperation)) m_ImGuizmoOperation = ImGuizmo::BOUNDS;

			if (DrawImGuizmo(pcTransform, m_SceneRenderer->GetActiveCamera(), static_cast<ImGuizmo::OPERATION>(m_ImGuizmoAllowedOperation & m_ImGuizmoOperation), { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y }))
			{
				if (m_SelectedEntity.HasParent())
				{
					auto parenEntity = m_SelectedEntity.GetParent();
					auto parentTransform = scene->ComputePCTransform(parenEntity);
					pcTransform = glm::inverse(parentTransform) * pcTransform;
				}

				transform.SetTransform(pcTransform);
			}
		}
	}
	{

	}
}

bool EditorLayer::EntitiesList(const char* search, shade::SharedPointer<shade::Scene>& scene)
{
	static ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

	bool isItemSelected = false;

	scene->View<shade::TagComponent>().Each([&](shade::ecs::Entity& entity, shade::TagComponent& tag)
		{
			std::string	name = tag + "##" + entity;
			ImGuiTreeNodeFlags nodeFlags = baseFlags;

			if (!entity.HasParent() || !entity.GetParent().IsValid())
			{
				nodeFlags |= (!entity.HasChildren()) ? ImGuiTreeNodeFlags_Leaf : 0;

				if (m_SelectedEntity == entity)
					nodeFlags |= ImGuiTreeNodeFlags_Selected;

				bool isNodeOpen = ImGui::TreeNodeEx(name.c_str(), nodeFlags);

				if (ImGui::IsItemClicked())
				{
					m_SelectedEntity = entity;
					isItemSelected = true;
				}
					
				if (isNodeOpen)
				{
					EntitiesList(search, entity);
					ImGui::TreePop();
				}
			}
		});

	return isItemSelected;
}

void EditorLayer::EntitiesList(const char* search, shade::ecs::Entity& entity)
{
	static ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	for (auto& child : entity)
	{
		std::string name = child.GetComponent<shade::TagComponent>() + "##" + child;
		ImGuiTreeNodeFlags nodeFlags = baseFlags;

		nodeFlags |= (!child.HasChildren()) ? ImGuiTreeNodeFlags_Leaf : 0;

		if (m_SelectedEntity == child)
			nodeFlags |= ImGuiTreeNodeFlags_Selected;

		bool isNodeOpen = ImGui::TreeNodeEx(name.c_str(), nodeFlags);

		if (ImGui::IsItemClicked())
			m_SelectedEntity = child;

		if (isNodeOpen)
		{
			EntitiesList(search, child);
			ImGui::TreePop();
		}
	}
}

void EditorLayer::Entities(shade::SharedPointer<shade::Scene>& scene)
{
	if (ImGui::ListBoxHeader("##EntitiesList", ImGui::GetContentRegionAvail()))
	{
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("New entity"))
			{
				auto entity = scene->CreateEntity();
				entity.AddComponent<shade::TagComponent>("Entity");
			}
			if (ImGui::BeginMenu("Lighting"))
			{
				if (ImGui::MenuItem("Global"))
				{
					auto entity = scene->CreateEntity();
					entity.AddComponent<shade::TagComponent>("Global light");
					entity.AddComponent<shade::GlobalLightComponent>(shade::GlobalLightComponent::Create());
					entity.AddComponent<shade::TransformComponent>();
				}
				if (ImGui::MenuItem("Point"))
				{
					auto entity = scene->CreateEntity();
					entity.AddComponent<shade::TagComponent>("Point light");
					entity.AddComponent<shade::PointLightComponent>(shade::PointLightComponent::Create());
					entity.AddComponent<shade::TransformComponent>();
				}
				if (ImGui::MenuItem("Spot"))
				{
					auto entity = scene->CreateEntity();
					entity.AddComponent<shade::TagComponent>("Spot light");
					entity.AddComponent<shade::SpotLightComponent>(shade::SpotLightComponent::Create());
					entity.AddComponent<shade::TransformComponent>();
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		EntitiesList("", scene);
		
		ImGui::ListBoxFooter();

		if (m_SelectedEntity.IsValid())
		{
			if (ImGui::BeginPopupContextItem("##ComponentPopup"))
			{
				AddComponent<shade::TransformComponent>("Transform", false, m_SelectedEntity, [&](shade::ecs::Entity& entity)
					{
						entity.AddComponent<shade::TransformComponent>();
					}, m_SelectedEntity);
				AddComponent<shade::ModelComponent>("Model", false, m_SelectedEntity, [&](shade::ecs::Entity& entity)
					{
						entity.AddComponent<shade::ModelComponent>(shade::Model::CreateEXP());

					}, m_SelectedEntity);

				if (ImGui::BeginMenu("Lightning"))
				{
					AddComponent<shade::GlobalLightComponent>("Global light", false, m_SelectedEntity, [&](shade::ecs::Entity& entity)
						{
							entity.AddComponent<shade::GlobalLightComponent>(shade::GlobalLightComponent::Create());
						}, m_SelectedEntity);
					AddComponent<shade::SpotLightComponent>("Spot light", false, m_SelectedEntity, [&](shade::ecs::Entity& entity)
						{
							entity.AddComponent<shade::SpotLightComponent>(shade::SpotLightComponent::Create());
						}, m_SelectedEntity);
					AddComponent<shade::PointLightComponent>("Point light", false, m_SelectedEntity, [&](shade::ecs::Entity& entity)
						{
							entity.AddComponent<shade::PointLightComponent>(shade::PointLightComponent::Create());
						}, m_SelectedEntity);

					ImGui::EndMenu();
				}
				AddComponent<shade::RigidBodyComponent>("Rigid body", false, m_SelectedEntity, [&](shade::ecs::Entity& entity)
					{
						entity.AddComponent<shade::RigidBodyComponent>();
					}, m_SelectedEntity);

				AddComponent<shade::AnimationGraphComponent>("Animation graph", false, m_SelectedEntity, [&](shade::ecs::Entity& entity)
					{
						// WRONG ASSET Creation !!
						auto& graph = entity.AddComponent<shade::AnimationGraphComponent>();
						graph.AnimationGraph = shade::SharedPointer<shade::animation::AnimationGraph>::Create(&graph.GraphContext);
						graph.GraphContext.Controller = shade::animation::AnimationController::Create();
					}, m_SelectedEntity);

				AddComponent<shade::NativeScriptComponent>("Native script", false, m_SelectedEntity, [&](shade::ecs::Entity& entity)
					{
						entity.AddComponent<shade::NativeScriptComponent>();
					}, m_SelectedEntity);
				AddComponent<shade::CameraComponent>("Camera", false, m_SelectedEntity, [&](shade::ecs::Entity& entity)
					{
						entity.AddComponent<shade::CameraComponent>(shade::CameraComponent::Create())->SetPrimary(true);

					}, m_SelectedEntity);

				ImGui::Separator();
				if (ImGui::MenuItem("New entity as child"))
				{
					shade::ecs::Entity entity = scene->CreateEntity();
					entity.AddComponent<shade::TagComponent>("New enity");
					m_SelectedEntity.AddChild(entity);
				}
				if (m_SelectedEntity.HasParent())
				{
					if (ImGui::MenuItem("Unset parent"))
					{
						m_SelectedEntity.UnsetParent();
					}
				}
				ImGui::Separator();
				// Remove, need to create specific fucntion ?
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7, 0, 0, 1));
				if (ImGui::MenuItem("Remove entity"))
				{
					m_SelectedEntity.Destroy();
					m_SelectedEntity = { shade::ecs::null, nullptr };
				}

				ImGui::PopStyleColor();
				ImGui::EndPopup();
			}
		}
	}


	/*scene->View<std::string>().Each([](shade::ecs::Entity& entity, std::string& tag)
		{
			std::string		_tag = tag + "##" + entity;

		});*/
}

void EditorLayer::AssetsExplorer()
{

	static std::string assetMetaPath = SHADE_ASSET_META_FILE_PATH;

	if (ImGui::BeginTable("#RegisterNewAssetOrOverrideTable", 3, ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		{
			if (ImGui::Button("Register new asset"))
				m_IsAddNewAssetModalOpen = true;
		}
		ImGui::TableNextColumn();
		{
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			InputTextD("##AssetMetaPath", assetMetaPath);
		}
		ImGui::TableNextColumn();
		{
			if (ImGui::Button("Override asset meta"))
			{
				if (assetMetaPath.empty())
					shade::AssetManager::Save();
				else
					shade::AssetManager::Save(assetMetaPath);
			}
		}

		ImGui::EndTable();
	}
	//static std::string assetMetaFilePath = shade::AssetManager::G

	//InputTextCol("Asset meta file: ",)
	//ImGui::Text("Some Path");

	auto width = ImGui::GetContentRegionAvail().x / 3;
	auto height = width / 2;

	ImGui::BeginChild("Categories", ImVec2(width, height), true);
	ImGui::Text("Categories:");
	ImGui::Separator();

	static shade::AssetMeta::Category selectedCategory = shade::AssetMeta::Category::None;
	static shade::AssetMeta::Type selectedType = shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM;
	static shade::SharedPointer<shade::AssetData> selectedAssetData;

	if (ImGui::Selectable("Secondary", selectedCategory == shade::AssetMeta::Category::Secondary))
		selectedCategory = shade::AssetMeta::Category::Secondary;
	if (ImGui::Selectable("Primary", selectedCategory == shade::AssetMeta::Category::Primary))
		selectedCategory = shade::AssetMeta::Category::Primary;
	if (ImGui::Selectable("Blueprint", selectedCategory == shade::AssetMeta::Category::Blueprint))
		selectedCategory = shade::AssetMeta::Category::Blueprint;

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::BeginChild("Types", ImVec2(width, height), true);
	ImGui::Text("Types:");
	ImGui::Separator();
	for (shade::AssetMeta::Type type = shade::AssetMeta::Type::Asset; type < shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM; ((std::uint32_t&)type)++)
	{
		if (ImGui::Selectable(shade::AssetMeta::GetTypeAsString(type).c_str(), selectedType == type))
			selectedType = type;
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("Assets", ImVec2(width, height), true);
	static std::string search;
	InputTextCol("Assets:", search);
	ImGui::Separator();
	for (auto& [id, asset] : shade::AssetManager::GetAssetDataList(selectedCategory))
	{
		if (asset->GetCategory() == selectedCategory && asset->GetType() == selectedType)
		{
			if (id.find(search) != std::string::npos)
			{
				if (ImGui::Selectable(id.c_str(), selectedAssetData == asset))
					selectedAssetData = asset;
			}
		}
	}
	ImGui::EndChild();
	ImGui::EndGroup();

	ImGui::Spacing();

	EditAsset(selectedAssetData);

	ImGui::SetNextWindowSize({ 300, 200 });
	DrawModal("Register new asset", m_IsAddNewAssetModalOpen, [&]()
		{
			auto width = ImGui::GetContentRegionAvail().x / 3;
			auto height = width / 2;

			static shade::AssetMeta::Category selectedCategory = shade::AssetMeta::Category::None;
			static shade::AssetMeta::Type selectedType = shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM;
			static std::string id;
			static std::string path;
			static shade::SharedPointer<shade::AssetData> assetData = shade::SharedPointer<shade::AssetData>::Create();


			if (ImGui::BeginTable("##RegisterNewAssetTable", 2))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn(); { ImGui::Text("Category"); }

				ImGui::TableNextColumn();
				{
					std::vector<std::string> items(shade::AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM);
					for (shade::AssetMeta::Category cat = shade::AssetMeta::Category::None; cat < shade::AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM; ((std::uint32_t&)cat)++)
						items[cat] = shade::AssetMeta::GetCategoryAsString(cat);
					std::string currentItem = shade::AssetMeta::GetCategoryAsString(selectedCategory);

					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (DrawCombo(" ##Category", currentItem, items, ImGuiSelectableFlags_None, ImGuiComboFlags_None))
						selectedCategory = shade::AssetMeta::GetCategoryFromString(currentItem);

					assetData->SetCategory(selectedCategory);
				}
				ImGui::TableNextRow();
				ImGui::TableNextColumn(); { ImGui::Text("Type"); }
				ImGui::TableNextColumn();
				{
					std::vector<std::string> items(shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM);
					for (shade::AssetMeta::Type typ = shade::AssetMeta::Type::Asset; typ < shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM; ((std::uint32_t&)typ)++)
						items[typ] = shade::AssetMeta::GetTypeAsString(typ);
					std::string currentItem = shade::AssetMeta::GetTypeAsString(selectedType);

					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					DrawCombo(" ##Type", currentItem, items, ImGuiSelectableFlags_None, ImGuiComboFlags_None);

					selectedType = shade::AssetMeta::GetTypeFromString(currentItem);

					assetData->SetType(selectedType);
				}
				ImGui::TableNextRow();
				ImGui::TableNextColumn(); { ImGui::Text("Id"); }
				ImGui::TableNextColumn();
				{
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					if (InputTextD("##Id", id)) { assetData->SetId(id); }
				}
				{
					if (selectedCategory == shade::AssetMeta::Category::Secondary)
					{

						ImGui::TableNextRow();
						ImGui::TableNextColumn(); { ImGui::Text("Path"); }
						ImGui::TableNextColumn();
						{
							if (ImGui::BeginTable("##registerNewAsset_SelectPath", 2, ImGuiTableFlags_SizingStretchProp))
							{
								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								{
									ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
									if (InputTextD(" ##Path", path)) { assetData->SetAttribute<std::string>("Path", path); }
								}
								ImGui::TableNextColumn();
								{
									if (ImGui::Button("..."))
									{
										auto selectedPath = shade::FileDialog::OpenFile("");
										if (!selectedPath.empty())
										{
											path = selectedPath.string();
											assetData->SetAttribute<std::string>("Path", path);
										}

									}
								}
								ImGui::EndTable();
							}
						}

					}
					if (selectedCategory == shade::AssetMeta::Category::Primary)
					{
						std::vector<std::string> items;
						static std::string currentItem = "None";

						for (auto& [id, reference] : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
						{
							if (reference->GetType() == selectedType)
							{
								items.emplace_back(id);
							}
						}

						if (items.size())
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); { ImGui::Text("Reference"); }
							ImGui::TableNextColumn();
							{
								if (DrawCombo(" ##Reference", currentItem, items, ImGuiSelectableFlags_None, ImGuiComboFlags_None))
								{
									auto assetReference = shade::AssetManager::GetAssetData(shade::AssetMeta::Category::Secondary, currentItem);
									if (assetReference)
									{
										assetData->SetReference(assetReference);
									}
								}
							}
						}
					}
				}

				ImGui::EndTable();
			}

			if (!id.empty())
			{
				if (ImGui::Button("Save", { ImGui::GetContentRegionAvail().x, 0 }))
				{
					shade::AssetManager::AddNewAssetData(assetData);
					// Reset
					assetData = shade::SharedPointer<shade::AssetData>::Create();
					selectedCategory = shade::AssetMeta::Category::None;
					selectedType = shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM;
					id.clear();
					path.clear();
				}
			}
		});
}

void EditorLayer::Creator()
{
	auto width = ImGui::GetContentRegionAvail().x / 3;
	auto height = width / 2;

	static shade::AssetMeta::Type selectedType = shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM;
	static shade::SharedPointer<shade::AssetData> selectedAssetData;

	ImGui::BeginChild("##CreateorTypes", ImVec2(ImGui::GetContentRegionAvail().x, height), true);
	ImGui::Text("Types:");
	ImGui::Separator();
	for (shade::AssetMeta::Type type = shade::AssetMeta::Type::Asset; type < shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM; ((std::uint32_t&)type)++)
	{
		if (ImGui::Selectable(shade::AssetMeta::GetTypeAsString(type).c_str(), selectedType == type))
			selectedType = type;
	}
	ImGui::EndChild();

	{
		ImGui::BeginChild("##Createor", ImVec2(ImGui::GetContentRegionAvail()), true);

		switch (selectedType)
		{
		case shade::AssetMeta::Asset:
			break;
		case shade::AssetMeta::Model:
			break;
		case shade::AssetMeta::Mesh:
			break;
		case shade::AssetMeta::Material: CreateMaterial();
			break;
		case shade::AssetMeta::Texture:
			break;
		case shade::AssetMeta::Animation:
			break;
		case shade::AssetMeta::Sound:
			break;
		case shade::AssetMeta::CollisionShapes: CreateCollisionShapes();
			break;
		case shade::AssetMeta::Other:
			break;
		case shade::AssetMeta::ASSET_TYPE_MAX_ENUM:
			break;
		default:
			break;
		}
		ImGui::EndChild();
	}

}

void EditorLayer::EditAsset(shade::SharedPointer<shade::AssetData>& assetData)
{
	auto width = ImGui::GetContentRegionAvail().x / 6;
	auto height = ImGui::GetContentRegionAvail().y / 3;

	if (assetData)
	{
		const shade::AssetMeta::Category category = assetData->GetCategory();
		const shade::AssetMeta::Type type = assetData->GetType();

		std::string id = assetData->GetId();

		if (ImGui::BeginTable("##EditExistingAssetTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn(); { ImGui::Text("Id         "); }
			ImGui::TableNextColumn();
			{

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				if (InputTextD(" ##Id", id)) { assetData->SetId(id); }
			}
			ImGui::TableNextRow();
			ImGui::TableNextColumn(); { ImGui::Text("Category"); }
			ImGui::TableNextColumn();
			{
				std::vector<std::string> items(shade::AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM);
				for (shade::AssetMeta::Category cat = shade::AssetMeta::Category::None; cat < shade::AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM; ((std::uint32_t&)cat)++)
					items[cat] = shade::AssetMeta::GetCategoryAsString(cat);
				std::string currentItem = shade::AssetMeta::GetCategoryAsString(category);

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				DrawCombo(" ##Category", currentItem, items, ImGuiSelectableFlags_None, ImGuiComboFlags_None);
			}
			ImGui::TableNextRow();
			ImGui::TableNextColumn(); { ImGui::Text("Type"); }
			ImGui::TableNextColumn();
			{
				std::vector<std::string> items(shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM);
				for (shade::AssetMeta::Type typ = shade::AssetMeta::Type::Asset; typ < shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM; ((std::uint32_t&)typ)++)
					items[typ] = shade::AssetMeta::GetTypeAsString(typ);
				std::string currentItem = shade::AssetMeta::GetTypeAsString(type);

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				DrawCombo(" ##Type", currentItem, items, ImGuiSelectableFlags_None, ImGuiComboFlags_None);
			}
			if (assetData->GetCategory() == shade::AssetMeta::Category::Primary)
			{
				std::vector<std::string> items;
				std::string currentItem = (assetData->GetReference()) ? assetData->GetReference()->GetId() : "None";

				for (auto& [id, reference] : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
				{
					if (reference->GetType() == assetData->GetType())
					{
						items.emplace_back(id);
					}
				}

				if (items.size())
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); { ImGui::Text("Reference"); }
					ImGui::TableNextColumn();
					{
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
						if (DrawCombo("Reference", currentItem, items, ImGuiSelectableFlags_None, ImGuiComboFlags_None))
						{
							auto assetReference = shade::AssetManager::GetAssetData(shade::AssetMeta::Category::Secondary, currentItem);
							if (assetReference)
								assetData->SetReference(assetReference);
						}
					}
				}
			}

			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					static std::string search;

					if (ImGui::BeginTable("##ExistingDependciesTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn(); { ImGui::Text("Dependencies"); }
						ImGui::TableNextColumn();
						{
							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
							InputTextD("##Dependencies", search);
						}
						ImGui::EndTable();
					}

					if (ImGui::ListBoxHeader("Dependencies##Dependencies", { ImGui::GetContentRegionAvail().x, height }))
					{
						for (auto i = 0; i < assetData->GetDependencies().size(); i++)
						{
							if (assetData->GetDependencies()[i]->GetId().find(search) != std::string::npos)
							{
								if (ImGui::Selectable(assetData->GetDependencies()[i]->GetId().c_str()))
									assetData->GetDependencies().erase(assetData->GetDependencies().begin() + i);
							}
						}

						ImGui::ListBoxFooter();
					}

				}
				ImGui::TableNextColumn();
				{
					static std::string search;

					if (ImGui::BeginTable("##AddNewDependancyTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn(); { ImGui::Text("Add new dependency"); }
						ImGui::TableNextColumn();
						{
							ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
							InputTextD("##AddNewDependency", search);
						}
						ImGui::EndTable();
					}

					if (ImGui::BeginTable("##AddNewDependancySelectCategoryTable", 2))
					{
						ImGui::TableSetupColumn("Primary");
						ImGui::TableSetupColumn("Secondary");
						ImGui::TableHeadersRow();

						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						{
							if (ImGui::ListBoxHeader("##AddDependenciesPrimary", { ImGui::GetContentRegionAvail().x, height - 20 }))
							{
								for (auto& [id, asset] : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Primary))
								{
									if (type != asset->GetType() && asset->GetId().find(search) != std::string::npos)
									{
										if (ImGui::Selectable(asset->GetId().c_str()))
										{
											assetData->AddDependency(asset);
										}
									}
								}
								ImGui::ListBoxFooter();
							}
						}
						ImGui::TableNextColumn();
						{
							if (ImGui::ListBoxHeader("##AddDependenciesSecondary", { ImGui::GetContentRegionAvail().x, height - 20 }))
							{
								for (auto& [id, asset] : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
								{
									if (type != asset->GetType() && asset->GetId().find(search) != std::string::npos)
									{
										if (ImGui::Selectable(asset->GetId().c_str()))
										{
											assetData->AddDependency(asset);
										}
									}
								}
								ImGui::ListBoxFooter();
							}
						}
						ImGui::EndTable();
					}
				}
			}
			ImGui::EndTable();
		}
		if (ImGui::TreeNodeEx("Attributes", ImGuiTreeNodeFlags_Framed))
		{
			if (ImGui::Button("Create new", { ImGui::GetContentRegionAvail().x,0 }))
				m_IsAddNewAttributeModalOpen = true;


			if (ImGui::BeginTable("##AddRemoveAttributeTable", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp, { ImGui::GetContentRegionAvail().x, height / 2.f }))
			{
				ImGui::TableSetupColumn("Key            ");
				ImGui::TableSetupColumn("Value");
				ImGui::TableHeadersRow();

				for (auto& [key, attribute] : assetData->GetAttributes())
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); { ImGui::Text(key.c_str()); }
					ImGui::TableNextColumn();
					{
						ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
						InputTextD(std::string("##Attribute" + key).c_str(), attribute);
					}
					ImGui::TableNextColumn();
					{
						if (ImGui::Button(std::string("Remove##" + key).c_str()))
						{
							assert(false);
						}
					}
				}
				ImGui::EndTable();
			}

			ImGui::TreePop();
		}
	}

	DrawModal("Attribute", m_IsAddNewAttributeModalOpen, [&]()
		{
			static std::string key, value;
			InputTextCol("Key", key, 100, 300);
			InputTextCol("Value", value, 100, 300);

			if (!key.empty() && !value.empty())
			{
				if (ImGui::Button("Save", { ImGui::GetContentRegionAvail().x, 0 }))
				{
					assetData->SetAttribute(key, value);
					key.clear(); value.clear();
				}
			}
		});
}

void EditorLayer::RenderSettings(shade::SharedPointer<shade::SceneRenderer>& renderer)
{
	if (ImGui::TreeNodeEx("Render", ImGuiTreeNodeFlags_Framed))
	{
		if (ImGui::TreeNodeEx("Bloom", ImGuiTreeNodeFlags_Framed))
		{

			ImGui::Checkbox("Enable", &renderer->GetSettings().BloomSettings.Enabled);
			DragFloat("Threshold", &renderer->GetSettings().BloomSettings.Threshold, 0.001f, 0.f, FLT_MAX, 80.f);
			DragFloat("Knee", &renderer->GetSettings().BloomSettings.Knee, 0.001f, 0.f, FLT_MAX, 80.f);
			DragFloat("Exposure", &renderer->GetSettings().BloomSettings.Exposure, 0.001f, 0.f, FLT_MAX, 80.f);

			//ImGui::SliderInt("Samples", (int*)&renderer->GetSettings().BloomSettings.Samples, 1, 10);
			SliderInt("Samples", (int*)&renderer->GetSettings().BloomSettings.Samples, 1, 10, 80.f);
			//DrawCurve("Curve", glm::value_ptr(m_PPBloom->GetCurve()), 3, ImVec2{ ImGui::GetContentRegionAvail().x, 70 });*/
			ImGui::TreePop();

			if (ImGui::TreeNodeEx("Down + Up samples ", ImGuiTreeNodeFlags_Framed))
			{
				static int mipLevel = 0;
				SliderInt("Level", &mipLevel, 0, m_SceneRenderer->GetSettings().BloomSettings.Samples - 1, 80.f);
				DrawImage(m_SceneRenderer->GetBloomRenderTarget(), { 300, 200 }, {}, mipLevel);
				ImGui::TreePop();
			}
		}

		if (ImGui::TreeNodeEx("Color correction", ImGuiTreeNodeFlags_Framed))
		{

			ImGui::Checkbox("Enable", &renderer->GetSettings().ColorCorrectionSettings.Enabled);
			DragFloat("Gamma", &renderer->GetSettings().ColorCorrectionSettings.Gamma, 0.001f, 0.f, FLT_MAX, 80.f);
			DragFloat("Exposure", &renderer->GetSettings().ColorCorrectionSettings.Exposure, 0.001f, 0.f, FLT_MAX, 80.f);
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("SSAO", ImGuiTreeNodeFlags_Framed))
		{
			ImGui::Checkbox("Enable", &m_SceneRenderer->GetSettings().RenderSettings.SSAOEnabled);
			DragFloat("Radius", &m_SceneRenderer->GetSettings().SSAOSettings.Radius, 0.01f, -FLT_MAX, FLT_MAX, 80.f);
			DragFloat("Bias", &m_SceneRenderer->GetSettings().SSAOSettings.Bias, 0.01f, -FLT_MAX, FLT_MAX, 80.f);
			SliderInt("Blur samples", (int*)&m_SceneRenderer->GetSettings().SSAOSettings.BlurSamples, 0, 20);
			if (ImGui::TreeNodeEx("SSAO map", ImGuiTreeNodeFlags_Framed))
			{
				DrawImage(m_SceneRenderer->GetSAAORenderTarget(), { 300, 200 }, {});
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx("Shadows", ImGuiTreeNodeFlags_Framed))
		{
			ImGui::Checkbox("Global light shadows", &renderer->GetSettings().RenderSettings.GlobalShadowsEnabled);
			ImGui::Checkbox("Spot light shadows", &renderer->GetSettings().RenderSettings.SpotShadowEnabled);
			ImGui::Checkbox("Point light shadows", &renderer->GetSettings().RenderSettings.PointShadowEnabled);
			ImGui::Checkbox("Point light shadows split by faces", &renderer->GetSettings().IsPointLightShadowSplitBySides);
			ImGui::TreePop();
		}
		ImGui::Checkbox("Light culling", &renderer->GetSettings().RenderSettings.LightCulling);


		if (ImGui::TreeNodeEx("Debug", ImGuiTreeNodeFlags_Framed))
		{
			ImGui::Checkbox("Show Light complexity", &renderer->GetSettings().RenderSettings.ShowLightComplexity);
			ImGui::Checkbox("Show Grid", &renderer->GetSettings().IsGridShow);
			ImGui::Checkbox("Show AABB&OBB", &renderer->GetSettings().IsAABB_OBBShow);
			ImGui::Checkbox("Show Point Light", &renderer->GetSettings().IsPointLightShow);
			ImGui::Checkbox("Show Spot Light", &renderer->GetSettings().IsSpotLightShow);
			ImGui::Checkbox("Show Global light shadow's cascades", &renderer->GetSettings().RenderSettings.ShowShadowCascades);

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}


	if (ImGui::TreeNodeEx("Statistic", ImGuiTreeNodeFlags_Framed))
	{
		ImGui::Text("Submited instances %d", m_SceneRenderer->GetStatistic().SubmitedInstances);
		ImGui::Separator();
		ImGui::Text("Submited point lights %d", m_SceneRenderer->GetStatistic().SubmitedPointLights);
		ImGui::Separator();
		ImGui::Text("Submited spot lights %d", m_SceneRenderer->GetStatistic().SubmitedSpotLights);
		ImGui::Separator();

		ImGui::Text("Light culling pre depth pass %0.3f ms", m_SceneRenderer->GetStatistic().LightCullingPreDepth);
		ImGui::Separator();
		ImGui::Text("Light culling compute pass %0.3f ms", m_SceneRenderer->GetStatistic().LightCullingCompute);
		ImGui::Separator();
		ImGui::Text("Instanced geometry pass %0.3f ms", m_SceneRenderer->GetStatistic().InstanceGeometry);
		ImGui::Separator();
		ImGui::Text("Global light shadow pre depth pass %0.3f ms", m_SceneRenderer->GetStatistic().GlobalLightPreDepth);
		ImGui::Separator();
		ImGui::Text("Spot light shadow pre depth pass %0.3f ms", m_SceneRenderer->GetStatistic().SpotLightPreDepth);
		ImGui::Separator();
		ImGui::Text("Point light shadow pre depth pass %0.3f ms", m_SceneRenderer->GetStatistic().PointLightPreDepth);
		ImGui::Separator();
		ImGui::Text("Bloom pass %0.3f ms", m_SceneRenderer->GetStatistic().Bloom);
		ImGui::Separator();
		ImGui::Text("Color correction pass %0.3f ms", m_SceneRenderer->GetStatistic().ColorCorrection);
		ImGui::Separator();

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Pipelines", ImGuiTreeNodeFlags_Framed))
	{
		if (ImGui::Button("Recompile all pipelines"))
			m_SceneRenderer->RecompileAllPipelines();

		ImGui::TreePop();
	}
}

void EditorLayer::EntityInspector(shade::ecs::Entity& entity)
{
	if (entity.IsValid())
	{
		DrawComponent<shade::TagComponent>("Tag", entity, &EditorLayer::TagComponent,
			[&](auto isTreeOpen)->bool { return EditComponent<shade::TagComponent>(entity, {}, isTreeOpen); }, this, entity);
		DrawComponent<shade::TransformComponent>("Transform", entity, &EditorLayer::TransformComponent,
			[&](auto isTreeOpen)->bool { return EditComponent<shade::TransformComponent>(entity, {}, isTreeOpen);  }, this, entity);
		DrawComponent<shade::GlobalLightComponent>("Global Light", entity, &EditorLayer::GlobalLightComponent,
			[&](auto isTreeOpen)->bool { return EditComponent<shade::GlobalLightComponent>(entity, {}, isTreeOpen); }, this, entity);
		DrawComponent<shade::PointLightComponent>("Point Light", entity, &EditorLayer::PointLightComponent,
			[&](auto isTreeOpen)->bool { return EditComponent<shade::PointLightComponent>(entity, {}, isTreeOpen); }, this, entity);
		DrawComponent<shade::SpotLightComponent>("Spot Light", entity, &EditorLayer::SpotLightComponent,
			[&](auto isTreeOpen)->bool { return EditComponent<shade::SpotLightComponent>(entity, {}, isTreeOpen); }, this, entity);
		DrawComponent<shade::CameraComponent>("Camera", entity, &EditorLayer::CameraComponent,
			[&](auto isTreeOpen)->bool { return EditComponent<shade::CameraComponent>(entity, {}, isTreeOpen); }, this, entity);
		DrawComponent<shade::ModelComponent>("Model", entity, &EditorLayer::ModelComponent,
			[&](auto isTreeOpen)->bool { return EditComponent<shade::ModelComponent>(entity, {}, isTreeOpen); }, this, entity);
		DrawComponent<shade::RigidBodyComponent>("Rigid Body", entity, &EditorLayer::RgidBodyComponent,
			[&](auto isTreeOpen)->bool { return EditComponent<shade::RigidBodyComponent>(entity, {}, isTreeOpen); }, this, entity);
		DrawComponent<shade::AnimationGraphComponent>("Animations", entity, &EditorLayer::AnimationGraphComponent,
			[&](auto isTreeOpen)->bool { return EditComponent<shade::AnimationGraphComponent>(entity, {}, isTreeOpen); }, this, entity);
		DrawComponent<shade::NativeScriptComponent>("Naitve script", entity, &EditorLayer::NativeScriptComponent,
			[&](auto isTreeOpen)->bool { return EditComponent<shade::NativeScriptComponent>(entity, {}, isTreeOpen); }, this, entity);
	}
}

void EditorLayer::AddNewAsset()
{
	auto width = ImGui::GetContentRegionAvail().x / 5;
	static shade::AssetMeta::Category category = shade::AssetMeta::Category::None;
	static shade::AssetMeta::Type type = shade::AssetMeta::Asset;

	std::string id;

	if (InputTextCol("Id", id, width, ImGui::GetContentRegionAvail().x))
	{

	}
	{
		std::vector<std::string> items(shade::AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM);
		for (shade::AssetMeta::Category cat = shade::AssetMeta::Category::None; cat < shade::AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM; ((std::uint32_t&)cat)++)
			items[cat] = shade::AssetMeta::GetCategoryAsString(cat);
		std::string currentItem = shade::AssetMeta::GetCategoryAsString(category);

		if (ComboCol("Category", currentItem, items, ImGuiSelectableFlags_None, ImGuiComboFlags_None, width))
		{
			category = shade::AssetMeta::GetCategoryFromString(currentItem);
		}
	}

	{
		std::vector<std::string> items(shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM);
		for (shade::AssetMeta::Type typ = shade::AssetMeta::Type::Asset; typ < shade::AssetMeta::Type::ASSET_TYPE_MAX_ENUM; ((std::uint32_t&)typ)++)
			items[typ] = shade::AssetMeta::GetTypeAsString(typ);
		std::string currentItem = shade::AssetMeta::GetTypeAsString(type);

		if (ComboCol("Type", currentItem, items, ImGuiSelectableFlags_None, ImGuiComboFlags_None, width))
		{
			type = shade::AssetMeta::GetTypeFromString(currentItem);
		}
	}
	{
		std::vector<std::string> items;
		static std::string currentItem = "None";

		for (auto& [id, reference] : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
		{
			if (reference->GetType() == type)
			{
				items.emplace_back(id);
			}
		}

		if (ComboCol("Reference", currentItem, items, ImGuiSelectableFlags_None, ImGuiComboFlags_None, width))
		{
			auto assetReference = shade::AssetManager::GetAssetData(shade::AssetMeta::Category::Secondary, currentItem);
			if (assetReference)
			{
				// TODO:
			}
		}
	}



	ImGui::Button("Add New");
}

void EditorLayer::TagComponent(shade::ecs::Entity& entity)
{
	auto& tag = entity.GetComponent<shade::TagComponent>();
	InputTextCol("Tag", tag);
}

void EditorLayer::TransformComponent(shade::ecs::Entity& entity)
{
	auto& transform = entity.GetComponent<shade::TransformComponent>();
	ImGui::AlignTextToFramePadding();
	//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0 , ImGui::GetStyle().FramePadding.y * 2.5f });
	DragFloat3("Position", glm::value_ptr(transform.GetPosition()), 0.f, 0.1f, -FLT_MAX, FLT_MAX);
	DragFloat3("Rotation", glm::value_ptr(transform.GetRotation()), 0.f, 0.1f, -FLT_MAX, FLT_MAX);
	DragFloat3("Scale", glm::value_ptr(transform.GetScale()), 0.f, 0.1f, 0.f, FLT_MAX);
	//ImGui::PopStyleVar();
}

void EditorLayer::GlobalLightComponent(shade::ecs::Entity& entity)
{
	auto& globalLight = entity.GetComponent<shade::GlobalLightComponent>();
	DragFloat("Intensity", &globalLight->Intensity, 0.01f, 0.f);
	ColorEdit3("Diffuse color", glm::value_ptr(globalLight->DiffuseColor));
	ColorEdit3("Specular color", glm::value_ptr(globalLight->SpecularColor));

	DragFloat("Near", &globalLight->zNearPlaneOffset);
	DragFloat("Far", &globalLight->zFarPlaneOffset);
}

void EditorLayer::PointLightComponent(shade::ecs::Entity& entity)
{
	auto& pointLight = entity.GetComponent<shade::PointLightComponent>();
	DragFloat("Intensity", &pointLight->Intensity, 0.01f);
	DragFloat("Distance", &pointLight->Distance, 0.01f);
	DragFloat("Falloff", &pointLight->Falloff, 0.01f, -FLT_MAX);
	ColorEdit3("Diffuse color", glm::value_ptr(pointLight->DiffuseColor));
	ColorEdit3("Specular color", glm::value_ptr(pointLight->SpecularColor));
}

void EditorLayer::SpotLightComponent(shade::ecs::Entity& entity)
{
	auto& spotLight = entity.GetComponent<shade::SpotLightComponent>();
	DragFloat("Intensity", &spotLight->Intensity, 0.01);
	DragFloat("Distance", &spotLight->Distance, 0.01);
	DragFloat("Falloff", &spotLight->Falloff, 0.01);
	DragFloat("MinAngle", &spotLight->MinAngle, 0.01);
	DragFloat("MaxAngle", &spotLight->MaxAngle, 0.01);
	ColorEdit3("Diffuse color", glm::value_ptr(spotLight->DiffuseColor));
	ColorEdit3("Specular color", glm::value_ptr(spotLight->SpecularColor));
}

void EditorLayer::CameraComponent(shade::ecs::Entity& entity)
{
	auto& camera = entity.GetComponent<shade::CameraComponent>();
	DragFloat("Fov", &camera->GetFov(), 0.1);
	DragFloat("Aspect", &camera->GetAspect(), 0.1);
	DragFloat("ZNear", &camera->GetNear(), 0.1);
	DragFloat("ZFar", &camera->GetFar(), 0.1);
}


void CreateArrowButton(const std::string& id, bool& isPopupOpen, float& screenPosY, std::function<bool()>& AssetLoadCallback, const std::function<bool()>& callback) {
	if (ImGui::ArrowButton(id.c_str(), ImGuiDir_Down))
	{
		AssetLoadCallback = callback;
		isPopupOpen = !isPopupOpen;
		screenPosY = ImGui::GetCursorScreenPos().y;
	}
}

template <typename T>
bool LoadAsset(const std::string& search, shade::Asset<T>& asset, shade::AssetMeta::Type type, shade::AssetMeta::Category category) {
	for (const auto& assetData : shade::AssetManager::GetAssetDataList(category)) {
		if (assetData.second->GetType() == type && assetData.first.find(search) != std::string::npos) {
			if (ImGui::Selectable(assetData.first.c_str(), false)) {
				shade::AssetManager::GetAsset<T, shade::BaseAsset::InstantiationBehaviour::Synchronous>(
					assetData.first, category, shade::BaseAsset::LifeTime::KeepAlive,
					[&](auto& loadedAsset) mutable {
						asset = loadedAsset;
					}
				);
				return true;
			}
		}
	}
	return false;
}

void OpenImageCallback(shade::Asset<shade::Texture2D>& texture)
{
	auto path = shade::FileDialog::OpenFile("Texture (*.dds) \0*.dds\0");
	if (!path.empty())
	{
		if (std::ifstream file = std::ifstream(path, std::ios::binary))
		{
			shade::render::Image image; shade::Serializer::Deserialize(file, image);
			texture = shade::Texture2D::CreateEXP(shade::render::Image2D::Create(image));
		}
	}
	else
	{
		SHADE_CORE_WARNING("Couldn't open texture file, path ={0}", path);
	}
}

void HandleTexture(EditorLayer& layer, const std::string& buttonId, const char8_t* c, const char* textureTypeName, bool& IsAssetLoadPopupOpen, float& screenPostionY, std::string& search, shade::Asset<shade::Texture2D>& texture, std::function<bool()>& AssetLoadCallback)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	shade::ImGuiLayer::DrawFontIcon(u8"\xe9a1", 1, 0.6f);
	if (ImGui::IsItemHovered() && texture)
	{
		ImGui::BeginTooltip();
		layer.DrawImage(texture, ImVec2{ 300, 300 }, ImVec4{ 0.995f, 0.857f, 0.420f, 1.000f });
		ImGui::EndTooltip();

	}

	ImGui::TableNextColumn();
	shade::ImGuiLayer::DrawFontIcon(c, 1, 0.6f, textureTypeName);
	ImGui::TableNextColumn();
	if (shade::ImGuiLayer::IconButton("Open", u8"\xe85f", 1, 1.f))
	{
		OpenImageCallback(texture);
	}
	ImGui::TableNextColumn();
	std::string buttonTitle = (!texture || !texture->GetAssetData()) ? "NONE" : texture->GetAssetData()->GetId();
	ImGui::BeginDisabled();
	ImGui::Button(buttonTitle.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0.f });
	ImGui::EndDisabled();
	ImGui::TableNextColumn();

	CreateArrowButton(("##TextureAssetPopup" + buttonId).c_str(), IsAssetLoadPopupOpen, screenPostionY, AssetLoadCallback, [&]() { return LoadAsset<shade::Texture2D>(search, texture, shade::AssetMeta::Type::Texture, shade::AssetMeta::Category::Secondary); });
	ImGui::TableNextColumn();
	if (shade::ImGuiLayer::IconButton(("##Remove" + buttonId).c_str(), u8"\xe85d", 1, 1.0f)) { texture = nullptr; }
}

void EditorLayer::ModelComponent(shade::ecs::Entity& entity)
{
	ImGui::ShowDemoWindow();
	std::function<void(shade::Asset<shade::Texture2D>&)> OpenImageCallBack = [](shade::Asset<shade::Texture2D>& texture)
		{
			auto path = shade::FileDialog::OpenFile("Texture (*.dds) \0*.dds\0");
			if (!path.empty())
			{
				if (std::ifstream file = std::ifstream(path, std::ios::binary))
				{
					shade::render::Image image; shade::Serializer::Deserialize(file, image);
					texture = shade::Texture2D::CreateEXP(shade::render::Image2D::Create(image));
				}
			}
			else
			{
				SHADE_CORE_WARNING("Couldn't open texture file, path ={0}", path);
			}
		};

	static std::function<bool()> AssetLoadCallback;

	static bool IsAssetLoadPopupOpen = false; static float screenPostionY = 0.f;

	auto& model = entity.GetComponent<shade::ModelComponent>();

	std::string search;

	if (ImGui::BeginTable("##SelectModelAsset", 5, ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		{
			ImGuiLayer::DrawFontIcon(u8"\xf1b3", 1, 0.6f);
		}
		ImGui::TableNextColumn();
		{
			if (ImGuiLayer::IconButton("##AddMesh", u8"\xf1b2", 1, 1.0f))
			{
				model->AddMesh(shade::Mesh::CreateEXP());
			}
		}
		ImGui::TableNextColumn();
		{
			std::string buttonTitle = (!model || !model->GetAssetData()) ? "NONE" : model->GetAssetData()->GetId();
			ImGui::BeginDisabled();
			ImGui::Button(buttonTitle.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0.f });
			ImGui::EndDisabled();
		}
		ImGui::TableNextColumn();
		{
			CreateArrowButton("##ModelAssetPopup", IsAssetLoadPopupOpen, screenPostionY, AssetLoadCallback, [&]() { return LoadAsset<shade::Model>(search, model, shade::AssetMeta::Type::Model, shade::AssetMeta::Category::Primary); });
		}
		ImGui::TableNextColumn();
		{
			if (ImGuiLayer::IconButton("##RemoveModel", u8"\xe85d", 1, 1.0f)) { model = shade::Model::CreateEXP(); }
		}
		ImGui::EndTable();
	}

	if (model)
	{
		auto& meshes = model->GetMeshes(); // Сохраните контейнер для удаления

		for (auto it = meshes.begin(); it != meshes.end(); )
		{
			auto& mesh = *it;
			auto& material = mesh->GetMaterial();
			material = !material ? shade::Material::CreateEXP() : material;

			bool isOpen = false;

			if (ImGui::BeginTable("##SelectMeshAssetTable", 6, ImGuiTableFlags_SizingStretchProp, { ImGui::GetContentRegionAvail().x, 0 }))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				{
					ImGuiLayer::DrawFontIcon(u8"\xf1b2", 1, 0.65f);
				}
				ImGui::TableNextColumn();
				{
					if (ImGuiLayer::IconButton("Open", u8"\xe85f", 1, 1.f))
					{
						auto path = shade::FileDialog::OpenFile("Shade mesh(*.s_mesh) \0*.s_mesh\0");
						if (shade::File file = shade::File(path.string(), shade::File::In, "@s_mesh", shade::File::VERSION(0, 0, 1)))
							file.Read(mesh);
						else
							SHADE_CORE_WARNING("Couldn't open mesh file, path ={0}", path);
					}
				}
				ImGui::TableNextColumn();
				{
					if (ImGuiLayer::IconButton("Save", u8"\xe8b9", 1, 1.f))
					{
						auto path = shade::FileDialog::SaveFile("Shade mesh(*.s_mesh) \0*.s_mesh\0");
						if (!path.empty())
						{
							if (shade::File file = shade::File(path.string(), shade::File::Out, "@s_mesh", shade::File::VERSION(0, 0, 1)))
								file.Write(mesh);
							else
								SHADE_CORE_WARNING("Couldn't save mesh file, path ={0}", path);
						}
					}
				}
				ImGui::TableNextColumn();
				{
					ImGui::SetCursorScreenPos({ ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y - ImGui::GetStyle().ItemInnerSpacing.y });
					ImGui::Dummy({ ImGui::GetContentRegionAvail().x - 5.f, 0.f });

					isOpen = ImGui::TreeNodeEx((mesh->GetAssetData()) ? mesh->GetAssetData()->GetId().c_str() : "NONE", ImGuiTreeNodeFlags_Framed);
					if (isOpen) ImGui::TreePop();
				}
				ImGui::TableNextColumn();
				{
					CreateArrowButton("##MeshAssetPopup", IsAssetLoadPopupOpen, screenPostionY, AssetLoadCallback, [&]() { return LoadAsset<shade::Mesh>(search, mesh, shade::AssetMeta::Type::Mesh, shade::AssetMeta::Category::Primary); });
				}

				bool isRemove = false;
				ImGui::TableNextColumn();
				{
					if (ImGuiLayer::IconButton("##RemoveMesh", u8"\xe85d", 1, 1.0f))
					{
						it = meshes.erase(it);
						isRemove = true;
					}
				}
				ImGui::EndTable();

				if (isRemove) break;
			}

			if (isOpen)
			{
				if (ImGui::BeginTable("##SelectMaterialTable", 6, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuterH, { ImGui::GetContentRegionAvail().x, 0 }))
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					{
						ImGuiLayer::DrawFontIcon(u8"\xeaac", 1, 0.6f);
					}
					ImGui::TableNextColumn();
					{
						if (ImGuiLayer::IconButton("Open", u8"\xe85f", 1, 1.f))
						{
							auto path = shade::FileDialog::OpenFile("Shade material(*.s_mat) \0*.s_mat\0");
							if (!path.empty())
							{
								if (shade::File file = shade::File(path.string(), shade::File::In, "@s_mat", shade::File::VERSION(0, 0, 1)))
									file.Read(material);
								else
									SHADE_CORE_WARNING("Couldn't open material file, path ={0}", path);
							}
						}
					}
					ImGui::TableNextColumn();
					{
						if (ImGuiLayer::IconButton("Save", u8"\xe8b9", 1, 1.f))
						{
							auto path = shade::FileDialog::SaveFile("Shade material(*.s_mat) \0*.s_mat\0");
							if (!path.empty())
							{
								if (shade::File file = shade::File(path.string(), shade::File::Out, "@s_mat", shade::File::VERSION(0, 0, 1)))
									file.Write(material);
								else
									SHADE_CORE_WARNING("Couldn't save material file, path ={0}", path);
							}
						}
					}
					ImGui::TableNextColumn();
					{
						ImGui::SetCursorScreenPos({ ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y - ImGui::GetStyle().ItemInnerSpacing.y });
						ImGui::Dummy({ ImGui::GetContentRegionAvail().x , 0.f });

						std::string buttonTitle = (!material || !material->GetAssetData()) ? "NONE" : material->GetAssetData()->GetId();
						ImGui::BeginDisabled();
						ImGui::Button(buttonTitle.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0.f });
						ImGui::EndDisabled();
					}
					ImGui::TableNextColumn();
					{
						CreateArrowButton("##MaterialAssetPopup", IsAssetLoadPopupOpen, screenPostionY, AssetLoadCallback, [&]() { return LoadAsset<shade::Material>(search, material, shade::AssetMeta::Type::Material, shade::AssetMeta::Category::Primary); });
					}
					ImGui::TableNextColumn();
					{
						if (ImGuiLayer::IconButton("##MaterialRemove", u8"\xe85d", 1, 1.0f)) { material = shade::Material::CreateEXP(); }
					}

					HandleTexture(*this, "Albedo", u8"\xe9a5", "Albedo", IsAssetLoadPopupOpen, screenPostionY, search, material->TextureAlbedo, AssetLoadCallback);
					HandleTexture(*this, "Diffuse", u8"\xe9a4", "Diffuse", IsAssetLoadPopupOpen, screenPostionY, search, material->TextureDiffuse, AssetLoadCallback);
					HandleTexture(*this, "Specular", u8"\xea1a", "Specular", IsAssetLoadPopupOpen, screenPostionY, search, material->TextureSpecular, AssetLoadCallback);
					HandleTexture(*this, "Normals", u8"\xe96f", "Normal map", IsAssetLoadPopupOpen, screenPostionY, search, material->TextureNormals, AssetLoadCallback);
					HandleTexture(*this, "Metallic", u8"\xe953", "Metallic", IsAssetLoadPopupOpen, screenPostionY, search, material->TextureMetallic, AssetLoadCallback);
					HandleTexture(*this, "Roughness", u8"\xe978", "Roughness", IsAssetLoadPopupOpen, screenPostionY, search, material->TextureRoughness, AssetLoadCallback);

					ImGui::EndTable();
				}

				if (ImGui::BeginTable("##MaterialAndLods", 2))
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					Material(material.Get());
					ImGui::TableNextColumn();
					{
						const int maxFaces = mesh->GetLod(0).Indices.size() / 3;

						for (int i = 1; i < mesh->GetLods().size(); i++)
						{
							int faces = mesh->GetLod(i).Indices.size() / 3;

							if (DragInt(std::format("Lod level #:{}, faces :", i).c_str(), &faces, 1, 1, maxFaces))
							{
								mesh->RecalculateLod(i, faces);
							}
						}
					}

					ImGui::EndTable();
				}

			}

			it++;
		}
	}

	if (IsAssetLoadPopupOpen)
	{
		ImGuiLayer::BeginWindowOverlay("##AssetLoadOverlay",
			ImGui::GetWindowViewport(),
			std::size_t(this),
			{ ImGui::GetContentRegionAvail().x, 300.f },
			ImVec2{ ImGui::GetWindowPos().x + ImGui::GetStyle().IndentSpacing + 7.f, screenPostionY + 2.f }, 1.0f,
			[&]() mutable
			{
				ImGuiLayer::InputTextCol("Search", search);

				if (ImGui::BeginListBox("##SelectAsset", ImVec2{ ImGui::GetContentRegionAvail() }))
				{
					IsAssetLoadPopupOpen = !AssetLoadCallback();
					ImGui::EndListBox();
				}
			});
	}

	//	for (auto& mesh : model->GetMeshes())
	//	{
	//		const int maxFaces = mesh->GetLod(0).Indices.size() / 3;
	//		static int faces = maxFaces;
	//		static float lambda = 0.1;

	//		if (ImGui::TreeNodeEx(mesh->GetAssetData()->GetId().c_str(), ImGuiTreeNodeFlags_Framed))
	//		{
	//			ImGui::Text("Set globally :"); ImGui::Separator();
	//			if (ImGui::BeginTable(mesh->GetAssetData()->GetId().c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp, { 0, 0 }))
	//			{
	//				ImGui::PushID(mesh->GetAssetData()->GetId().c_str());
	//				ImGui::TableNextColumn();
	//				{
	//				
	//					/*if (DragInt(std::string("Faces" + mesh->GetAssetData()->GetId()).c_str(), &faces, 1, 1, maxFaces))
	//						mesh->RecalculateAllLods(shade::Drawable::MAX_LEVEL_OF_DETAIL, maxFaces, faces, lambda);*/
	//				}
	//				ImGui::TableNextColumn();
	//				{
	//					if (DragFloat(std::string("Split power##" + mesh->GetAssetData()->GetId()).c_str(), &lambda, 0.01, 0.01, 1.0))
	//						mesh->RecalculateAllLods(shade::Drawable::MAX_LEVEL_OF_DETAIL, maxFaces, faces, lambda);
	//				}
	//				ImGui::PopID();
	//				ImGui::EndTable();
	//			}

	//			ImGui::Text("Set manually :"); ImGui::Separator();
	//			for (int i = 1; i < mesh->GetLods().size(); i++)
	//			{
	//				int faces = mesh->GetLod(i).Indices.size() / 3;

	//				if (DragInt(std::format("Lod level #:{}, faces :", i).c_str(), &faces, 1, 1, maxFaces))
	//				{
	//					mesh->RecalculateLod(i, faces);
	//				}
	//			}

	//			if (ImGui::Button("Save"))
	//			{
	//				auto path = mesh->GetAssetData()->GetReference()->GetAttribute<std::string>("Path");

	//				if (!path.empty())
	//				{
	//					shade::File file(path, shade::File::Out, "@s_mesh", shade::File::VERSION(0, 0, 1));
	//					file.Write(mesh);
	//				}
	//				else
	//				{
	//					SHADE_CORE_WARNING("Couldn't save mesh, asset path is empty!");
	//				}

	//			}
	//			ImGui::TreePop();
	//		}

	//		auto& meshId = mesh->GetAssetData()->GetId();


	//		if (DrawButton("Asset", meshId.c_str()))
	//		{
	//			m_SelectedMesh = mesh;
	//			m_SelectedMaterial = mesh->GetMaterial();
	//		}
	//		if (mesh->GetMaterial())
	//		{

	//			if (DrawButton("Material", mesh->GetMaterial()->GetAssetData()->GetId().c_str()))
	//			{
	//				m_SelectedMaterial = mesh->GetMaterial();
	//			}

	//		}

	//	}
	//	ImGui::TreePop();
	//}
}

void EditorLayer::RgidBodyComponent(shade::ecs::Entity& entity)
{
	auto& rigidBody = entity.GetComponent<shade::RigidBodyComponent>();

	std::vector<std::string> colliderType(shade::physic::CollisionShape::Shape::SHAPE_MAX_ENUM);
	for (shade::physic::CollisionShape::Shape type = shade::physic::CollisionShape::Shape::Sphere; type < shade::physic::CollisionShape::Shape::SHAPE_MAX_ENUM; ((std::uint32_t&)type)++)
		colliderType[type] = shade::physic::CollisionShape::GetShapeAsString(type);

	std::string currentShape = "Mesh";

	ImGui::Separator();

	if (entity.HasComponent<shade::ModelComponent>())
	{
		auto& meshes = entity.GetComponent<shade::ModelComponent>()->GetMeshes();

		if (meshes.size() && !rigidBody.GetCollidersCount())
		{
			ImGui::SetNextWindowSize(ImGui::GetContentRegionAvail());
			DrawModal("Sellect collision shape asset", m_IsAddCollisionShapeModal, [&]()
				{
					static std::string id;
					static std::string search;

					InputTextCol("Search:", search);
					ImGui::Separator();

					for (const auto& assetData : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
					{
						if (assetData.second->GetType() == shade::AssetMeta::Type::CollisionShapes && assetData.first.find(search) != std::string::npos)
						{
							if (ImGui::Selectable(assetData.first.c_str(), id == assetData.first))
							{
								shade::AssetManager::GetAsset<shade::physic::CollisionShapes, shade::BaseAsset::InstantiationBehaviour::Synchronous>(assetData.first, shade::AssetMeta::Category::Secondary, shade::BaseAsset::LifeTime::DontKeepAlive, [&](auto& asset) mutable
									{
										rigidBody.AddCollider(asset);
									});

								m_IsAddCollisionShapeModal = false;
							}
						}
					}
				});

			if (ImGui::Button("Select shapes from assets:", { ImGui::GetContentRegionAvail().x, 0 }))
				m_IsAddCollisionShapeModal = true;

			ImGui::Separator();
		}
		else
		{
			if (DrawButton(std::string("Collider's asset: " + rigidBody.GetCollisionShapes()->GetAssetData()->GetId()).c_str(), "Remove"))
				rigidBody.AddCollider(nullptr);
			ImGui::Separator();
		}
	}

	std::vector<std::string> bodyType = { "Static","Dynamic" };

	std::string currentItem = rigidBody.GetBodyType() == shade::RigidBodyComponent::Type::Static ? "Static" : "Dynamic";

	if (ComboCol("BodyType", currentItem, bodyType, ImGuiSelectableFlags_None, ImGuiComboFlags_None, 50))
	{
		if (currentItem == "Static")
			rigidBody.SetBodyType(shade::RigidBodyComponent::Type::Static);
		else
			rigidBody.SetBodyType(shade::RigidBodyComponent::Type::Dynamic);
	}

	glm::vec3 linearVelocity = rigidBody.LinearVelocity;
	glm::vec3 angularVelocity = rigidBody.AngularVelocity;
	float mass = rigidBody.Mass;
	float friction = rigidBody.StaticFriction;
	float restitution = rigidBody.Restitution;


	if (DragFloat3("Linear velocity", glm::value_ptr(linearVelocity), 0.0, 0.001, -FLT_MAX, FLT_MAX))
		rigidBody.LinearVelocity = linearVelocity;

	if (DragFloat3("Angular velocity", glm::value_ptr(angularVelocity), 0.0, 0.001, -FLT_MAX, FLT_MAX))
		rigidBody.AngularVelocity = angularVelocity;

	if (DragFloat("Mass", &mass)) rigidBody.Mass = mass;

	if (DragFloat("Friction", &friction)) rigidBody.StaticFriction = friction;

	if (DragFloat("Restitution", &restitution)) rigidBody.Restitution = restitution;

}

void EditorLayer::AnimationGraphComponent(shade::ecs::Entity& entity)
{
	auto& graph = entity.GetComponent<shade::AnimationGraphComponent>();

	if (!m_GraphEditor.GetRootGraph() && graph.AnimationGraph)
	{
		m_GraphEditor.Initialize(graph.AnimationGraph);
	}
	else if (m_GraphEditor.GetRootGraph() != graph.AnimationGraph.Raw())
	{
		m_GraphEditor.Initialize(graph.AnimationGraph);
	}

	static std::string search; static bool animGraphAssetPopop = false, skeletonAssetPopop = false;

	ImVec2 graphArrowButtonSP, skeletonArrowButtonSP;

	if (ImGui::BeginTable("##SelectAnimationGraphTable", 4, ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		{
			ImGuiLayer::DrawFontIcon(u8"\xe984", 1, 0.6f);
		}
		ImGui::TableNextColumn();
		{
			if (ImGuiLayer::IconButton("Open", u8"\xe85f", 1, 1.f))
			{
				auto path = shade::FileDialog::OpenFile("Shade graph(*.graph) \0*.graph\0");

				if (!path.empty())
				{
					if (shade::File file = shade::File(path.string(), shade::File::In, "@s_animgraph", shade::File::VERSION(0, 0, 1)))
					{
						graph.GraphContext.Drop();
						graph.AnimationGraph = shade::SharedPointer<shade::animation::AnimationGraph>::Create(&graph.GraphContext); file.Read(graph.AnimationGraph); m_GraphEditor.Initialize(graph.AnimationGraph);
					}
					else
					{
						SHADE_CORE_WARNING("Couldn't open graph file, path ={0}", path);
					}
				}
			}
			ImGui::SameLine();
			if (ImGuiLayer::IconButton("Save", u8"\xe8b9", 1, 1.f))
			{
				auto path = shade::FileDialog::SaveFile("Shade graph(*.graph) \0*.graph\0");

				if (!path.empty())
				{
					if (shade::File file = shade::File(path.string(), shade::File::Out, "@s_animgraph", shade::File::VERSION(0, 0, 1)))
					{
						file.Write(graph.AnimationGraph);
					}
					else
					{
						SHADE_CORE_WARNING("Couldn't open graph file, path ={0}", path);
					}
				}
			}
		}
		ImGui::TableNextColumn();
		{
			std::string buttonTitle = (!graph.AnimationGraph || !graph.AnimationGraph->GetAssetData()) ? "Not set" : graph.AnimationGraph->GetAssetData()->GetId();
			ImGui::BeginDisabled();
			ImGui::Button(buttonTitle.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0.f });
			ImGui::EndDisabled();
		}
		ImGui::TableNextColumn();
		{
			if (ImGui::ArrowButton("##AnimGraphOpenPopup", ImGuiDir_Down))
			{
				animGraphAssetPopop = !animGraphAssetPopop;
				if (animGraphAssetPopop) skeletonAssetPopop = false;
			}

			graphArrowButtonSP = ImGui::GetCursorScreenPos();
		}
		ImGui::EndTable();
	}

	if (ImGui::BeginTable("##SelectSkeletonTable", 4, ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		{
			shade::ImGuiLayer::DrawFontIcon(u8"\xf29a", 1, 0.6f);
		}
		ImGui::TableNextColumn();
		{
			if (ImGuiLayer::IconButton("Open", u8"\xe85f", 1, 1.f))
			{
				auto path = shade::FileDialog::OpenFile("Shade skeleton(*.s_skel) \0*.s_skel\0");

				if (!path.empty())
				{
					if (shade::File file = shade::File(path.string(), shade::File::In, "@s_skel", shade::File::VERSION(0, 0, 1)))
					{
						graph.GraphContext.Skeleton = shade::Skeleton::CreateEXP();
						file.Read(graph.GraphContext.Skeleton);
					}
					else
					{
						SHADE_CORE_WARNING("Couldn't open skeleton file, path ={0}", path);
					}
				}
			}
		}
		ImGui::TableNextColumn();
		{
			std::string buttonTitle = (!graph.GraphContext.Skeleton || !graph.GraphContext.Skeleton->GetAssetData()) ? "Not set" : graph.GraphContext.Skeleton->GetAssetData()->GetId();
			ImGui::BeginDisabled();
			ImGui::Button(buttonTitle.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0.f });
			ImGui::EndDisabled();
		}
		ImGui::TableNextColumn();
		{
			if (ImGui::ArrowButton("##SeletonOpenPopup", ImGuiDir_Down))
			{
				skeletonAssetPopop = !skeletonAssetPopop;
				if (skeletonAssetPopop) animGraphAssetPopop = false;
			}

			skeletonArrowButtonSP = ImGui::GetCursorScreenPos();
		}
		ImGui::EndTable();
	}

	if (animGraphAssetPopop)
	{
		ImGuiLayer::BeginWindowOverlay("##AnimationGraphAssetOverlay",
			ImGui::GetWindowViewport(),
			entity.GetID(),
			ImGui::GetContentRegionAvail(),
			ImVec2{ ImGui::GetWindowPos().x + ImGui::GetStyle().IndentSpacing + 7.f, graphArrowButtonSP.y + 2.f }, 1.0f,
			[&]() mutable
			{
				ImGuiLayer::InputTextCol("Search", search);

				if (ImGui::BeginListBox("##SelectAnimationGraph", ImVec2{ ImGui::GetContentRegionAvail() }))
				{
					for (const auto& assetData : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
					{
						if (assetData.second->GetType() == shade::AssetMeta::Type::AnimationGraph && assetData.first.find(search) != std::string::npos)
						{
							if (ImGui::Selectable(assetData.first.c_str(), false))
							{
								//shade::AssetManager::GetAsset<shade::animation::AnimationGraph,
								//	shade::BaseAsset::InstantiationBehaviour::Synchronous>(assetData.first,
								//		shade::AssetMeta::Category::Secondary,
								//		shade::BaseAsset::LifeTime::KeepAlive,
								//		[&](auto& animation) mutable
								//		{
								//			//node.ResetAnimationData(animation);
								//		});

								animGraphAssetPopop = false;
							}

						}
					}
					ImGui::EndListBox();
				}

			});
	}

	if (skeletonAssetPopop)
	{
		ImGuiLayer::BeginWindowOverlay("##SkeletonAssetOverlay",
			ImGui::GetWindowViewport(),
			entity.GetID(),
			ImGui::GetContentRegionAvail(),
			ImVec2{ ImGui::GetWindowPos().x + ImGui::GetStyle().IndentSpacing + 7.f, skeletonArrowButtonSP.y + 2.f }, 1.0f,
			[&]() mutable
			{
				ImGuiLayer::InputTextCol("Search", search);

				if (ImGui::BeginListBox("##SelectSkeleton", ImVec2{ ImGui::GetContentRegionAvail() }))
				{
					for (const auto& assetData : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
					{
						if (assetData.second->GetType() == shade::AssetMeta::Type::Skeleton && assetData.first.find(search) != std::string::npos)
						{
							if (ImGui::Selectable(assetData.first.c_str(), false))
							{
								shade::AssetManager::GetAsset<shade::Skeleton,
									shade::BaseAsset::InstantiationBehaviour::Synchronous>(assetData.first,
										shade::AssetMeta::Category::Secondary,
										shade::BaseAsset::LifeTime::KeepAlive,
										[&](auto& skeleton) mutable
										{
											graph.GraphContext.Skeleton = skeleton;
										});

								skeletonAssetPopop = false;
							}

						}
					}
					ImGui::EndListBox();
				}

			});
	}

	m_GraphEditor.Edit("Graph editor", { 500, 500 });
}

void EditorLayer::NativeScriptComponent(shade::ecs::Entity& entity)
{
	auto& script = entity.GetComponent<shade::NativeScriptComponent>();

	static std::string moduleName = (script.GetIsntace()) ? script.GetIsntace()->GetModuleName() : "", functionName = (script.GetIsntace()) ? script.GetIsntace()->GetName() : "",
		moduleSearch, functionSearch;

	ImGui::BeginChild("##ScriptModuels", ImGui::GetContentRegionAvail() / 2.f, true);
	ImGuiLayer::InputTextCol("Modules:", moduleSearch);
	ImGui::Separator();

	for (auto& [name, lib] : shade::scripts::ScriptManager::GetLibraries())
	{
		if (name.find(moduleSearch) != std::string::npos)
		{
			if (ImGui::Selectable(name.c_str(), moduleName == name))
				moduleName = name;
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::BeginChild("##ScriptFunctions", { 0.f, ImGui::GetContentRegionAvail().y / 2.f }, true);
	ImGuiLayer::InputTextCol("Functions:", functionSearch);
	ImGui::Separator();
	if (shade::scripts::ScriptManager::GetLibraries().find(moduleName) != shade::scripts::ScriptManager::GetLibraries().end())
	{
		for (auto& [name, function] : shade::scripts::ScriptManager::GetLibraries().at(moduleName)->GetExportedFunctions())
		{
			if (name.find(functionSearch) != std::string::npos)
			{
				if (ImGui::Selectable(name.c_str(), functionName == name))
				{
					functionName = name;

					if (auto instance = shade::scripts::ScriptManager::InstantiateScript<shade::ecs::ScriptableEntity*>(moduleName, functionName))
					{
						script.Bind(instance);
					}
				}
			}
		}
	}
	ImGui::EndChild();
	ImGui::EndGroup();
}

void EditorLayer::MaterialEdit(shade::Material& material)
{
	static std::string path;

	if (ImGui::BeginTable("##SomeTable", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn(); { if (ImGui::Button("Select default")) { m_SelectedMaterial = nullptr; path.clear(); } }
			ImGui::TableNextColumn();
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				InputTextD("##MaterialPath", path);
			}
			ImGui::TableNextColumn();
			{
				if (ImGui::Button("..."))
				{
					path = shade::FileDialog::OpenFile("Shade material(*.s_mat) \0*.s_mat\0").string();
					if (!path.empty())
					{
						shade::File file(path, shade::File::In, "@s_mat", shade::File::VERSION(0, 0, 1));
						if (file.IsOpen())
						{
							m_SelectedMaterial = shade::SharedPointer<shade::Material>::Create();
							file.Read(m_SelectedMaterial);
						}
					}
				}
			}
		}
		ImGui::EndTable();
	}

	Material(material);

	if (&material != shade::Renderer::GetDefaultMaterial().Raw())
	{
		if (ImGui::Button("Save", { ImGui::GetContentRegionAvail().x, 0 }))
		{
			shade::File file(path, shade::File::Out, "@s_mat", shade::File::VERSION(0, 0, 1));
			if (file.IsOpen())
			{
				file.Write(material);
			}
		}
	}
}

void EditorLayer::BoneMaskEdnitor(const shade::Skeleton::BoneNode* node, shade::animation::BoneMask& boneMask)
{
	bool overrideByParrent = false;

	if (ImGui::TreeNodeEx(node->Name.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed))
	{
		if (ImGui::BeginTable("##BoneMask", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					ImGui::Text("Weight");
				}
				ImGui::TableNextColumn();
				{
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::DragFloat(std::string("##" + node->Name).c_str(), &boneMask.Weights.at(node->ID).second, 0.01f, 0.f, 1.f);
				}
				ImGui::TableNextColumn();
				{
					ImGui::Checkbox("P##", &overrideByParrent);
				}
			}
			ImGui::EndTable();
		}

		for (auto& child : node->Children)
			BoneMaskEdnitor(child, boneMask);

		ImGui::TreePop();
	}
}

void EditorLayer::Material(shade::Material& material)
{
	if (ImGui::BeginTable("##MaterailTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn(); { ImGui::Text("Ambient"); }
			ImGui::TableNextColumn();
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit3("##Ambient", glm::value_ptr(material.ColorAmbient));
			}
		}
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn(); { ImGui::Text("Diffuse"); }
			ImGui::TableNextColumn();
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit3("##Diffuse", glm::value_ptr(material.ColorDiffuse));
			}
		}
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn(); { ImGui::Text("Specular"); }
			ImGui::TableNextColumn();
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit3("##Specular", glm::value_ptr(material.ColorSpecular));
			}
		}
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn(); { ImGui::Text("Emmisive"); }
			ImGui::TableNextColumn();
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##Emmisive", &material.Emmisive, 0.01f, 0);
			}
		}
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn(); { ImGui::Text("Opacity"); }
			ImGui::TableNextColumn();
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##Opacity", &material.Opacity, 0.01f, 0);
			}
		}
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn(); { ImGui::Text("Shininess, Strength"); }
			ImGui::TableNextColumn();
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 2.f);
				ImGui::DragFloat("##Shininess", &material.Shininess, 0.01f, 0); ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##Shininess Strength", &material.ShininessStrength, 0.01f, 0);
			}
		}
		/*ImGui::TableNextRow();
		{
			ImGui::TableNextColumn(); { ImGui::Text("Shininess Strength"); }
			ImGui::TableNextColumn();
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##Shininess Strength", &material.ShininessStrength, 0.01f, 0);
			}
		}*/
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn(); { ImGui::Text("Refracti"); }
			ImGui::TableNextColumn();
			{
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##Refracti", &material.Refracti, 0.01f, 0);
			}
		}
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn(); { ImGui::Text("Normal map"); }
			ImGui::TableNextColumn(); { ImGui::Checkbox("##NormalMap", &material.NormalMapEnabled); }
		}
		ImGui::TableNextRow();
		{
			ImGui::TableNextColumn(); { ImGui::Text("Bump map"); }
			ImGui::TableNextColumn(); { ImGui::Checkbox("##BumpMap", &material.BumpMapEnabled); }
		}

		ImGui::EndTable();
	}

}

void EditorLayer::AnimationSequencer()
{
	//// sequence with default values
	//ImSequencer::MySequence mySequence;
	//mySequence.mFrameMin = -100;
	//mySequence.mFrameMax = 1000;
	//mySequence.myItems.push_back(MySequence::MySequenceItem{ 0, 10, 30, false });
	//mySequence.myItems.push_back(MySequence::MySequenceItem{ 1, 20, 30, true });
	//mySequence.myItems.push_back(MySequence::MySequenceItem{ 3, 12, 60, false });
	//mySequence.myItems.push_back(MySequence::MySequenceItem{ 2, 61, 90, false });
	//mySequence.myItems.push_back(MySequence::MySequenceItem{ 4, 90, 99, false });

	//// let's create the sequencer
	//static int selectedEntry = -1;
	//static int firstFrame = 0;
	//static bool expanded = true;
	//static int currentFrame = 100;

	//ImGui::PushItemWidth(130);
	//ImGui::InputInt("Frame Min", &mySequence.mFrameMin);
	//ImGui::SameLine();
	//ImGui::InputInt("Frame ", &currentFrame);
	//ImGui::SameLine();
	//ImGui::InputInt("Frame Max", &mySequence.mFrameMax);
	//ImGui::PopItemWidth();
	//Sequencer(&mySequence, &currentFrame, &expanded, &selectedEntry, &firstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
	//// add a UI to edit that particular item
	//if (selectedEntry != -1)
	//{
	//	const MySequence::MySequenceItem& item = mySequence.myItems[selectedEntry];
	//	ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
	//	// switch (type) ....
	//}
}

void EditorLayer::CreateMaterial()
{
	static std::string path;
	static shade::SharedPointer<shade::Material> material = shade::SharedPointer<shade::Material>::Create();

	Material(*material);

	if (ImGui::BeginTable("Path", 2, ImGuiTableFlags_SizingStretchProp, { ImGui::GetContentRegionAvail().x, 0 }))
	{
		ImGui::TableNextColumn();
		{
			InputTextCol("Path", path);
		}
		ImGui::TableNextColumn();
		{
			if (ImGui::Button("..."))
			{
				path = shade::FileDialog::SaveFile("Shade material(*.s_mat) \0*.s_mat\0").string();
			}
		}
		ImGui::EndTable();
	}

	if (!path.empty())
	{
		if (ImGui::Button("Save", { ImGui::GetContentRegionAvail().x, 0 }))
		{
			shade::File file(path, shade::File::Out, "@s_mat", shade::File::VERSION(0, 0, 1));
			if (file.IsOpen())
			{
				file.Write(material);
				file.CloseFile();
				path.clear();
			}
		}
	}
}

void EditorLayer::CreateCollisionShapes()
{
	static std::string search;
	InputTextCol("Search:", search);
	static shade::SharedPointer<shade::AssetData> selectedAssetData;

	ImGui::Separator();
	for (auto& [id, asset] : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Primary))
	{
		if (asset->GetType() == shade::AssetMeta::Type::Model)
		{
			if (id.find(search) != std::string::npos)
			{
				if (ImGui::Selectable(id.c_str(), selectedAssetData == asset))
					selectedAssetData = asset;
			}
		}
	}
	ImGui::Separator();

	static std::string path;

	ImGui::SetNextWindowSize(ImGui::GetContentRegionAvail());
	DrawModal("Create new asset", m_IsCreateNewRawAssetModalOpen, [&]()
		{
			if (ImGui::BeginTable("Path", 2, ImGuiTableFlags_SizingStretchProp, { ImGui::GetContentRegionAvail().x, 0 }))
			{
				ImGui::TableNextColumn();
				{
					InputTextCol("Path", path);
				}
				ImGui::TableNextColumn();
				{
					if (ImGui::Button("Select"))
					{
						auto selectedPath = shade::FileDialog::SaveFile("");
						if (!selectedPath.empty())
							path = selectedPath.string();
					}
				}
				ImGui::EndTable();
			}

			if (ImGui::Button("Save", { ImGui::GetContentRegionAvail().x, 0 }))
			{
				if (!path.empty())
				{
					static auto shapes = shade::physic::CollisionShapes::CreateEXP();

					shade::AssetManager::GetAsset<shade::Model, shade::BaseAsset::InstantiationBehaviour::Synchronous>(selectedAssetData->GetId(), shade::AssetMeta::Category::Primary, shade::BaseAsset::LifeTime::DontKeepAlive, [&](auto& asset) mutable
						{
							for (const auto& mesh : *shade::Asset<shade::Model>(asset))
							{
								auto collider = shade::SharedPointer<shade::physic::MeshShape>::Create();
								collider->SetMinMaxHalfExt(mesh->GetMinHalfExt(), mesh->GetMaxHalfExt());

								auto lod = mesh->GetLod(9);

								auto [hull, indices] = shade::physic::algo::GenerateConvexHull(lod.Vertices, 0.0001);

								for (auto& point : hull)
									collider->AddVertex(point);

								shapes->AddShape(collider);
							}
						});


					shade::File file(path.c_str(), shade::File::Out, "@s_c_shape", shade::File::VERSION(0, 0, 1));
					if (file.IsOpen())
					{
						file.Write(shapes.Get());
						file.CloseFile();
					}
					else
					{
						SHADE_CORE_WARNING("Couldn't save convex shapes, path ={0}", path);
					}

					m_IsCreateNewRawAssetModalOpen = false;
				}
			}

		});

	if (ImGui::Button("Create convex shapes", { ImGui::GetContentRegionAvail().x, 0 }))
		m_IsCreateNewRawAssetModalOpen = true;
}
