#include "shade_pch.h"
#include "EditorAnimGraph.h"

void editor_animation_graph::PoseNodeDeligate::ProcessSideBar()
{
	PoseNode& node = GetNode()->As<animation::PoseNode>();
	float& start = node.GetAnimationData().Start;
	float& end = node.GetAnimationData().End;
	float& duration = node.GetAnimationData().Duration;

	ImGui::Text("Node: Pose"); ImGui::Separator();

	if (ImGui::BeginChildEx("Node: Pose", std::size_t(&node), ImGui::GetContentRegionAvail(), true, 0))
	{
		if (ImGui::BeginTable("Table", 2, ImGuiTableFlags_SizingStretchProp))
		{
			(!node.GetAnimationData().Animation) ? ImGui::BeginDisabled() : void();
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					ImGui::Text("Time line");
				}
				ImGui::TableNextColumn();
				{
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::SliderFloat("##Timeline", &node.GetAnimationData().CurrentPlayTime, start, end);
					ImGui::PopItemWidth();
				}
			}
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					ImGui::Text("Start >-< End");
				}
				ImGui::TableNextColumn();
				{
					glm::vec2 startEnd = { start , end };
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
					if (ImGui::DragFloat2("##StartEnd", glm::value_ptr(startEnd), 0.01f, 0.0f, duration))
					{
						start = startEnd.x; end = startEnd.y;
					}
					ImGui::PopItemWidth();
				}
			}
			(!node.GetAnimationData().Animation) ? ImGui::EndDisabled() : void();
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					ImGui::Text("Animation clip");
				}
				ImGui::TableNextColumn();
				{
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
					std::string buttonTitle = (!node.GetAnimationData().Animation) ? "Select" : node.GetAnimationData().Animation->GetAssetData()->GetId();

					if (ImGui::Button(buttonTitle.c_str(), ImVec2{ImGui::GetContentRegionAvail().x, 0.f}))
						m_IsAnimationPopupActive = (!m_IsAnimationPopupActive) ? true : false;
					ImGui::PopItemWidth();

					if (m_IsAnimationPopupActive)
					{
						ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);
						ImGui::SetNextWindowBgAlpha(0.5f); // Transparent background
						ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos(), ImGuiCond_Always);
						ImGui::SetNextWindowSize(ImVec2{ ImGui::GetContentRegionAvail().x, 0.f }, ImGuiCond_Always);

						if (ImGui::Begin("SearchOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
							ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_Tooltip))
						{

							ImGuiLayer::InputTextCol("Search", m_Search);
							
							if (ImGui::BeginListBox("##SelectAnimation", ImVec2{ ImGui::GetContentRegionAvail().x, 0.f }))
							{
								for (const auto& assetData : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
								{
									if (assetData.second->GetType() == shade::AssetMeta::Type::Animation && assetData.first.find(m_Search) != std::string::npos)
									{
										SHADE_CORE_INFO("{}", assetData.first);

										if (ImGui::Selectable(assetData.first.c_str(), false))
										{
											shade::AssetManager::GetAsset<shade::Animation,
												shade::BaseAsset::InstantiationBehaviour::Synchronous>(assetData.first, 
												shade::AssetMeta::Category::Secondary, 
												shade::BaseAsset::LifeTime::KeepAlive, 
												[&](auto& animation) mutable
												{
													node.ResetAnimationData(animation);
												});

											m_IsAnimationPopupActive = false;
										}

									}
								}
								ImGui::EndListBox();
							}

						} ImGui::End();
					}

				}
			}

			ImGui::EndTable();
		}

		//ImGui::PopItemWidth();



		//ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		//ImGui::BeginGroup();
		//(!node.GetAnimationData().Animation) ? ImGui::BeginDisabled() : void();
		//{
		//	ImGui::Text("Time Line"); ImGui::SameLine();
		//	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		//	ImGui::SliderFloat("##TimeLine", &node.GetAnimationData().CurrentPlayTime, start, end);
		//	ImGui::PopItemWidth();

		//	glm::vec2 startEnd = { start , end };
		//	if (ImGui::DragFloat2("##StartEnd", glm::value_ptr(startEnd), 0.01f, 0.0f, duration))
		//	{
		//		start = startEnd.x; end = startEnd.y;
		//	}
		//}
		//(!node.GetAnimationData().Animation) ? ImGui::EndDisabled() : void();

		//if (!node.GetAnimationData().Animation)
		//{
		//	ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);
		//	ImGui::SetNextWindowBgAlpha(0.5f); // Transparent background
		//	ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos(), ImGuiCond_Always);
		//	ImGui::SetNextWindowSize(ImVec2{ ImGui::GetContentRegionAvail().x, 0.f }, ImGuiCond_Always);

		//	if (ImGui::Begin("SearchOverlay", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
		//		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_Tooltip))
		//	{
		//		if (ImGui::BeginListBox("##SelectAnimation", ImVec2{ ImGui::GetContentRegionAvail().x, 0.f }))
		//		{
		//			if (ImGui::Selectable("items[n]", false))
		//			{
		//				// Select Animation
		//				ImGui::Text("Time Line"); ImGui::SameLine();
		//			}

		//			ImGui::EndListBox();
		//		}

		//		
		//	} ImGui::End();
		//	
		//}
		//ImGui::EndGroup();
		//ImGui::PopItemWidth();

		ImGui::EndChild();
	}

	//std::vector<std::string> items;

	//for (const auto& assetData : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
	//{
	//	if (assetData.second->GetType() == shade::AssetMeta::Type::Animation)
	//	{
	//		items.emplace_back(assetData.first);
	//	}
	//}

	//std::string currentItem = "";

	//if (node.GetAnimationData().Animation)
	//{
	//	currentItem = node.GetAnimationData().Animation->GetAssetData()->GetId();
	//}

	//ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	//if (ImGui::BeginCombo("##", currentItem.c_str(), 0)) // The second parameter is the label previewed before opening the combo.
	//{
	//	for (auto& element : items)
	//	{
	//		bool isSelected = (currentItem == element);
	//		if (ImGui::Selectable(element.c_str(), isSelected, 0))
	//		{
	//			currentItem = element;
	//		}
	//		if (isSelected)
	//			ImGui::SetItemDefaultFocus();
	//	}
	//	ImGui::EndCombo();

	//	if (currentItem.size())
	//		shade::AssetManager::GetAsset<shade::Animation, shade::BaseAsset::InstantiationBehaviour::Synchronous>(currentItem, shade::AssetMeta::Category::Secondary, shade::BaseAsset::LifeTime::KeepAlive, [&](auto& animation) mutable
	//			{
	//				node.ResetAnimationData(animation);
	//			});

	//}

	//ImGui::PopItemWidth();
}