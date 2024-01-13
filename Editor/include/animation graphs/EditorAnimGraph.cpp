#include "shade_pch.h"
#include "EditorAnimGraph.h"


editor_animation_graph::OutputPoseNodeDeligate::OutputPoseNodeDeligate(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
	BaseNodeDeligate(id, node)
{
	Style.Title = "Result";
	Style.HeaderColor = ImVec4{ 0.7, 0.7, 0.7, 1.0 };
	Style.Size = ImVec2{ 120.f, 100.f };
}

void editor_animation_graph::OutputPoseNodeDeligate::ProcessEndpoint(const GraphNode::EndpointIDX& endpointIdentifier, EndpointPrototype& endpoint)
{

	auto& node = GetNode()->As<OutputPoseNode>();
	const EndpointPrototype::EndpointType type = endpoint.GetType();

	switch (endpoint.GetType()) // TODO: Rename -  GetConenctionType
	{
		case EndpointPrototype::Input:
		{
			endpoint.Style.Color = ImVec4{ 0.5f, 0.7f, 0.f, 1.f };
			endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };

			std::size_t hash = 0;
			auto pose = node.GetEndpoint<animation::GraphNode::Connection::Input>(endpointIdentifier)->As<NodeValueType::Pose>();
			if (pose) hash = pose->GetAnimationHash();

			ImGui::Text("Pose");
			ImGuiLayer::HelpMarker("#", std::format("{:x}", hash).c_str());
		}
	}
}

void editor_animation_graph::OutputPoseNodeDeligate::ProcessBodyContent()
{

}

editor_animation_graph::PoseNodeDeligate::PoseNodeDeligate(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
	BaseNodeDeligate(id, node)
{
	Style.Title = "Pose";
}

void editor_animation_graph::PoseNodeDeligate::ProcessSideBar()
{
	PoseNode& node = GetNode()->As<animation::PoseNode>();

	float& start = node.GetAnimationData().Start;
	float& end = node.GetAnimationData().End;
	float& duration = node.GetAnimationData().Duration;
	float& ticksPerSecond = node.GetAnimationData().TicksPerSecond;

	ImGui::Text("Node: Pose"); ImGui::Separator();

	if (ImGui::BeginChildEx("Node: Pose", std::size_t(&node), ImGui::GetContentRegionAvail(), true, 0))
	{
		const ImVec2 windowPosition = ImGui::GetWindowPos();
		const ImVec2 windowSize = ImGui::GetWindowSize();

		if (ImGui::BeginTable("Table", 2, ImGuiTableFlags_SizingStretchProp))
		{
			(!node.GetAnimationData().Animation) ? ImGui::BeginDisabled() : void();
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					shade::ImGuiLayer::DrawFontIcon(u8"\xe889", 1, 0.5f);
				}
				ImGui::TableNextColumn();
				{
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::SliderFloat("##Timeline", &node.GetAnimationData().CurrentPlayTime, 0.f, duration);
					ImGui::PopItemWidth();
				}
			}
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					shade::ImGuiLayer::DrawFontIcon(u8"\xe84c", 1, 0.5f); ImGui::SameLine(); ImGui::Text("/"); ImGui::SameLine(); shade::ImGuiLayer::DrawFontIcon(u8"\xf11e", 1, 0.5f);
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
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					shade::ImGuiLayer::DrawFontIcon(u8"\xea8b", 1, 0.5f);
				}
				ImGui::TableNextColumn();
				{
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::DragFloat("##Tiks", &ticksPerSecond, 0.01f, 0.0f);
					ImGui::PopItemWidth();
				}
			}
			(!node.GetAnimationData().Animation) ? ImGui::EndDisabled() : void();
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					shade::ImGuiLayer::DrawFontIcon(u8"\xe823", 1, 0.5f);
				}
				ImGui::TableNextColumn();
				{
					if (ImGui::BeginTable("##SelectAnimationTable", 2, ImGuiTableFlags_SizingStretchProp))
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						{
							std::string buttonTitle = (!node.GetAnimationData().Animation) ? "Not set" : node.GetAnimationData().Animation->GetAssetData()->GetId();
							ImGui::BeginDisabled();
							ImGui::Button(buttonTitle.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0.f });
							ImGui::EndDisabled();
						}
						ImGui::TableNextColumn();
						{
							if (ImGui::ArrowButton("##OpenPopup", ImGuiDir_Down))
							{
								m_IsAnimationPopupActive = (!m_IsAnimationPopupActive) ? true : false;
							}
						}
						ImGui::EndTable();
					}

					if (m_IsAnimationPopupActive)
					{
						ImGuiLayer::BeginWindowOverlay("##AnimationSearchOverlay", ImGui::GetWindowViewport(), std::size_t(&node), ImVec2{ windowSize.x - 10.f, 0.f }, ImVec2{ windowPosition.x + 5.f, ImGui::GetCursorScreenPos().y + 5.f }, 0.3f,
							[&]() mutable
							{
								ImGuiLayer::InputTextCol("Search", m_Search);

								if (ImGui::BeginListBox("##SelectAnimation", ImVec2{ ImGui::GetContentRegionAvail().x, 0.f }))
								{
									for (const auto& assetData : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
									{
										if (assetData.second->GetType() == shade::AssetMeta::Type::Animation && assetData.first.find(m_Search) != std::string::npos)
										{
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

							});
					}
				}
			}

			ImGui::EndTable();
		}
		ImGui::EndChild();
	}
}

void editor_animation_graph::PoseNodeDeligate::ProcessEndpoint(const GraphNode::EndpointIDX& endpointIdentifier, EndpointPrototype& endpoint)
{
	auto& node = GetNode()->As<PoseNode>();
	const EndpointPrototype::EndpointType type = endpoint.GetType();

	switch (endpoint.GetType()) // TODO: Rename -  GetConenctionType
	{
	case EndpointPrototype::Output:
	{
		endpoint.Style.Color = Style.HeaderColor;
		endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };

		ImGui::Text("Pose");
	}
	}
}

void editor_animation_graph::PoseNodeDeligate::ProcessBodyContent()
{
	PoseNode& node = GetNode()->As<animation::PoseNode>();
	float& start = node.GetAnimationData().Start;
	float& end = node.GetAnimationData().End;
	float& duration = node.GetAnimationData().Duration;
	float& currentTime = node.GetAnimationData().CurrentPlayTime;

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	
	{

		// Calculate the normalized value between 0 and 1
		float result = (currentTime - start) / (end - start);
		// Clamp the result to make sure it stays between 0 and 1
		result = glm::clamp(result, 0.0f, 1.0f);

		ImGui::ProgressBar(result, ImVec2(0.f, 0.f), std::format("{:.1f}", currentTime).c_str());
	}

	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::Spacing();

	std::string animation = (!node.GetAnimationData().Animation) ? "  Not select" : node.GetAnimationData().Animation->GetAssetData()->GetId();
	shade::ImGuiLayer::DrawFontIcon(u8"\xe823", 1, 0.5f); 
	ImGui::SameLine(); ImGui::Text(animation.c_str());

}

editor_animation_graph::BlendNodeDeligate::BlendNodeDeligate(GraphNode::NodeIDX id, SharedPointer<GraphNode> node):
	BaseNodeDeligate(id, node)
{
	Style.Title = "Blend";
}

void editor_animation_graph::BlendNodeDeligate::ProcessBodyContent()
{
	
}

void editor_animation_graph::BlendNodeDeligate::ProcessEndpoint(const GraphNode::EndpointIDX& endpointIdentifier, EndpointPrototype& endpoint)
{
	auto& node = GetNode()->As<BlendNode2D>();
	const EndpointPrototype::EndpointType type = endpoint.GetType();

	switch (type)
	{
	case EndpointPrototype::Input:
	{
		if (endpointIdentifier == 0) // Blend Weight
		{
			ImGui::Text("Blend Weight");
			endpoint.Style.Color = ImVec4{ 0.3, 0.5, 1.0, 1.0 };
			endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };
		}
		if (endpointIdentifier == 1)
		{
			endpoint.Style.Color = ImVec4{ 0.6, 0.4, 0.0, 1.0 };
			endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };
			ImGui::Text("Bone Mask");
		}
		if (endpointIdentifier > 1)
		{
			endpoint.Style.Color = Style.HeaderColor;
			endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };

			 
			std::size_t hash = 0;
			auto pose = node.GetEndpoint<animation::GraphNode::Connection::Input>(endpointIdentifier)->As<NodeValueType::Pose>();
			if (pose) hash = pose->GetAnimationHash();

			ImGui::Text("Pose");  ImGui::SameLine();
			ImGuiLayer::HelpMarker("#", std::format("{:x}", hash).c_str());
		}
		break;
	}
	case EndpointPrototype::Output:
	{
		endpoint.Style.Color = Style.HeaderColor;
		endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };

		ImGui::Text("Pose"); break;
	}
	}
}

editor_animation_graph::BoneMaskNodeDeligate::BoneMaskNodeDeligate(GraphNode::NodeIDX id, SharedPointer<GraphNode> node)
	:
	BaseNodeDeligate(id, node)
{
	Style.Title = "Bone Mask";
	Style.HeaderColor = ImVec4{ 0.6, 0.4, 0.0, 1.0 };
	Style.Size = ImVec2{ 150, 100 };
}

void editor_animation_graph::BoneMaskNodeDeligate::ProcessSideBar()
{
	BoneMaskNode& node = GetNode()->As<animation::BoneMaskNode>();
	ImGui::Text("Node: Bone Mask");

	if (ImGui::BeginChildEx("Node: Bone Mask", std::size_t(&node), ImGui::GetContentRegionAvail(), true, 0))
	{
		ImGuiLayer::InputTextCol("Search", m_Search);

		if (ImGui::BeginTable("##BoneMaskTable", 2, ImGuiTableFlags_SizingFixedFit))
		{
			for (auto& [key, value] : node.GetBoneMask().Weights)
			{
				if (value.first.find(m_Search) != std::string::npos)
				{
					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();

						shade::ImGuiLayer::DrawFontIcon(u8"\xf2d8", 1, 0.6f); ImGui::SameLine(); //ImGui::Text(value.first.c_str());

						ImGui::Text(value.first.c_str());

						ImGui::TableNextColumn();
						ImGui::PushID(key);
						ImGui::PushItemWidth(50.f);
						ImGui::DragFloat("##", &value.second, 0.001f, 0.f, 1.f);
						ImGui::PopItemWidth();
						ImGui::PopID();
					}
				}
			}

			ImGui::EndTable();
		}
		ImGui::EndChild();
	}

}

void editor_animation_graph::BoneMaskNodeDeligate::ProcessBodyContent()
{
	auto& node = GetNode()->As<BoneMaskNode>();

	shade::ImGuiLayer::DrawFontIcon(u8"\xf29a", 1, 0.6f); ImGui::SameLine(); ImGui::Text(node.GetGraphContext().Skeleton->GetAssetData()->GetId().c_str());
}

void editor_animation_graph::BoneMaskNodeDeligate::ProcessEndpoint(const GraphNode::EndpointIDX& endpointIdentifier, EndpointPrototype& endpoint)
{
	auto& node = GetNode()->As<BoneMaskNode>();
	const EndpointPrototype::EndpointType type = endpoint.GetType();

	switch (endpoint.GetType()) // TODO: Rename -  GetConenctionType
	{
	case EndpointPrototype::Output:
	{
		endpoint.Style.Color = Style.HeaderColor;
		endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };

		//ImGui::Text("Mask");
	}
	}
}

editor_animation_graph::ValueNodeDeligate::ValueNodeDeligate(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
	BaseNodeDeligate(id, node)
{
	Style.Title = "Float";
	Style.HeaderColor = ImVec4{ 0.3, 0.5, 1.0, 1.0 };
	Style.Size = ImVec2{ 100, 100 };
}

void editor_animation_graph::ValueNodeDeligate::ProcessBodyContent()
{
	auto& node = GetNode()->As<animation::BlendNode2D>();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::DragFloat("##value", &node.GetEndpoint<GraphNode::Connection::Output>(0)->As<NodeValueType::Float>(), 0.001, 0.0, 1.0);
	ImGui::PopItemWidth();
}

void editor_animation_graph::ValueNodeDeligate::ProcessEndpoint(const GraphNode::EndpointIDX& endpointIdentifier, EndpointPrototype& endpoint)
{
	auto& node = GetNode()->As<animation::ValueNode>();
	const EndpointPrototype::EndpointType type = endpoint.GetType();

	switch (type)
	{
	case EndpointPrototype::Output:
	{
		endpoint.Style.Color = Style.HeaderColor;
		endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };
	}
	}
}
void editor_animation_graph::GraphDeligate::ProcessSideBar()
{
	AnimationGraph& graph = GetGraph().Get();

	//ImGui::ColorEdit4("Connection", (float*)&this->m_VisualStyle.ConnectionColor);

	ImGui::Text("Animation graph");
	ImGui::Separator();

	if (ImGui::BeginChildEx("Node: Pose", std::size_t(&graph), ImGui::GetContentRegionAvail(), true, 0))
	{
		const ImVec2 windowPosition = ImGui::GetWindowPos();
		const ImVec2 windowSize = ImGui::GetWindowSize();

		if (ImGui::BeginTable("Table", 2, ImGuiTableFlags_SizingStretchProp))
		{
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					shade::ImGuiLayer::DrawFontIcon(u8"\xf29a", 1, 0.7f); 
				}
				ImGui::TableNextColumn();
				{
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
					
					if (ImGui::BeginTable("##SelectSkeletonTable", 2, ImGuiTableFlags_SizingStretchProp))
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						{
							std::string buttonTitle = (!graph.GetSkeleton()) ? "Not set" : graph.GetSkeleton()->GetAssetData()->GetId();
							ImGui::BeginDisabled();
							ImGui::Button(buttonTitle.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0.f });
							ImGui::EndDisabled();
						}
						ImGui::TableNextColumn();
						{
							if (ImGui::ArrowButton("##OpenPopup", ImGuiDir_Down))
							{
								m_IsSkeletonPopupActive = (!m_IsSkeletonPopupActive) ? true : false;
							}
						}
						ImGui::EndTable();
					}

					if (m_IsSkeletonPopupActive)
					{
						ImGuiLayer::BeginWindowOverlay("##SkeletonSearchOverlay", ImGui::GetWindowViewport(), std::size_t(&graph), ImVec2{ windowSize.x - 10.f, 0.f }, ImVec2{ windowPosition.x + 5.f, ImGui::GetCursorScreenPos().y + 5.f }, 0.3f,
							[&]() mutable
							{
								ImGuiLayer::InputTextCol("Search", m_Search);

								if (ImGui::BeginListBox("##SelectAnimation", ImVec2{ ImGui::GetContentRegionAvail().x, 0.f }))
								{
									for (const auto& assetData : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
									{
										if (assetData.second->GetType() == shade::AssetMeta::Type::Skeleton && assetData.first.find(m_Search) != std::string::npos)
										{
											if (ImGui::Selectable(assetData.first.c_str(), false))
											{
												shade::AssetManager::GetAsset<shade::Skeleton,
													shade::BaseAsset::InstantiationBehaviour::Synchronous>(assetData.first,
														shade::AssetMeta::Category::Secondary,
														shade::BaseAsset::LifeTime::KeepAlive,
														[&](auto& skeleton) mutable
														{
															graph.SetSkeleton(skeleton);
														});

												m_IsSkeletonPopupActive = false;
											}
										}
									}
									ImGui::EndListBox();
								}

							});
					}
				}
			}

			ImGui::EndTable();
		}

		ImGui::EndChild();
	}
}
