#include "shade_pch.h"
#include "EditorAnimGraph.h"
#include <shade/core/system/FileDialog.h>

ImRect graph_editor::GraphNodePrototype::GetScaledRectangle(const InternalContext* context, const GraphVisualStyle* graphStyle)
{
	const ImVec2 scaledPosition = ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor;
	const ImVec2 pMin = (context->Offset + scaledPosition);
	const ImVec2 pMax = (context->Offset + scaledPosition + (Style.Size * context->Scale.Factor));

	return ImRect{ pMin, pMax };
}

void graph_editor::GraphNodePrototype::ProcessPopup(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, const std::string& search)
{
	shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1", 1, 0.5f);
	ImGui::SameLine();
	ImGui::Text("Animation");
	ImGui::SameLine();
	shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1\xf2d1\xf2d1", 1, 0.5f);

	!GetNode()->GetGraphContext()->As<AnimationGraphContext>().Skeleton ? ImGui::BeginTooltip(), ImGui::Text("Add skeleton to animation graph first!"), ImGui::EndTooltip(), ImGui::BeginDisabled() : void();

	if (ImGui::BeginMenu("+ Blend"))
	{
		if (ImGui::MenuItem("Blend"))
		{
			auto node = GetNode()->CreateNode<animation::BlendNode>();
			GetEditor()->InitializeRecursively(node, GetEditor()->GetNodes());
		}
		if (ImGui::MenuItem("Blend Tree 2D"))
		{
			auto node = GetNode()->CreateNode<animation::BlendTree2D>();
			GetEditor()->InitializeRecursively(node, GetEditor()->GetNodes());
		}
		ImGui::EndMenu();
	}
	if (ImGui::Selectable("+ Bone mask"))
	{
		auto node = GetNode()->CreateNode<animation::BoneMaskNode>();
		GetEditor()->InitializeRecursively(node, GetEditor()->GetNodes());
	}
	if (ImGui::Selectable("+ State machine"))
	{
		auto node = GetNode()->CreateNode<animation::state_machine::StateMachineNode>();
		GetEditor()->InitializeRecursively(node, GetEditor()->GetNodes());
	}
	if (ImGui::Selectable("+ Pose"))
	{
		auto node = GetNode()->CreateNode<animation::PoseNode>();
		GetEditor()->InitializeRecursively(node, GetEditor()->GetNodes());
	}
	if (ImGui::Selectable("+ Additive pose"))
	{
		auto node = GetNode()->CreateNode<animation::AdditivePose>();
		GetEditor()->InitializeRecursively(node, GetEditor()->GetNodes());
	}

	!GetNode()->GetGraphContext()->As<AnimationGraphContext>().Skeleton ? ImGui::EndDisabled() : void();
}

bool graph_editor::GraphNodePrototype::Draw(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	bool isClicked = false;

	ImGui::PushID(GetNode()->GetNodeIdentifier());

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x , GetScreenPosition().y } *context->Scale.Factor);

	DrawBody(context, graphStyle, nodes);

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor);

	ImGui::InvisibleButton(std::format("##NodeBox_{}", std::size_t(this)).c_str(), Style.Size * context->Scale.Factor);

	if (ImGui::IsItemActive())
	{
		isClicked = true;
	}

	MoveNode(context->Scale.Factor);

	ImGui::PopID();

	return isClicked;
}

void graph_editor::GraphNodePrototype::DrawHeader(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	auto node = GetNode();

	const ImVec2 scaledPosition = ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor;
	const ImVec2 pMin = (context->Offset + scaledPosition);
	const ImVec2 pMax = (context->Offset + scaledPosition + (ImVec2{ Style.Size.x, graphStyle->HeaderHeight } *context->Scale.Factor));

	context->DrawList->AddRectFilled(pMin, pMax, ImGui::ColorConvertFloat4ToU32(Style.HeaderColor), graphStyle->Rounding, ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersTopLeft);

	ImGui::SetCursorScreenPos(context->Offset + (graphStyle->Padding + ImVec2{ GetScreenPosition().x, GetScreenPosition().y }) * context->Scale.Factor);
	ImGui::PushStyleColor(ImGuiCol_Text, Style.HeaderTextColor);

	const float wfs = ImGui::GetCurrentWindow()->FontWindowScale;
	ImGui::SetWindowFontScale(ImGui::GetCurrentWindow()->FontWindowScale * 1.3f);

	const float columnsCount = node->CanBeOpen() ? 3 : 2;

	ImGui::PushID(node->GetNodeIdentifier());

	const std::string id = std::to_string(node->GetNodeIdentifier());

	if (ImGui::BeginTable(std::string(id + "##HeaderTableGeneral").c_str(), columnsCount, ImGuiTableFlags_SizingStretchProp, ImVec2{ Style.Size.x - graphStyle->Padding.x * 2.f, graphStyle->HeaderHeight } *context->Scale.Factor))
	{
		if (node->CanBeOpen())
		{
			ImGui::TableNextColumn();
			{
				if (ImGuiLayer::IconButton(std::string(id + "##Open").c_str(), u8"\xe867", 1, 0.8f))
					context->CurrentNode = node;
			}
		}
		ImGui::TableNextColumn();
		{
			std::string name = node->GetName();
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0,0,0, Style.HeaderColor.w * 0.2f });
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);


			if (!node->IsRenamable())
			{
				ImGui::Text(name.c_str());

				ImGui::SameLine();
			}
			else
			{
				if (ImGuiLayer::InputTextD(std::string(id + "##Name").c_str(), name))
					node->SetName(name);
			}

			ImGui::PopItemWidth();
			ImGui::PopStyleColor(1);

			if (ImGui::IsItemFocused())
			{
				ImGui::BeginTooltip();
				ImGui::Text(std::format("{:^5}", node->GetNodeIdentifier()).c_str());
				ImGui::EndTooltip();
			}
		}
		ImGui::TableNextColumn();
		{
			if (ImGuiLayer::IconButton("##RemoveConenctions", u8"\xf127", 1, 0.8f))
			{
				node->GetGraphContext()->RemoveAllConnection(node);
			}
			if (node->IsRemovable())
			{
				ImGui::SameLine();
				if (ImGuiLayer::IconButton(std::string(id + "##RemoveNode").c_str(), u8"\xe85d", 1, 0.8f))
				{
					GetEditor()->SetToRemove(node);
				}
			}
		}

		ImGui::EndTable();
	}
	ImGui::PopID();

	ImGui::PopStyleColor(1);

	ImGui::SetWindowFontScale(wfs);
}

void graph_editor::GraphNodePrototype::DrawBorder(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	const ImVec2 scaledPosition = ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor;
	const ImVec2 pMin = (context->Offset + scaledPosition);
	const ImVec2 pMax = (context->Offset + scaledPosition + Style.Size * context->Scale.Factor);

	const ImVec4 color = IsSelected() ? ImVec4(
		Style.BackgroundColor.x * 2.0f,
		Style.BackgroundColor.y * 2.0f,
		Style.BackgroundColor.z * 2.0f,
		Style.BackgroundColor.w) : Style.BackgroundColor;

	context->DrawList->AddRect(pMin, pMax, ImGui::ColorConvertFloat4ToU32(color), graphStyle->Rounding, 0, graphStyle->NodeBorderWidth * context->Scale.Factor);
}

void graph_editor::GraphNodePrototype::DrawBody(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{

	const ImVec2 nodeRectMin = context->Offset + GetScreenPosition() * context->Scale.Factor;
	const ImVec2 nodeRectMax = nodeRectMin + Style.Size * context->Scale.Factor;

	const ImVec4 bckColor = IsSelected() ? ImVec4(
		Style.BackgroundColor.x * 1.5f,
		Style.BackgroundColor.y * 1.5f,
		Style.BackgroundColor.z * 1.5f,
		Style.BackgroundColor.w) : Style.BackgroundColor;

	context->DrawList->AddRectFilled(nodeRectMin, nodeRectMax, ImGui::ColorConvertFloat4ToU32(bckColor), graphStyle->Rounding);

	DrawHeader(context, graphStyle, nodes);
	DrawBorder(context, graphStyle, nodes);
	DrawEndpoints(context, graphStyle, nodes);

	const ImVec2 endOfPrevRegion = ImGui::GetCursorScreenPos() - context->Offset;

	const ImVec2 newScreenPosition = context->Offset + ImVec2{ GetScreenPosition().x * context->Scale.Factor, endOfPrevRegion.y };
	const ImVec2 avalibleRegion{ nodeRectMax - newScreenPosition };

	ImGui::SetCursorScreenPos(newScreenPosition);

	ImGui::BeginGroup();
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, graphStyle->Padding * context->Scale.Factor);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, graphStyle->CellPadding * context->Scale.Factor);
		ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4{ 0.f, 0.f, 0.f, 0.f });

		if (ImGui::BeginTableEx("##BodyContentTable", std::size_t(this) + 2000, 1, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuterV, { avalibleRegion.x, 0 }))
		{
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					ProcessBodyContent(context);
				}
			}
			ImGui::EndTable();
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}
	ImGui::EndGroup();

	//DrawFooter(context, graphStyle, nodes);

	Style.Size.y += ImGui::GetCursorScreenPos().y - nodeRectMax.y;

	ImGui::Dummy({ 0, 0 });
}

void graph_editor::GraphNodePrototype::DrawFooter(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	const ImVec2 scaledPosition = ImVec2{ GetScreenPosition().x, GetScreenPosition().y + Style.Size.y - graphStyle->HeaderHeight } *context->Scale.Factor;
	const ImVec2 pMin = (context->Offset + scaledPosition);
	const ImVec2 pMax = (context->Offset + scaledPosition + (ImVec2{ Style.Size.x, graphStyle->HeaderHeight } *context->Scale.Factor));

	context->DrawList->AddRectFilled(pMin, pMax, ImGui::ColorConvertFloat4ToU32(Style.HeaderColor), graphStyle->Rounding, ImDrawFlags_RoundCornersBottomRight | ImDrawFlags_RoundCornersBottomLeft);

	//ImGui::SetCursorScreenPos(context->Offset + (graphStyle->Padding + ImVec2{ GetScreenPosition().x, GetScreenPosition().y }) * context->Scale.Factor);
	ImGui::PushStyleColor(ImGuiCol_Text, Style.HeaderTextColor);

	ImGui::SetWindowFontScale(ImGui::GetCurrentWindow()->FontWindowScale * 1.5f);

	ImGui::BeginGroup();
	{
		ImGui::Text(Style.Title.c_str());
		//ImGui::Text(GetNode()->GetName().c_str());
	}
	ImGui::EndGroup();

	ImGui::SetWindowFontScale(ImGui::GetCurrentWindow()->FontWindowScale / 1.5f);

	ImGui::PopStyleColor(1);

	//return nodePosition.y + m_VisualStyle.HeaderHeight;
}

void graph_editor::GraphNodePrototype::DrawEndpoints(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	const ImVec2 cellPaddingZoomed = graphStyle->CellPadding * context->Scale.Factor;

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x, GetScreenPosition().y + graphStyle->HeaderHeight } *context->Scale.Factor);
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPaddingZoomed);
	ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4{ 0.f, 0.f, 0.f, 0.f });

	auto* node = GetNode();
	auto* graphContext = node->GetGraphContext();
	const std::size_t outputCount = node->GetEndpoints()[shade::graphs::Connection::Output].GetSize();

	auto endpoints = node->GetEndpoints();
	auto inputs = endpoints[shade::graphs::Connection::Input].begin();
	auto outputs = endpoints[shade::graphs::Connection::Output].begin();
	auto& connections = GetNode()->GetConnections();
	bool needToOpenPopup = false;

	static std::function<void(graphs::BaseNode*, graphs::EndpointIdentifier)> Callback;

	if (ImGui::BeginTableEx("##EndpointsTable", std::size_t(this) + 1000, (outputCount) ? 2 : 1,
		ImGuiTableFlags_BordersOuterV |
		ImGuiTableFlags_SizingStretchProp |
		ImGuiTableFlags_NoClip,
		{ Style.Size.x * context->Scale.Factor, 0 }))
	{
		while (inputs != endpoints[shade::graphs::Connection::Input].end() || outputs != endpoints[shade::graphs::Connection::Output].end())
		{
			ImGui::TableNextRow();
			if (inputs != endpoints[shade::graphs::Connection::Input].end())
			{
				ImGui::TableNextColumn();

				const ImVec2 deltaPosition = ImGui::GetCursorScreenPos() - context->Offset;
				const ImVec2 endpointScreenPosition = ImVec2{ deltaPosition.x - cellPaddingZoomed.x, deltaPosition.y + cellPaddingZoomed.y };

				graphs::EndpointIdentifier endpoint = std::distance(endpoints[shade::graphs::Connection::Input].begin(), inputs);

				const ImVec4 color = GetValueColor(endpoints[shade::graphs::Connection::Input][endpoint]->GetType());

				ProcessEndpoint(endpoint, shade::graphs::Connection::Input, *inputs->first);

				bool referConnection = false;
				// If connections exist at this endpoint
				auto it = std::find_if(connections.begin(), connections.end(), [endpoint](const shade::graphs::Connection& c)
					{
						return c.ConnectedToEndpoint == endpoint;
					});

				if (it != connections.end())
				{
					// If connection with refer endpoint
					if (GetEditor()->GetPrototypedReferNode((*it).PConnectedFrom))
					{
						referConnection = true;
					}
				}

				if (!referConnection)
				{
					//table cutting circle !!
					if (ImGuiGraphNodeRender::DrawEndpoint(context->DrawList,
						context->Offset,
						graphStyle->EndpointRadius,
						context->Scale.Factor,
						endpointScreenPosition,
						color,
						{ color.x * 1.5f, color.y * 1.5f, color.z * 1.5f, color.w }))
					{
						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && context->ConnectionEstablish.IsOutPutSelect)
						{
							context->ConnectionEstablish.IsInputSelect = true;
							context->ConnectionEstablish.InputNode = node;
							context->ConnectionEstablish.InputEndpointIdentifier = endpoint;
						}
						else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
						{
							Callback = [node, endpoint](graphs::BaseNode* pNode, graphs::EndpointIdentifier connectedFromEndpoint)
								{
									node->ConnectNodes(endpoint, pNode, 0);
								};

							needToOpenPopup = true;
						}
					}
				}

				if (context->ConnectionEstablish.IsInputSelect && context->ConnectionEstablish.InputNode == node)
				{
					context->ConnectionEstablish.InputEndpointScreenPosition = endpointScreenPosition;
				}

				//graphs::EndpointIdentifier endpoint = std::distance(endpoints[shade::graphs::Connection::Input].begin(), inputs);

				auto connection = std::find_if(connections.begin(), connections.end(), [endpoint](const graphs::Connection& connection)
					{
						return (connection.ConnectedToEndpoint == endpoint);
					});

				if (connection != connections.end())
				{
					connection->ConnectedToPosition = glm::vec2(endpointScreenPosition.x, endpointScreenPosition.y);
				}

				inputs++;
			}
			else
			{
				ImGui::TableNextColumn();
				ImGui::Dummy(ImVec2{ ImGui::GetContentRegionAvail().x, 0 });
			}
			if (outputs != endpoints[shade::graphs::Connection::Output].end())
			{
				ImGui::TableNextColumn();

				const ImVec2 deltaPosition = ImGui::GetCursorScreenPos() - context->Offset;

				const ImVec2 endpointScreenPosition = ImVec2{ deltaPosition.x + ImGui::GetContentRegionAvail().x + cellPaddingZoomed.x, deltaPosition.y + cellPaddingZoomed.y };

				graphs::EndpointIdentifier endpoint = std::distance(endpoints[shade::graphs::Connection::Output].begin(), outputs);

				const ImVec4 color = GetValueColor(endpoints[shade::graphs::Connection::Output][endpoint]->GetType());

				ProcessEndpoint(endpoint, shade::graphs::Connection::Output, *outputs->first);

				if (ImGuiGraphNodeRender::DrawEndpoint(context->DrawList,
					context->Offset,
					graphStyle->EndpointRadius,
					context->Scale.Factor,
					endpointScreenPosition,
					color,
					{ color.x * 1.5f, color.y * 1.5f, color.z * 1.5f, color.w }))
				{
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						context->ConnectionEstablish.IsOutPutSelect = true;
						context->ConnectionEstablish.OutPutNode = node;
						context->ConnectionEstablish.OutPutEndpointIdentifier = endpoint;
					}
				}

				if (context->ConnectionEstablish.IsOutPutSelect && context->ConnectionEstablish.OutPutNode == node)
				{
					context->ConnectionEstablish.OutPutEndpointScreenPosition = endpointScreenPosition;
				}

				for (auto& [n, pack] : graphContext->GetNodes())
				{
					graphs::EndpointIdentifier endpoint = std::distance(endpoints[shade::graphs::Connection::Output].begin(), outputs);

					auto it = pack.Connections.begin();

					while (it != pack.Connections.end())
					{
						it = std::find_if(it, pack.Connections.end(), [endpoint, node](const graphs::Connection& connection)
							{
								return (connection.PConnectedFrom == node && connection.ConnectedFromEndpoint == endpoint);
							});

						if (it != pack.Connections.end())
						{
							it->ConnectedFromPosition = glm::vec2(endpointScreenPosition.x, endpointScreenPosition.y);
							++it;
						}
					}
				}
				outputs++;
			}
			else
			{
				ImGui::TableNextColumn();
			}
		}

		ImGui::EndTable();

	}

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();

	if (needToOpenPopup) ImGui::OpenPopup("##EndpointEditPopup");

	if (ImGui::BeginPopup("##EndpointEditPopup"))
	{
		shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1", 1, 0.5f);
		ImGui::SameLine();
		ImGui::Text("Input");
		ImGui::SameLine();
		shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1\xf2d1\xf2d1\xf2d1\xf2d1", 1, 0.5f);

		for (const auto [name, referNode] : GetEditor()->GetRootGraph()->As<AnimationGraph>().GetInputNodes())
		{
			shade::ImGuiLayer::DrawFontIcon(u8"\xe867", 1, 0.5f); ImGui::SameLine();

			if (ImGui::Selectable(name.c_str()))
			{
				if (context->CurrentNode)
				{
					Callback(referNode, 0);
					Callback = {}; // reset
				}
			}

		}
		ImGui::EndPopup();
	}

}

void graph_editor::GraphNodePrototype::DrawConnections(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	// Blink animation 
	static clock_t epoch = clock();
	static float duration = 1.0f, colorMultMin = 0.5f, colorMultMax = 1.5f;

	double t = std::fmod((clock() - epoch) / (double)CLOCKS_PER_SEC, duration) / duration;

	float sineWave = std::sin(t * 2.0 * IM_PI);
	float lerpValue = (sineWave + 1.0f) * 0.5f;

	float time = ImLerp(colorMultMin, colorMultMax, lerpValue);
	// !Blink animation 

	auto connections = GetNode()->GetConnections();

	for (auto& connection : connections)
	{
		const auto& endpoint = connection.PConnectedFrom->GetEndpoint<shade::graphs::Connection::Type::Output>(connection.ConnectedFromEndpoint);

		const ImVec4 color = GetValueColor(endpoint->GetType());

		if (nodes.find(std::size_t(connection.PConnectedFrom) ^ connection.PConnectedFrom->GetNodeIdentifier()) == nodes.end())
		{
			if (ImGuiGraphNodeRender::DrawReferNodeConnection(context->DrawList, context->Offset, context->Scale.Factor, ImVec2{ connection.ConnectedToPosition.x, connection.ConnectedToPosition.y },
				ImVec2{ 30.f, 30.f },
				{ color.x * time, color.y * time, color.z * time, color.w }, 1.f))
			{

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
				{
					GetNode()->DisconnectNodes(connection.ConnectedToEndpoint, connection.PConnectedFrom, connection.ConnectedFromEndpoint);
					break;
				}
				else if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
				{
					ImGui::BeginTooltipEx(0, ImGuiWindowFlags_NoBackground);
					ImGui::PushStyleColor(ImGuiCol_Text, { 0.8f, 0.2f, 0.2f, 1.f });
					ImGuiLayer::DrawFontIcon(u8"\xf127", 1, 1.8f);
					ImGui::PopStyleColor();
					ImGui::EndTooltip();
				}
				else
				{
					ImGui::BeginTooltip();
					ImGui::Text(connection.PConnectedFrom->GetName().c_str());
					ImGui::EndTooltip();
				}
			}
		}
		else
		{
			if (ImGuiGraphNodeRender::DrawConnection(
				context->DrawList,
				context->Offset,
				context->Scale.Factor,
				ImVec2{ connection.ConnectedToPosition.x,  connection.ConnectedToPosition.y },
				ImVec2{ connection.ConnectedFromPosition.x, connection.ConnectedFromPosition.y },
				color,
				ImVec4{ color.x * 1.5f, color.y * 1.5f, color.z * 1.5f, color.w },
				graphStyle->ConnectionThickness))
			{
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
				{
					GetNode()->DisconnectNodes(connection.ConnectedToEndpoint, connection.PConnectedFrom, connection.ConnectedFromEndpoint);
					break;
				}
				else if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
				{
					ImGui::BeginTooltipEx(0, ImGuiWindowFlags_NoBackground);
					ImGui::PushStyleColor(ImGuiCol_Text, { 0.8f, 0.2f, 0.2f, 1.f });
					ImGuiLayer::DrawFontIcon(u8"\xf127", 1, 1.8f);
					ImGui::PopStyleColor();
					ImGui::EndTooltip();
				}

			}
		}
	}

	if (context->ConnectionEstablish.IsOutPutSelect && context->ConnectionEstablish.OutPutNode == GetNode())
	{
		auto& endpoints = GetNode()->GetEndpoints();
		const auto& endpoint = endpoints[EndpointPrototype::EndpointType::Output].At(context->ConnectionEstablish.OutPutEndpointIdentifier);

		const ImVec4 color = GetValueColor(endpoint->GetType());

		ImGuiGraphNodeRender::DrawConnection(
			context->DrawList,
			context->Offset,
			context->Scale.Factor,
			ImGui::GetMousePos() - context->Offset,
			context->ConnectionEstablish.OutPutEndpointScreenPosition,
			color,
			{ color.x * time, color.y * time, color.z * time, color.w },
			graphStyle->ConnectionThickness);
	}
}

void graph_editor::GraphNodePrototype::DrawNodes()
{
	auto& context = GetEditor()->GetContext();

	context.DrawList->ChannelsSplit(2);
	
	GraphNodePrototype* pnOnTopOf = nullptr;

	for (auto node : GetNode()->GetInternalNodes())
	{
		if (node->GetNodeType() == shade::graphs::GetNodeTypeId<shade::animation::state_machine::TransitionNode>())
			continue;

		if (auto pN = GetEditor()->GetPrototypedNode(node))
		{
			
			if (pN->IsSelected())
				pnOnTopOf = pN;

			if (pnOnTopOf != pN)
			{
				context.DrawList->ChannelsSetCurrent(0);

				if (pN->Draw(&context, &GetEditor()->GetVisualStyle(), GetEditor()->GetNodes()))
				{
					GetEditor()->SetSelectedNode(pN);
					pN->SetSelected(true);
				}
			}

			if (GetEditor()->GetSelectedNode() != nullptr && pN != GetEditor()->GetSelectedNode())
			{
				pN->SetSelected(false);
			}
		}
	}

	if (pnOnTopOf)
	{
		context.DrawList->ChannelsSetCurrent(1);
		pnOnTopOf->Draw(&context, &GetEditor()->GetVisualStyle(), GetEditor()->GetNodes());
	}

	context.DrawList->ChannelsMerge();
}

void graph_editor::GraphNodePrototype::MoveNode(float scaleFactor)
{
	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		ImVec2 delta = ImGui::GetIO().MouseDelta / scaleFactor;

		if (fabsf(delta.x) >= 0.1f || fabsf(delta.y) >= 0.1f)
		{
			SetScreenPosition(ImVec2{ GetScreenPosition().x + delta.x,
				GetScreenPosition().y + delta.y });
		}
	}
}


void graph_editor::GraphEditor::InitializeRecursively(graphs::BaseNode* pNode, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, bool cursor)
{
	if (dynamic_cast<state_machine::StateMachineNode*>(pNode))
	{
		// Transition as graph node, contains nodes
		state_machine::StateMachineNode* machine = reinterpret_cast<state_machine::StateMachineNode*>(pNode);

		CreateNode<StateMachineNodeDelegate>(pNode, nodes);
		// Aka states
		for (auto state : machine->GetInternalNodes())
		{
			InitializeRecursively(state, nodes, cursor);
		}
		}
	else if (dynamic_cast<state_machine::TransitionNode*>(pNode))
	{
		// Transition as graph node, contains nodes
		state_machine::TransitionNode* transition = reinterpret_cast<state_machine::TransitionNode*>(pNode);
		
		/*CreateNode<TransitionNodeDelegate>(pNode, nodes); */

		for (auto node : transition->GetInternalNodes())
		{
			InitializeRecursively(node, nodes, cursor);
		}
	}
	else if (dynamic_cast<state_machine::StateNode*>(pNode))
	{
		state_machine::StateNode* state = reinterpret_cast<state_machine::StateNode*>(pNode);

		CreateNode<StateNodeDelegate>(pNode, nodes);

		for (auto node : state->GetInternalNodes())
		{
			InitializeRecursively(node, nodes, cursor);
		}
	}
	else if (dynamic_cast<graphs::BaseNode*>(pNode))
	{
		if (dynamic_cast<shade::animation::OutputPoseNode*>(pNode))
		{
			shade::animation::OutputPoseNode* value = reinterpret_cast<shade::animation::OutputPoseNode*>(pNode);

			CreateNode<OutputPoseNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::animation::BlendNode*>(pNode))
		{
			shade::animation::BlendNode* value = reinterpret_cast<shade::animation::BlendNode*>(pNode);

			CreateNode<BlendNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::animation::state_machine::OutputTransitionNode*>(pNode))
		{
			shade::animation::state_machine::OutputTransitionNode* value = reinterpret_cast<shade::animation::state_machine::OutputTransitionNode*>(pNode);

			CreateNode<OutputTransitionNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::animation::PoseNode*>(pNode))
		{
			shade::animation::PoseNode* value = reinterpret_cast<shade::animation::PoseNode*>(pNode);
			CreateNode<PoseNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::graphs::IntEqualsNode*>(pNode))
		{
			shade::graphs::IntEqualsNode* value = reinterpret_cast<shade::graphs::IntEqualsNode*>(pNode);
			CreateNode<IntEqualsNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::graphs::IntNode*>(pNode))
		{
			shade::graphs::IntNode* value = reinterpret_cast<shade::graphs::IntNode*>(pNode);
			CreateNode<IntNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::graphs::FloatEqualsNode*>(pNode))
		{
			shade::graphs::FloatEqualsNode* value = reinterpret_cast<shade::graphs::FloatEqualsNode*>(pNode);
			CreateNode<FloatEqualsNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::graphs::FloatNode*>(pNode))
		{
			shade::graphs::FloatNode* value = reinterpret_cast<shade::graphs::FloatNode*>(pNode);
			CreateNode<FloatNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::graphs::StringNode*>(pNode))
		{
			shade::graphs::StringNode* value = reinterpret_cast<shade::graphs::StringNode*>(pNode);
			CreateNode<StringNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::graphs::BoolNode*>(pNode))
		{
			shade::graphs::BoolNode* value = reinterpret_cast<shade::graphs::BoolNode*>(pNode);
			CreateNode<BoolNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::animation::BoneMaskNode*>(pNode))
		{
			shade::animation::BoneMaskNode* value = reinterpret_cast<shade::animation::BoneMaskNode*>(pNode);
			CreateNode<BoneMaskNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::graphs::FloatScaleRange*>(pNode))
		{
			shade::graphs::FloatScaleRange* value = reinterpret_cast<shade::graphs::FloatScaleRange*>(pNode);
			CreateNode<FloatScaleRangeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::animation::BlendTree2D*>(pNode))
		{
			shade::animation::BlendTree2D* value = reinterpret_cast<shade::animation::BlendTree2D*>(pNode);
			CreateNode<BlendTree2DNodeDelegate>(pNode, nodes);
		}
		if (dynamic_cast<shade::graphs::Vec2FloatNode*>(pNode))
		{
			shade::graphs::Vec2FloatNode* value = reinterpret_cast<shade::graphs::Vec2FloatNode*>(pNode);
			CreateNode<Vec2FloatDNodeDelegate>(pNode, nodes);
		}
		}

		if (!cursor)
		{
			const ImVec2 mousePos = (ImGui::GetIO().MousePos - m_Context.Offset) / m_Context.Scale.Factor;
			pNode->GetScreenPosition() = glm::vec2(mousePos.x, mousePos.y);

		}
}

graph_editor::GraphNodePrototype* graph_editor::GraphEditor::GetPrototypedNode(graphs::BaseNode* pNode)
{
	auto node = m_Nodes.find(std::size_t(pNode) ^ pNode->GetNodeIdentifier());

	if (node != m_Nodes.end())
		return node->second;
	else
		return nullptr;
}

graph_editor::GraphNodePrototype* graph_editor::GraphEditor::GetPrototypedReferNode(graphs::BaseNode* pNode)
{
	auto node = m_ReferNodes.find(std::size_t(pNode) ^ pNode->GetNodeIdentifier());

	if (node != m_ReferNodes.end())
		return node->second;
	else
		return nullptr;
}

bool graph_editor::GraphEditor::RemoveNode(graphs::BaseNode*& pNode)
{
	if (pNode)
	{
		// Keep as copy !!
		auto iNodes = pNode->GetInternalNodes();

		for (auto cN : iNodes)
			RemoveNode(cN);

		auto itN = m_Nodes.find(std::size_t(pNode) ^ pNode->GetNodeIdentifier());
		auto itRN = m_ReferNodes.find(std::size_t(pNode) ^ pNode->GetNodeIdentifier());

		if (itN != m_Nodes.end())
			m_Nodes.erase(itN);

		if (itRN != m_ReferNodes.end())
			m_ReferNodes.erase(itRN);


		pNode->GetParrentGraph()->RemoveNode(pNode);
		pNode = nullptr, m_pSelectedNode = nullptr;

		return true;
	}

	return false;
}


graph_editor::GraphEditor::~GraphEditor()
{
	for (auto [hash, node] : m_Nodes)
	{
		delete node;
	}
	for (auto [hash, node] : m_ReferNodes)
	{
		delete node;
	}
	m_Nodes.clear();
	m_ReferNodes.clear();
}

void graph_editor::GraphEditor::Initialize(Asset<AnimationGraph>& graph)
{	
	m_Initialized = false;

	for (auto [hash, node] : m_Nodes)
	{
		delete node;
	}
	for (auto [hash, node] : m_ReferNodes)
	{
		delete node;
	}

	m_pSelectedNode = nullptr;
	m_pRootGraph = nullptr;

	m_Nodes.clear();
	m_ReferNodes.clear();
	// Add to nodes !!
	CreateNode<AnimationGraphDelegate>(graph.Raw(), m_Nodes);

	m_Context.CurrentNode = graph.Raw();
	m_pRootGraph = graph;

	// Serialize internal nodes only
	for (auto pNode : m_pRootGraph->GetInternalNodes())
	{
		if (graph->GetInputNodes().end() == std::find_if(
			graph->GetInputNodes().begin(),
			graph->GetInputNodes().end(), [pNode](const std::pair<std::string, shade::graphs::BaseNode*>& input
				)
			{
				return input.second == pNode;
			}))
		{
			InitializeRecursively(pNode, m_Nodes, true);
		}
	}

	for (auto [name, node] : graph->GetInputNodes())
	{
		InitializeRecursively(node, m_ReferNodes, true);
	}

	//m_pGraph->Initialize();
}

void graph_editor::GraphEditor::SetToRemove(graphs::BaseNode* pNode)
{
	m_pNodeToRemove = pNode;
}

bool graph_editor::GraphEditor::Edit(const char* title, const ImVec2& size, const std::function<void()>& menuCallBack)
{
	if (!m_pRootGraph)
		return false;
	// Save current style 
	ImGuiStyle unscaledStyle = ImGui::GetStyle();
	ImGui::PushStyleColor(ImGuiCol_WindowBg, m_VisualStyle.BackgroundColor);
	ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
	m_Context.DrawList = ImGui::GetWindowDrawList();
	{
		if (ImGui::Begin(title, (bool*)0))
		{
			if (ImGui::BeginChild("##ImGuiGraphPrototype::ContentSideBar",
				ImVec2(ImGui::GetWindowSize().x / 8.f, ImGui::GetContentRegionAvail().y),
				true,
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoScrollWithMouse))
			{

				GetPrototypedNode(m_pRootGraph.Raw())->ProcessSideBar(&m_Context, m_Nodes, m_ReferNodes);

			} ImGui::EndChild();

			ImGui::SameLine();

			const ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
			const ImVec2 windowPos = cursorPosition, scrollRegionLocalPos(0, 0);
			m_Context.CanvasSize = ImVec2{ ImGui::GetWindowSize().x * 0.7f, ImGui::GetContentRegionAvail().y };
			m_Context.CanvasRect = ImRect{ windowPos, windowPos + m_Context.CanvasSize };
			m_Context.Offset = cursorPosition + (m_Context.CanvasPosition * m_Context.Scale.Factor);

			if (!m_Initialized)
			{
				m_Context.CanvasPosition.x = m_Context.CanvasRect.GetWidth() / 2.f;
				m_Context.CanvasPosition.y = m_Context.CanvasRect.GetHeight() / 2.f;
				m_Initialized = true;
			}

			if (ImGui::BeginChild("##ImGuiGraphPrototype::ScrollRegion",
				ImVec2(ImGui::GetWindowSize().x * 0.7f, 0),
				true,
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoScrollWithMouse))
			{

				m_Context.DrawList = ImGui::GetWindowDrawList();
				// Set all elements scale inside canvas 
				ImGui::GetStyle().ScaleAllSizes(m_Context.Scale.Factor);
				ImGui::SetWindowFontScale(m_Context.Scale.Factor);

				ProcessScale();

				ImGuiGraphNodeRender::DrawGrid(m_Context.DrawList,
					windowPos,
					m_Context.CanvasSize,
					m_Context.CanvasPosition,
					m_Context.Scale.Factor,
					ImGui::ColorConvertFloat4ToU32(m_VisualStyle.GridColorSmall),
					ImGui::ColorConvertFloat4ToU32(m_VisualStyle.GridColorBig),
					m_VisualStyle.GridSize);
				{

					ImVec2 center(m_Context.Offset.x, m_Context.Offset.y);

					m_Context.DrawList->AddCircleFilled(center, 5.0f, ImGui::ColorConvertFloat4ToU32({ 0.5, 0.1, 0.1, 1.0 })); // Размер точки равен 3.0f

					if (ImGui::IsItemActive())
					{
						if (m_pSelectedNode != nullptr)
							m_pSelectedNode->SetSelected(false);

						m_pSelectedNode = nullptr;

						m_Context.ConnectionEstablish.Reset();
					}

					if (auto pNode = GetPrototypedNode(m_Context.CurrentNode))
					{
						pNode->DrawNodes();
					}
					else
					{
						DrawNodes();
					}
					DrawConnections();
				}

				// Reset style back
				ImGui::GetStyle() = unscaledStyle;


			} ImGui::EndChild();

			ImGui::SameLine();

			if (ImGui::BeginChild("##ImGuiGraphPrototype::ContentSideBarNodeads",
				ImVec2(ImGui::GetWindowSize().x / 6.0f, 0),
				true,
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoScrollWithMouse))
			{

				if (m_pSelectedNode)
				{
					m_pSelectedNode->ProcessSideBar(&m_Context, m_Nodes, m_ReferNodes);
				}
				else
				{
					if (m_pRootGraph.Raw() != m_Context.CurrentNode)
					{
						if (auto pN = GetPrototypedNode(m_Context.CurrentNode))
						{
							pN->ProcessSideBar(&m_Context, m_Nodes, m_ReferNodes);
						}
					}
				}

			} ImGui::EndChild();

			if (m_Context.CanvasRect.Contains(ImGui::GetIO().MousePos))
			{
				// Scrolling
				if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
					m_Context.CanvasPosition += ImGui::GetIO().MouseDelta / m_Context.Scale.Factor;

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && !m_pSelectedNode)
				{
					ImGui::OpenPopup("##GraphEditorPopup");
				}
			}

			if (ImGui::BeginPopup("##GraphEditorPopup"))
			{

				std::string search; ImGuiLayer::InputTextCol("Search", search);

				if (auto pN = GetPrototypedNode(m_Context.CurrentNode))
				{
					pN->ProcessPopup(&m_Context, m_Nodes, search);
				}
				PopupMenu();

				ImGui::EndPopup();
			}
			ImGui::End();
		}
	}
	ImGui::PopStyleColor();


	ImGuiLayer::BeginWindowOverlay("Path",
		ImGui::GetCurrentWindow()->Viewport, 123124,
		ImVec2{ m_Context.CanvasSize.x, ImGui::GetFrameHeight() + 8.f },
		m_Context.CanvasRect.Min, 1.0f, [&]() {DrawPathRecursevly(m_Context.CurrentNode); });


	if (m_Context.ConnectionEstablish.IsInputSelect && m_Context.ConnectionEstablish.IsOutPutSelect)
	{
		m_pRootGraph->GetGraphContext()->ConnectNodes(
			m_Context.ConnectionEstablish.InputNode,
			m_Context.ConnectionEstablish.InputEndpointIdentifier,
			m_Context.ConnectionEstablish.OutPutNode,
			m_Context.ConnectionEstablish.OutPutEndpointIdentifier);
		m_Context.ConnectionEstablish.Reset();
	}

	// TODO: ноды по адресу не уникальны потому что мапа делает ресайз и адреса путаются,
	// нужно сделать так что бы BaseNode был уникльным присоздании как SharedPointer !!
	RemoveNode(m_pNodeToRemove);

	return false;

}

void graph_editor::GraphEditor::ProcessScale()
{
	ImGuiIO& io = ImGui::GetIO(); // todo add as arg

	if (m_Context.CanvasRect.Contains(io.MousePos))
	{
		if (io.MouseWheel <= -std::numeric_limits<float>::epsilon())
			m_Context.Scale.TargetFactor *= 1.0f - m_Context.Scale.RatioFactor;

		if (io.MouseWheel >= std::numeric_limits<float>::epsilon())
			m_Context.Scale.TargetFactor *= 1.0f + m_Context.Scale.RatioFactor;
	}

	const ImVec2 deltaMouseWorldPossition = CalculateMouseWorldPos(io.MousePos);

	m_Context.Scale.TargetFactor = ImClamp(m_Context.Scale.TargetFactor, m_Context.Scale.MinFactor, m_Context.Scale.MaxFactor);
	m_Context.Scale.Factor = ImClamp(m_Context.Scale.Factor, m_Context.Scale.MinFactor, m_Context.Scale.MaxFactor);
	m_Context.Scale.Factor = ImLerp(m_Context.Scale.Factor, m_Context.Scale.TargetFactor, m_Context.Scale.LerpFactor);

	const ImVec2 mouseWorldPossition = CalculateMouseWorldPos(io.MousePos);

	if (ImGui::IsMousePosValid())
		m_Context.CanvasPosition += mouseWorldPossition - deltaMouseWorldPossition;
}

ImVec2 graph_editor::GraphEditor::CalculateMouseWorldPos(const ImVec2& mousePosition)
{
	return (mousePosition - ImGui::GetCursorScreenPos()) / m_Context.Scale.Factor;
}

void graph_editor::GraphEditor::DrawNodes()
{
	m_Context.DrawList->ChannelsSplit(2);

	GraphNodePrototype* pnOnTopOf = nullptr;

	for (auto node : m_Context.CurrentNode->GetInternalNodes())
	{
		if (auto pN = GetPrototypedNode(node))
		{
			if (pN->IsSelected())
				pnOnTopOf = pN;

			if (pnOnTopOf != pN)
			{
				m_Context.DrawList->ChannelsSetCurrent(0);

				if (pN->Draw(&m_Context, &m_VisualStyle, m_Nodes))
				{
					m_pSelectedNode = pN;
					pN->SetSelected(true);
				}
			}

			if (m_pSelectedNode != nullptr && pN != m_pSelectedNode)
			{
				pN->SetSelected(false);
			}
		}
	}

	if (pnOnTopOf)
	{
		m_Context.DrawList->ChannelsSetCurrent(1);
		pnOnTopOf->Draw(&m_Context, &m_VisualStyle, m_Nodes);
	}

	m_Context.DrawList->ChannelsMerge();
}

void graph_editor::GraphEditor::DrawConnections()
{
	if (m_Context.CurrentNode)
	{
		for (auto& node : m_Context.CurrentNode->GetInternalNodes())
		{
			if (auto prototypedNode = GetPrototypedNode(node))
			{
				prototypedNode->DrawConnections(&m_Context, &m_VisualStyle, m_Nodes);
			}
		}

		//for (auto& node : m_Context.CurrentGraph->GetNode()->GetReferNodes())
		//{
		//	auto& connections = m_ReferNodes.begin()->second->GetNode()->GetConnections();

		//	if (connections.find(node) != connections.end())
		//	{
		//		auto& nodeConnections = connections.at(node);

		//		for (const auto& connection : nodeConnections)
		//		{
		//			const float size = 150.f;
		//			const ImVec2	p1 = m_Context.Offset + ImVec2{ connection.OutputScreenPosition.x - size, connection.OutputScreenPosition.y + size },
		//				p2 = m_Context.Offset + ImVec2{ connection.OutputScreenPosition.x - size, connection.OutputScreenPosition.y - size },
		//				p3 = m_Context.Offset + ImVec2{ connection.OutputScreenPosition.x, connection.OutputScreenPosition.y };

		//			m_Context.DrawList->AddTriangle(p1, p2, p3, ImGui::ColorConvertFloat4ToU32({ 1,1,1,1 }), 5.f);


		//			/*ImGuiGraphNodeRender::DrawConnection(
		//				m_Context.DrawList,
		//				m_Context.Offset,
		//				m_Context.Scale.Factor,
		//				ImVec2{ connection.InputScreenPosition.x,  connection.InputScreenPosition.y },
		//				ImVec2{ connection.OutputScreenPosition.x, connection.OutputScreenPosition.y },

		//				ImVec4{ 1, 1, 1, 1 },
		//				m_VisualStyle.ConnectionThickness);*/
		//		}
		//	}

		//	GetPrototypedReferNode(node)->DrawConnections(&m_Context, &m_VisualStyle, m_ReferNodes);
		//}
	}

}

void graph_editor::GraphEditor::PopupMenu()
{
	shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1", 1, 0.5f);
	ImGui::SameLine();
	ImGui::Text("Values");
	ImGui::SameLine();
	shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1\xf2d1\xf2d1\xf2d1", 1, 0.5f);

	if (ImGui::BeginMenu("+ Int"))
	{
		if (ImGui::MenuItem("Int"))  InitializeRecursively(m_Context.CurrentNode->CreateNode<graphs::IntNode>(), m_Nodes);
		if (ImGui::MenuItem("Int equals"))  InitializeRecursively(m_Context.CurrentNode->CreateNode<graphs::IntEqualsNode>(), m_Nodes);
		ImGui::BeginDisabled();
		ImGui::MenuItem("Int not equals");
		ImGui::MenuItem("Int higher");
		ImGui::MenuItem("Int lesser");
		ImGui::EndDisabled();

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("+ Float"))
	{

		if (ImGui::MenuItem("+ Float"))  InitializeRecursively(m_Context.CurrentNode->CreateNode<graphs::FloatNode>(), m_Nodes);
		if (ImGui::MenuItem("Float equals"))  InitializeRecursively(m_Context.CurrentNode->CreateNode<graphs::FloatEqualsNode>(), m_Nodes);
		if (ImGui::MenuItem("Float scale range"))  InitializeRecursively(m_Context.CurrentNode->CreateNode<graphs::FloatScaleRange>(), m_Nodes);

		ImGui::BeginDisabled();
		ImGui::MenuItem("Float not equals");
		ImGui::MenuItem("Float higher");
		ImGui::MenuItem("Float lesser");
		ImGui::EndDisabled();

		if (ImGui::MenuItem("Vec2"))  InitializeRecursively(m_Context.CurrentNode->CreateNode<graphs::Vec2FloatNode>(), m_Nodes);

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("+ Bool"))
	{
		if (ImGui::MenuItem("Bool")) InitializeRecursively(m_Context.CurrentNode->CreateNode<graphs::BoolNode>(), m_Nodes);

		ImGui::BeginDisabled();
		ImGui::MenuItem("And");
		ImGui::MenuItem("Or");
		ImGui::MenuItem("Not");
		ImGui::EndDisabled();

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("+ String"))
	{
		if (ImGui::MenuItem("String")) InitializeRecursively(m_Context.CurrentNode->CreateNode<graphs::StringNode>(), m_Nodes);
		ImGui::BeginDisabled();
		ImGui::EndDisabled();

		ImGui::EndMenu();
	}

	shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1", 1, 0.5f);
	ImGui::SameLine();
	ImGui::Text("Input");
	ImGui::SameLine();
	shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1\xf2d1\xf2d1\xf2d1\xf2d1", 1, 0.5f);

	ImGui::BeginDisabled();
	for (const auto [name, node] : m_pRootGraph->As<AnimationGraph>().GetInputNodes())
	{
		shade::ImGuiLayer::DrawFontIcon(u8"\xe867", 1, 0.5f); ImGui::SameLine();

		if (ImGui::Selectable(name.c_str()))
		{
			if (m_Context.CurrentNode)
			{
				//m_Context.CurrentGraph->GetNode()->AddReferNode(node);
			}
		}

	}
	ImGui::EndDisabled();

	ImGui::Dummy({ 0,15.f });
	ImGui::Separator();
	if (ImGui::Button("Close"))
		ImGui::CloseCurrentPopup();
}

void graph_editor::GraphEditor::DrawPathRecursevly(graphs::BaseNode* pNode)
{
	if (auto pN = pNode->GetParrentGraph())
	{
		if (auto spN = dynamic_cast<shade::animation::state_machine::TransitionNode*>(pNode))
		{
			DrawPathRecursevly(pN->GetParrentGraph());
		}
		else
		{
			DrawPathRecursevly(pN);
		}

	}

	ImGuiLayer::DrawFontIcon(u8"\xe982", 1, 0.6f); ImGui::SameLine();

	if (ImGui::Button(pNode->GetName().c_str()))
	{
		m_Context.CurrentNode = pNode;
	}

	ImGui::SameLine();
}

graph_editor::StateNodeDelegate::StateNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = STATE_NODE_COLOR;
}

// Надо это переделать, поместить это куда то в обище где все отрисовуются и проверять если это стейт или стейт машина то нужно делать коннекшены !
// Я бы все переделал конечно, прям все !! 

bool graph_editor::StateNodeDelegate::Draw(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	const ImVec2 nodeRectMin = context->Offset + GetScreenPosition() * context->Scale.Factor;
	const ImVec2 nodeRectMax = nodeRectMin + (Style.Size - ImVec2{ 15.f, 15.f }) * context->Scale.Factor;
	const ImRect nodeRect(nodeRectMin, nodeRectMax);

	bool isClicked = false;

	ImGui::PushID(GetNode()->GetNodeIdentifier());

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x , GetScreenPosition().y } *context->Scale.Factor);

	DrawBody(context, graphStyle, nodes);

	if (GetNode() == GetNode()->GetParrentGraph()->As<shade::animation::state_machine::StateMachineNode>().GetCurrentState())
	{
		context->DrawList->AddCircleFilled(nodeRectMax, 5.f * context->Scale.Factor, ImGui::ColorConvertFloat4ToU32(Style.HeaderColor), 0);
	}

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor);

	if (ImGui::InvisibleButton(std::format("##NodeBox_{}", std::size_t(this)).c_str(), Style.Size * context->Scale.Factor))
	{
	}

	if (ImGui::IsItemActive())
	{
		isClicked = true;
	}

	MoveNode(context->Scale.Factor);

	ImGui::PopID();

	return isClicked;
}

void graph_editor::StateNodeDelegate::DrawBody(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	const ImVec2 nodeRectMin = context->Offset + GetScreenPosition() * context->Scale.Factor;
	const ImVec2 nodeRectMax = nodeRectMin + Style.Size * context->Scale.Factor;

	const ImVec4 bckColor = IsSelected() ? ImVec4(
		Style.BackgroundColor.x * 1.5f,
		Style.BackgroundColor.y * 1.5f,
		Style.BackgroundColor.z * 1.5f,
		Style.BackgroundColor.w) : Style.BackgroundColor;

	context->DrawList->AddRectFilled(nodeRectMin, nodeRectMax, ImGui::ColorConvertFloat4ToU32(bckColor), graphStyle->Rounding);

	DrawHeader(context, graphStyle, nodes);
	DrawBorder(context, graphStyle, nodes);

	const ImVec2 endOfPrevRegion = ImGui::GetCursorScreenPos() - context->Offset;

	const ImVec2 newScreenPosition = context->Offset + ImVec2{ GetScreenPosition().x * context->Scale.Factor, endOfPrevRegion.y };
	const ImVec2 avalibleRegion{ nodeRectMax - newScreenPosition };

	ImGui::SetCursorScreenPos(newScreenPosition);

	ImGui::BeginGroup();
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, graphStyle->Padding * context->Scale.Factor);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, graphStyle->CellPadding * context->Scale.Factor);
		ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4{ 0.f, 0.f, 0.f, 0.f });

		if (ImGui::BeginTableEx("##BodyContentTable", std::size_t(this) + 2000, 1, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuterV, { avalibleRegion.x, 0 }))
		{
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					ProcessBodyContent(context);
				}
			}
			ImGui::EndTable();
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}
	ImGui::EndGroup();

	Style.Size.y += ImGui::GetCursorScreenPos().y - nodeRectMax.y;

	ImGui::Dummy({ 0, 0 });
}

graph_editor::OutputPoseNodeDelegate::OutputPoseNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.Title = "Out pose";
}

void graph_editor::OutputPoseNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	
}

void graph_editor::OutputPoseNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	ImGui::Text("Income pose");

	auto pose = GetNode()->GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(0);
	std::size_t hash = (pose) ? pose->GetAnimationHash() : 0;
	
	ImGuiLayer::HelpMarker("#", std::format("{:x}", hash).c_str());
}

graph_editor::BlendNodeDelegate::BlendNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = POSE_VALUE_COLOR;
	Style.Title = "Blend";
}

void graph_editor::BlendNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	switch (endpoint.GetType())
	{
	case NodeValueType::Float:
	{
		ImGui::Text("Blend weight");
		break;
	}
	case NodeValueType::Pose:
	{
		ImGui::Text("Pose");
		break;
	}
	case NodeValueType::BoneMask:
	{
		ImGui::Text("Bone mask");
		break;
	}
	}
}

graph_editor::OutputTransitionNodeDelegate::OutputTransitionNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = BOOL_VALUE_COLOR;
	Style.Title = "Transition";
	Style.Size.x = 310.f;
}

void graph_editor::OutputTransitionNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	switch (endpoint.GetType())
	{
	case NodeValueType::Bool:
	{
		if (identifier == 0)
		{
			ImGui::Text("Transition");

		}
		if (identifier == 3)
		{
			ImGui::Text("Reset play time"); ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2.f);
			ImGui::Checkbox("##Reset", &endpoint.As<NodeValueType::Bool>());
		}
		if (identifier == 4)
		{
			ImGui::Text("Can be interrupted"); ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2.f);
			ImGui::Checkbox("##Interrupted", &endpoint.As<NodeValueType::Bool>());
		}
		break;
	}
	case NodeValueType::Float:
	{
		if (identifier == 1)
		{
			ImGui::Text("Duration"); ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2.f);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat("##Duration", &endpoint.As<NodeValueType::Float>(), 0.001, 0.0, 1.0);
			ImGui::PopItemWidth();
		}
		if (identifier == 2)
		{
			ImGui::Text("Offset"); ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2.f);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat("##Offset", &endpoint.As<NodeValueType::Float>(), 0.001, 0.0, 1.0);
			ImGui::PopItemWidth();
		}
		break;
	}
	}
}

void graph_editor::OutputTransitionNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	auto& node = GetNode()->As<shade::animation::state_machine::OutputTransitionNode>();
	auto& style = node.GetSynStyle();

	if (ImGui::BeginTable("##SynchronizationTable", 2))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		{
			ImGui::Text("Synchronization");
		}
		ImGui::TableNextColumn();
		{

			static std::vector<std::string> elements = { "None", "Src frozen", "Src to dest", "Dest to src", "Dest and src", "Key frame" };
			static std::uint32_t selected = (std::uint8_t)style;

			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGuiLayer::DrawComboWithIndex("##Sync", selected, elements, 0, 0))
			{
				style = static_cast<shade::animation::state_machine::SyncStyle>(selected);
			}
			ImGui::PopItemWidth();
		}
		ImGui::EndTable();
	}

}

void graph_editor::OutputTransitionNodeDelegate::ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes)
{
	if (ImGui::BeginChild("##TransitionCurveEdit"))
	{
		auto& node = GetNode()->As<shade::animation::state_machine::OutputTransitionNode>();

		auto& control = node.GetCurveControllPoints();

		float blendMin = 0.f, blendMax = 1.f, transitionStart = 0.f, transitionEnd = node.GetTransitionDuration(), currentTime = node.GetTransitionAccumulator();

		//ImGui::BeginDisabled();

		shade::CurveEditor("BlendCurve", currentTime, blendMin, blendMax, transitionStart, transitionEnd, control);

		//ImGui::EndDisabled();
	}ImGui::EndChild();

}

graph_editor::PoseNodeDelegate::PoseNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = POSE_VALUE_COLOR;
	Style.Title = "Pose";
}

void graph_editor::PoseNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	PoseNode& node = GetNode()->As<animation::PoseNode>();

	float& start = node.GetAnimationData().Start;
	float& end = node.GetAnimationData().End;
	float& currentTime = node.GetAnimationData().CurrentPlayTime;
	auto& rootMotion = node.GetAnimationData().HasRootMotion;

	auto pose = GetNode()->GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0);
	std::size_t hash = (pose) ? pose->GetAnimationHash() : 0;

	(!rootMotion) ? ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)) : void();
	shade::ImGuiLayer::DrawFontIcon(u8"\xf21d", 1, 0.5f);
	(!rootMotion) ? ImGui::PopStyleColor() : void();

	ImGui::SameLine(); ImGuiLayer::HelpMarker("#", std::format("{:x}", hash).c_str()); ImGui::SameLine();

	{
		// Calculate the normalized value between 0 and 1
		float result = (currentTime - start) / (end - start);
		// Clamp the result to make sure it stays between 0 and 1
		result = glm::clamp(result, 0.0f, 1.0f);

		ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2{ 0, ImGui::GetFrameHeight() / 4.f });
		ImGui::ProgressBar(result, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() / 7.f), "");
	}
}

void graph_editor::PoseNodeDelegate::ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes)
{
	PoseNode& node = GetNode()->As<animation::PoseNode>();

	float& start = node.GetAnimationData().Start;
	float& end = node.GetAnimationData().End;
	float& duration = node.GetAnimationData().Duration;
	float& ticksPerSecond = node.GetAnimationData().TicksPerSecond;
	bool& isloop = node.GetAnimationData().IsLoop;
	auto& state = node.GetAnimationData().State;
	auto& rootMotion = node.GetAnimationData().HasRootMotion;
	auto& syncGroup = node.GetAnimationData().SyncGroupName;
	auto& syncGroups = node.GetGraphContext()->As<AnimationGraphContext>().SyncGroups;

	const float FPS = (node.GetAnimationData().Animation) ? node.GetAnimationData().Animation->GetFps() : 0.f;
	if (ImGui::BeginChildEx("Node: Pose", std::size_t(&node), {0, ImGui::GetContentRegionAvail().y / 2.f}, true, 0))
	{
		const ImVec2 windowPosition = ImGui::GetWindowPos();
		const ImVec2 windowSize = ImGui::GetWindowSize();

		ImGui::Dummy({ (ImGui::GetContentRegionAvail().x - 30.f) / 2.f ,0 }); ImGui::SameLine();
		if (state == Animation::State::Play)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertFloat4ToU32({ 0.7f, 0.7f, 0.2f, 1.f }));
			if (ImGuiLayer::IconButton("##Pause", u8"\xf28c", 1, 1.1f))
			{
				state = Animation::State::Pause;
			}
			ImGui::PopStyleColor();
		}
		else if (state == Animation::State::Pause || state == Animation::State::Stop)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertFloat4ToU32({ 0.2f, 0.7f, 0.2f, 1.f }));
			if (ImGuiLayer::IconButton("##Pause", u8"\xe88c", 1, 1.1f))
			{
				state = Animation::State::Play;
			}
			ImGui::PopStyleColor();
		}
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::ColorConvertFloat4ToU32({ 0.7f, 0.2f, 0.2f, 1.f }));
		if (ImGuiLayer::IconButton("##Stop", u8"\xf28e", 1, 1.1f))
		{
			state = Animation::State::Stop;
		}
		ImGui::PopStyleColor();

		ImGui::Separator();

		if (ImGui::BeginTable("Table", 2, ImGuiTableFlags_SizingStretchProp))
		{
			(!node.GetAnimationData().Animation) ? ImGui::BeginDisabled() : void();
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					shade::ImGuiLayer::DrawFontIcon(u8"\xe8e3", 1, 0.5f);
				}
				ImGui::TableNextColumn();
				{
					if (ImGui::BeginTable("##TimeLineTable", 2, ImGuiTableFlags_SizingStretchProp))
					{
						ImGui::TableNextColumn();
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
						ImGui::SliderFloat("##Timeline", &node.GetAnimationData().CurrentPlayTime, 0.f, end);
						ImGui::PopItemWidth();
						ImGui::TableNextColumn();
						ImGuiLayer::ToggleButtonIcon("##IsLoopButton", &isloop, u8"\xea9b", 1, 0.7f);

						ImGui::EndTable();
					}
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

					if (ImGui::BeginTable("##StartEndTable", 2, ImGuiTableFlags_SizingStretchSame))
					{
						ImGui::TableNextColumn();
						{
							ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 20.f);
							ImGui::DragFloat("##Start", &start, 0.01f, 0.0f, end); ImGui::SameLine();
							ImGui::PopItemWidth();

							if (ImGuiLayer::IconButton("Reset", u8"\xe888", 1, 0.9f)) { start = 0.f; }
						}
						ImGui::TableNextColumn();
						{
							ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 30.f);
							ImGui::DragFloat("##End", &end, 0.01f, start, duration); ImGui::SameLine();
							ImGui::PopItemWidth();
							if (ImGuiLayer::IconButton("Reset", u8"\xe888", 1, 0.9f)) { end = duration; }
						}



						ImGui::EndTable();
					}
				}
			}
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					shade::ImGuiLayer::DrawFontIcon(u8"\xe988", 1, 0.7f, "Hello");
					ImGuiLayer::HelpMarker("#", std::format("FPS: {:.2f}", FPS).c_str());
				}
				ImGui::TableNextColumn();
				{
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
					ImGui::DragFloat("##Tiks", &ticksPerSecond, 0.01f, 0.0f, FLT_MAX);
					ImGui::PopItemWidth();
				}
			}
			(!node.GetAnimationData().Animation) ? ImGui::EndDisabled() : void();
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					shade::ImGuiLayer::DrawFontIcon(u8"\xe823", 1, 0.6f);
					ImGui::SameLine();
					{
						if (ImGuiLayer::IconButton("Open", u8"\xe85f", 1, 1.0f))
						{
							auto path = shade::FileDialog::OpenFile("Shade mesh(*.s_anim) \0*.s_anim\0");

							if (shade::file::File file = shade::file::FileManager::LoadFile(path.string(), "@s_anim"))
							{
								node.GetAnimationData().Animation = shade::Animation::CreateEXP();
								file.Read(node.GetAnimationData().Animation);
								node.ResetAnimationData(node.GetAnimationData().Animation);
							}
							else
								SHADE_CORE_WARNING("Couldn't open animation file, path ={0}", path);
						}
					}
				}
				ImGui::TableNextColumn();
				{
					if (ImGui::BeginTable("##SelectAnimationTable", 2, ImGuiTableFlags_SizingStretchProp))
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						{
							std::string buttonTitle = (!node.GetAnimationData().Animation || !node.GetAnimationData().Animation->GetAssetData()) ? "Not set" : node.GetAnimationData().Animation->GetAssetData()->GetId();
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
						ImGuiLayer::BeginWindowOverlay("##AnimationSearchOverlay",
							ImGui::GetWindowViewport(),
							std::size_t(&node), { ImGui::GetWindowSize().x, 200.f }, ImVec2{ ImGui::GetWindowPos().x + ImGui::GetStyle().IndentSpacing + 7.f, ImGui::GetCursorScreenPos().y + 2.f }, 1.0f,
							[&]() mutable
							{
								ImGuiLayer::InputTextCol("Search", m_Search);

								if (ImGui::BeginListBox("##SelectAsset", ImVec2{ ImGui::GetContentRegionAvail() }))
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
				ImGui::TableNextRow();
				{
					ImGui::TableNextColumn();
					{
						(!rootMotion) ? ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)) : void();
						shade::ImGuiLayer::DrawFontIcon(u8"\xf21d", 1, 0.7f);
						(!rootMotion) ? ImGui::PopStyleColor() : void();

						ImGui::SameLine();
						if (shade::ImGuiLayer::ToggleButton("RootMotion", &rootMotion))
						{
							node.ResetAnimationData(node.GetAnimationData());
						}
					}
					ImGui::TableNextColumn();
				}
			}
			ImGui::EndTable();
		}

	} ImGui::EndChild();

	if (ImGui::BeginChild("Sync groups", ImGui::GetContentRegionAvail(), true, 0))
	{
		ImGui::Text("Synchronization groups");
		std::vector<std::string> groups = { "None", "A", "B" };

		for (auto& [key, v] : syncGroups.GetGroups())
			groups.emplace_back(key);

		ImGuiLayer::IconButton("##Add", u8"\xc000", 1, 1.0f); ImGui::SameLine();



		ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
		if (ImGuiLayer::DrawCombo("##Sync Groups", syncGroup, groups, 0, 0))
		{
			syncGroups.AddGroup(syncGroup.c_str());
			syncGroups.GetGroup(syncGroup.c_str())->AddNode(&node);
		}
		ImGui::PopItemWidth();
	} ImGui::EndChild();
}

void graph_editor::PoseNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	//std::size_t hash = (endpoint.As<NodeValueType::Pose>()) ? endpoint.As<NodeValueType::Pose>()->GetAnimationHash() : 0;
	ImGui::Text("Pose"); //ImGui::SameLine(); ImGuiLayer::HelpMarker("#", std::format("{:x}", hash).c_str());
}

graph_editor::StateMachineNodeDelegate::StateMachineNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = STATE_MACHINE_NODE_COLOR;
}

void graph_editor::StateMachineNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	ImGui::Text(GetNode()->GetName().c_str());
}

void graph_editor::StateMachineNodeDelegate::ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes)
{
	static std::string search;
	ImGuiLayer::InputTextCol("Search", search);

	auto& node = GetNode()->As<state_machine::StateMachineNode>();

	if (ImGui::BeginListBox("##States", ImGui::GetContentRegionAvail()))
	{
		for (auto state : node.GetInternalNodes())
		{
			if (state->GetNodeType() != shade::animation::state_machine::TransitionNode::GetNodeStaticType())
			{
				if (state->GetName().find(search) != std::string::npos)
				{
					if (ImGui::RadioButton(std::string(state->GetName() + "##" + std::to_string(std::size_t(state))).c_str(), state == node.GetCurrentState()))
					{
						node.SetRootNode(state);
					}
				}
			}			
		}
		ImGui::EndListBox();
	}
}

void graph_editor::StateMachineNodeDelegate::ProcessPopup(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, const std::string& search)
{
	shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1", 1, 0.5f);
	ImGui::SameLine();
	ImGui::Text("States");
	ImGui::SameLine();
	shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1\xf2d1\xf2d1", 1, 0.5f);

	if (ImGui::Selectable("+ State"))
	{
		auto state = GetNode()->As<animation::state_machine::StateMachineNode>().CreateState("State");
		GetEditor()->InitializeRecursively(state, GetEditor()->GetNodes());
	}
	if (ImGui::Selectable("+ State machine"))
	{
		auto state = GetNode()->CreateNode<animation::state_machine::StateMachineNode>();
		GetEditor()->InitializeRecursively(state, GetEditor()->GetNodes());
	}
}

void graph_editor::StateMachineNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	switch (endpoint.GetType())
	{
	case NodeValueType::Float:
		ImGui::Text("Transition blend");
		break;
	case NodeValueType::Pose:
		ImGui::Text("Outcome pose");
		break;
	}
}

bool graph_editor::StateMachineNodeDelegate::Draw(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	const ImVec2 nodeRectMin = context->Offset + GetScreenPosition() * context->Scale.Factor;
	const ImVec2 nodeRectMax = nodeRectMin + (Style.Size - ImVec2{ 15.f, 15.f }) * context->Scale.Factor;
	const ImRect nodeRect(nodeRectMin, nodeRectMax);

	bool isClicked = false;

	ImGui::PushID(GetNode()->GetNodeIdentifier());

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x , GetScreenPosition().y } *context->Scale.Factor);

	DrawBody(context, graphStyle, nodes);

	if (GetNode()->GetParrentGraph()->GetNodeType() == shade::animation::state_machine::StateMachineNode::GetNodeStaticType()
		&& GetNode()->GetParrentGraph()->GetRootNode() == GetNode())
	{
		context->DrawList->AddCircleFilled(nodeRectMax, 5.f * context->Scale.Factor, ImGui::ColorConvertFloat4ToU32(Style.HeaderColor), 0);
	}

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor);

	ImGui::InvisibleButton(std::format("##NodeBox_{}", std::size_t(this)).c_str(), Style.Size * context->Scale.Factor);

	if (ImGui::IsItemActive())
	{
		isClicked = true;
	}

	MoveNode(context->Scale.Factor);

	ImGui::PopID();

	return isClicked;
}

void graph_editor::StateMachineNodeDelegate::DrawBody(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	const ImVec2 nodeRectMin = context->Offset + GetScreenPosition() * context->Scale.Factor;
	const ImVec2 nodeRectMax = nodeRectMin + Style.Size * context->Scale.Factor;

	const ImVec4 bckColor = IsSelected() ? ImVec4(
		Style.BackgroundColor.x * 1.5f,
		Style.BackgroundColor.y * 1.5f,
		Style.BackgroundColor.z * 1.5f,
		Style.BackgroundColor.w) : Style.BackgroundColor;

	context->DrawList->AddRectFilled(nodeRectMin, nodeRectMax, ImGui::ColorConvertFloat4ToU32(bckColor), graphStyle->Rounding);

	DrawHeader(context, graphStyle, nodes);
	DrawBorder(context, graphStyle, nodes);

	if(GetNode()->GetParrentGraph()->GetNodeType() != shade::animation::state_machine::StateMachineNode::GetNodeStaticType())
		DrawEndpoints(context, graphStyle, nodes);
	
	const ImVec2 endOfPrevRegion = ImGui::GetCursorScreenPos() - context->Offset;

	const ImVec2 newScreenPosition = context->Offset + ImVec2{ GetScreenPosition().x * context->Scale.Factor, endOfPrevRegion.y };
	const ImVec2 avalibleRegion{ nodeRectMax - newScreenPosition };

	ImGui::SetCursorScreenPos(newScreenPosition);

	ImGui::BeginGroup();
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, graphStyle->Padding * context->Scale.Factor);
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, graphStyle->CellPadding * context->Scale.Factor);
		ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4{ 0.f, 0.f, 0.f, 0.f });

		if (ImGui::BeginTableEx("##BodyContentTable", std::size_t(this) + 2000, 1, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuterV, { avalibleRegion.x, 0 }))
		{
			ImGui::TableNextRow();
			{
				ImGui::TableNextColumn();
				{
					ProcessBodyContent(context);
				}
			}
			ImGui::EndTable();
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}
	ImGui::EndGroup();

	//DrawFooter(context, graphStyle, nodes);

	Style.Size.y += ImGui::GetCursorScreenPos().y - nodeRectMax.y;

	ImGui::Dummy({ 0, 0 });
}

void graph_editor::StateMachineNodeDelegate::DrawNodes()
{
	TransitionEsteblish();
	DrawTransitions();
	GraphNodePrototype::DrawNodes();
}

void graph_editor::StateMachineNodeDelegate::DrawTransitions()
{
	for (auto& pNode : GetNode()->GetInternalNodes())
	{
		if (auto state = dynamic_cast<animation::state_machine::StateNode*>(pNode))
		{
			for (auto transition : state->GetTransitions())
			{
				auto source = GetEditor()->GetPrototypedNode(state);

				if (auto* destination = GetEditor()->GetPrototypedNode(transition->GetTransitionData().DestinationState))
				{
					const ImRect sourceRect = source->GetScaledRectangle(&GetEditor()->GetContext(), &GetEditor()->GetVisualStyle());
					const ImRect destinationRect = destination->GetScaledRectangle(&GetEditor()->GetContext(), &GetEditor()->GetVisualStyle());

					if (ImGuiGraphNodeRender::DrawTransition(GetEditor()->GetContext().DrawList,
						sourceRect,
						destinationRect,
						source->Style.BorderColor,
						3.f, GetEditor()->GetContext().Scale.Factor))
					{
						if (ImGui::IsMouseDoubleClicked(0))
						{
							GetEditor()->GetContext().CurrentNode = transition;
						}
						else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
						{
							state->RemoveTransition(transition); break;
						}
						else if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
						{
							ImGui::BeginTooltipEx(0, ImGuiWindowFlags_NoBackground);
							ImGui::PushStyleColor(ImGuiCol_Text, { 0.8f, 0.2f, 0.2f, 1.f });
							ImGuiLayer::DrawFontIcon(u8"\xf127", 1, 1.8f);
							ImGui::PopStyleColor();
							ImGui::EndTooltip();
						}
					}
				}
			}
		}
	}
}

void graph_editor::StateMachineNodeDelegate::TransitionEsteblish()
{
	auto& context = GetEditor()->GetContext();

	for (auto& pNode : GetNode()->GetInternalNodes())
	{
		auto& state = pNode->As<animation::state_machine::StateNode>();
		if (auto protNode = GetEditor()->GetPrototypedNode(&state))
		{
			if (!m_TransitionEstablish.From && protNode->IsSelected() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
			{
				m_TransitionEstablish.From = &state;
			}

			if (m_TransitionEstablish.From && m_TransitionEstablish.From != &state && protNode->IsSelected())
			{
				m_TransitionEstablish.To = &state;
			}
		}
	}

	if (m_TransitionEstablish.From && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
	{
		const ImVec2 nodeRectMin = context.Offset + GetEditor()->GetPrototypedNode(m_TransitionEstablish.From)->GetScreenPosition() * context.Scale.Factor;
		const ImVec2 nodeRectMax = nodeRectMin + (Style.Size - ImVec2{ 15.f, 15.f }) * context.Scale.Factor;
		const ImRect nodeRect(nodeRectMin, nodeRectMax);
		ImGuiGraphNodeRender::DrawArrowLine(context.DrawList, nodeRect.GetCenter(), ImGui::GetIO().MousePos, Style.HeaderColor, 0.95, 3.f, context.Scale.Factor);
	}

	if (m_TransitionEstablish.From && m_TransitionEstablish.To)
	{
		if (auto transition = m_TransitionEstablish.From->As<animation::state_machine::StateNode>().EmplaceTransition((animation::state_machine::StateNode*)m_TransitionEstablish.To))
		{
			GetEditor()->InitializeRecursively(transition, GetEditor()->GetNodes());
		}
		m_TransitionEstablish.Reset();
	}

	if (ImGui::IsKeyReleased(ImGuiKey_LeftCtrl))
	{
		m_TransitionEstablish.Reset();
	}



	//const ImVec2 nodeRectMin = context.Offset + GetScreenPosition() * context.Scale.Factor;
	//const ImVec2 nodeRectMax = nodeRectMin + (Style.Size - ImVec2{ 15.f, 15.f }) * context.Scale.Factor;
	//const ImRect nodeRect(nodeRectMin, nodeRectMax);
	//// Нужно эстеблишь добваить в отдельную фугкцию !!и в стейт машин наверно !
	//if (m_TransitionEstablish.To && m_TransitionEstablish.From)
	//{
	//	if (auto transition = m_TransitionEstablish.From->As<animation::state_machine::StateNode>().EmplaceTransition((animation::state_machine::StateNode*)m_TransitionEstablish.To))
	//	{
	//		GetEditor()->InitializeRecursively(transition, GetEditor()->GetNodes());
	//	}
	//	m_TransitionEstablish.Reset();
	//}
	//else if (m_TransitionEstablish.From && m_TransitionEstablish.From == GetNode() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
	//{
	//	ImGuiGraphNodeRender::DrawArrowLine(context.DrawList, nodeRect.GetCenter(), ImGui::GetIO().MousePos, Style.HeaderColor, 0.95, 3.f, context.Scale.Factor);
	//}
	//else if (ImGui::IsKeyReleased(ImGuiKey_LeftCtrl))
	//{
	//	m_TransitionEstablish.Reset();
	//}

	//bool isClicked = false;

	//DrawTransition(context, graphStyle, nodes);

	//ImGui::PushID(GetNode()->GetNodeIdentifier());

	//ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x , GetScreenPosition().y } *context->Scale.Factor);

	//if (GetNode() == GetNode()->GetParrentGraph()->As<shade::animation::state_machine::StateMachineNode>().GetCurrentState())
	//{
	//	context->DrawList->AddCircleFilled(nodeRectMax, 5.f * context->Scale.Factor, ImGui::ColorConvertFloat4ToU32(Style.HeaderColor), 0);
	//}

	//ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor);

	if (ImGui::InvisibleButton(std::format("##NodeBox_{}", std::size_t(this)).c_str(), Style.Size * context.Scale.Factor))
	{
		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && m_TransitionEstablish.From != nullptr && m_TransitionEstablish.From != GetNode())
		{
			m_TransitionEstablish.To = GetNode();
		}
		else
		{
			m_TransitionEstablish.From = GetNode();
		}
	}
}

void graph_editor::AnimationGraphDelegate::ProcessSideBar(const InternalContext* pContext, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes)
{
	auto inputs = GetNode()->As<AnimationGraph>().GetInputNodes();

	InternalContext	context; GraphVisualStyle  style; ImVec2 offset(5.f, 30.f);

	context.DrawList = ImGui::GetWindowDrawList();

	context.Offset = ImGui::GetWindowPos();

	static std::string search;
	ImGuiLayer::InputTextCol("Search", search);

	if (ImGui::BeginChild("Inputs", { 0, 0 }, true))
	{
		for (auto [name, input] : inputs)
		{
			if (name.find(search) != std::string::npos)
			{
				auto node = referNodes.at(std::size_t(input) ^ input->GetNodeIdentifier());

				const std::string id = std::to_string(node->GetNode()->GetNodeIdentifier());

				ImGui::PushStyleColor(ImGuiCol_ChildBg, node->Style.HeaderColor);
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, style.Rounding);
				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, style.Rounding);

				ImGui::Dummy({ 0.f, 5.f });

				if (ImGui::BeginChildEx("##Inputs", std::size_t(input), { 0.f, node->Style.Size.y / 2.f }, true, 0))
				{
					if (ImGui::BeginTable("##HeaderTable", 2, ImGuiTableFlags_SizingStretchProp))
					{
						ImGui::PushStyleColor(ImGuiCol_Text, Style.HeaderTextColor);
						ImGui::TableNextColumn();
						{
							std::string name = node->GetNode()->GetName();
							ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{ 0,0,0, Style.HeaderColor.w * 0.2f });
							ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
							if (ImGuiLayer::InputTextD(std::string(id + "##Name").c_str(), name))
							{
								GetNode()->As<AnimationGraph>().RenameInputNode(node->GetNode(), name);
							}
							ImGui::PopItemWidth();
							ImGui::PopStyleColor(1);

						}
						ImGui::TableNextColumn();
						{
							if (ImGui::BeginTable("Buttons", 2))
							{
								ImGui::TableNextColumn();
								if (ImGuiLayer::IconButton("##RemoveConenctions", u8"\xf127", 1, 1.0f))
								{
									node->GetNode()->GetGraphContext()->RemoveAllConnection(node->GetNode());
								}

								ImGui::TableNextColumn();

								if (ImGuiLayer::IconButton("##RemoveNode", u8"\xe85d", 1, 1.0f))
								{
									GetEditor()->SetToRemove(node->GetNode());
								}

								ImGui::EndTable();
							}
						}
						ImGui::PopStyleColor();

						ImGui::EndTable();
					}

					node->ProcessBodyContent(&context);


				} ImGui::EndChild();

				ImGui::PopStyleColor(); ImGui::PopStyleVar(2);
			}

		}


	} ImGui::EndChild();


	if (ImGui::GetCurrentWindow()->Rect().Contains(ImGui::GetIO().MousePos))
	{
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			ImGui::OpenPopup("##GraphEditorPopup");
	}

	if (ImGui::BeginPopup("##GraphEditorPopup"))
	{
		shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1", 1, 0.5f);
		ImGui::SameLine();
		ImGui::Text("Values");
		ImGui::SameLine();
		shade::ImGuiLayer::DrawFontIcon(u8"\xf2d1\xf2d1\xf2d1\xf2d1\xf2d1", 1, 0.5f);

		if (ImGui::Selectable("+ Int"))
		{
			CreateReferNode<graphs::IntNode>("Int :");
		}
		if (ImGui::Selectable("+ Float"))
		{
			CreateReferNode<graphs::FloatNode>("Float :");
		}
		if (ImGui::Selectable("+ Vec2 float"))
		{
			CreateReferNode<graphs::Vec2FloatNode>("Vec2 :");
		}
		if (ImGui::Selectable("+ String"))
		{
			CreateReferNode<graphs::StringNode>("String :");
		}
		if (ImGui::Selectable("+ Bool"))
		{
			CreateReferNode<graphs::BoolNode>("Bool :");
		}

		ImGui::Dummy(ImVec2{ 0, 10.f });

		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

bool graph_editor::AnimationGraphDelegate::DeleteReferNode(const std::string& name)
{
	return false;
}

graph_editor::IntEqualsNodeDelegate::IntEqualsNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = INT_VALUE_COLOR;
	Style.Title = "Int == ?";
}

void graph_editor::IntEqualsNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	if (type == graphs::Connection::Type::Input)
	{
		switch (endpoint.GetType())
		{
		case NodeValueType::Int:
		{
			if (identifier == 0)
			{
				ImGui::Text("Input");
			}
			if (identifier == 1)
			{
				ImGui::Text("Equals");
			}

			//ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragInt(std::string("##" + std::to_string(identifier)).c_str(), &endpoint.As<NodeValueType::Int>(), 0.1, 0, INT_MAX);
			ImGui::PopItemWidth();
			break;
		}
		}
	}
	else
	{
		switch (endpoint.GetType())
		{
		case NodeValueType::Bool:
		{
			ImGui::Text("Result ");
			break;
		}
		}
	}
}

graph_editor::IntNodeDelegate::IntNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = INT_VALUE_COLOR;
	Style.Title = "Int";
}

void graph_editor::IntNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{

}

void graph_editor::IntNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	auto node = GetNode();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::DragInt("##Value", &node->GetEndpoint<shade::graphs::Connection::Output>(0)->As<NodeValueType::Int>(), 0.1, -INT32_MAX, INT32_MAX);
	ImGui::PopItemWidth();
}

graph_editor::FloatEqualsNodeDelegate::FloatEqualsNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = FLOAT_VALUE_COLOR;
	Style.Title = "Float equals";
}

void graph_editor::FloatEqualsNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	if (type == graphs::Connection::Type::Input)
	{
		switch (endpoint.GetType())
		{
		case NodeValueType::Float:
		{
			if (identifier == 0)
			{
				ImGui::Text("Input");
			}
			if (identifier == 1)
			{
				ImGui::Text("Equals");
			}
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat(std::string("##" + std::to_string(identifier)).c_str(), &endpoint.As<NodeValueType::Float>(), 0.1, -FLT_MAX, FLT_MAX);
			ImGui::PopItemWidth();
			break;
		}
		}
	}
	else
	{
		switch (endpoint.GetType())
		{
		case NodeValueType::Bool:
		{
			ImGui::Text("Result ");
			break;
		}
		}
	}
}

graph_editor::FloatNodeDelegate::FloatNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = FLOAT_VALUE_COLOR;
	Style.Title = "Float";
}

void graph_editor::FloatNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{

}

void graph_editor::FloatNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	auto node = GetNode();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::DragFloat("##Value", &node->GetEndpoint<shade::graphs::Connection::Output>(0)->As<NodeValueType::Float>(), 0.1, -FLT_MAX, FLT_MAX);
	ImGui::PopItemWidth();
}

graph_editor::BoneMaskNodeDelegate::BoneMaskNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.Title = "Bone Mask";
	Style.HeaderColor = BONEMASK_VALUE_COLOR;
}

void  graph_editor::BoneMaskNodeDelegate::ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes)
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

						ImGui::Text(std::to_string(key).c_str()); ImGui::SameLine(); ImGui::Text(value.first.c_str());


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

	} ImGui::EndChild();

}

void graph_editor::BoneMaskNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	auto& node = GetNode()->As<BoneMaskNode>();

	//shade::ImGuiLayer::DrawFontIcon(u8"\xf29a", 1, 0.6f); ImGui::SameLine(); ImGui::Text(node.GetGraphContext()->As<animation::AnimationGraphContext>().Skeleton->GetAssetData()->GetId().c_str());
}

void graph_editor::BoneMaskNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	ImGui::Text("Result");
}

graph_editor::StringNodeDelegate::StringNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.Title = "String";
	Style.HeaderColor = STRING_VALUE_COLOR;
}

void graph_editor::StringNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	auto node = GetNode();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGuiLayer::InputTextD("##Value", node->GetEndpoint<shade::graphs::Connection::Output>(0)->As<NodeValueType::String>());
	ImGui::PopItemWidth();
}

graph_editor::BoolNodeDelegate::BoolNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.Title = "Bool";
	Style.HeaderColor = BOOL_VALUE_COLOR;
}

void graph_editor::BoolNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	auto node = GetNode();
	std::vector<std::string> elements = { "False", {"True"} };
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGuiLayer::DrawComboWithIndex("##Bool", (std::uint32_t&)node->GetEndpoint<shade::graphs::Connection::Output>(0)->As<NodeValueType::Bool>(), elements, 0, 0);
	ImGui::PopItemWidth();
}

graph_editor::TransitionNodeDelegate::TransitionNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
}

graph_editor::FloatScaleRangeDelegate::FloatScaleRangeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
}

void graph_editor::FloatScaleRangeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	if (type == graphs::Connection::Type::Input)
	{
		if (identifier == 0)
		{
			ImGui::Text("Value");
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat(std::string("##" + std::to_string(identifier)).c_str(), &endpoint.As<NodeValueType::Float>(), 0.1, -FLT_MAX, FLT_MAX);
			ImGui::PopItemWidth();
		}
		if (identifier == 1)
		{
			ImGui::Text("Min");
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat(std::string("##" + std::to_string(identifier)).c_str(), &endpoint.As<NodeValueType::Float>(), 0.1, -FLT_MAX, FLT_MAX);
			ImGui::PopItemWidth();
		}
		if (identifier == 2)
		{
			ImGui::Text("Max");
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat(std::string("##" + std::to_string(identifier)).c_str(), &endpoint.As<NodeValueType::Float>(), 0.1, -FLT_MAX, FLT_MAX);
			ImGui::PopItemWidth();
		}
		if (identifier == 3)
		{
			ImGui::Text("Range min");
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat(std::string("##" + std::to_string(identifier)).c_str(), &endpoint.As<NodeValueType::Float>(), 0.1, -FLT_MAX, FLT_MAX);
			ImGui::PopItemWidth();
		}
		if (identifier == 4)
		{
			ImGui::Text("Range max");
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat(std::string("##" + std::to_string(identifier)).c_str(), &endpoint.As<NodeValueType::Float>(), 0.1, -FLT_MAX, FLT_MAX);
			ImGui::PopItemWidth();
		}
	}
	else
	{
		ImGui::Text("Result ");
	}
}

void graph_editor::FloatScaleRangeDelegate::ProcessBodyContent(const InternalContext* context)
{
}

graph_editor::BlendTree2DNodeDelegate::BlendTree2DNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.Size = { 300, 200 };
}

void graph_editor::BlendTree2DNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	if (shade::ImGuiLayer::IconButton("##AddInput", u8"\xe9b2", 1, 0.8f))
	{
		GetNode()->As<animation::BlendTree2D>().AddInputPose();
	}

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Add new input");
		ImGui::EndTooltip();
	}
}

void graph_editor::BlendTree2DNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	auto node = GetNode();

	static ImRect rect;

	auto it = std::find_if(node->GetConnections().begin(), node->GetConnections().end(), [identifier](const shade::graphs::Connection& c)
		{
			return c.ConnectedToEndpoint == identifier;
		});

	if (type == graphs::Connection::Type::Input)
	{
		if (endpoint.GetType() == NodeValueType::Pose)
		{
			rect.Min.x = ImGui::GetCursorScreenPos().x - ImGui::GetStyle().ItemSpacing.x;
			rect.Min.y = ImGui::GetCursorScreenPos().y - ImGui::GetStyle().CellPadding.y;

			if (it != node->GetConnections().end())
			{
				ImGui::Text(it->PConnectedFrom->GetName().c_str());
			}
			else
			{
				ImGui::Text("Pose");
			}

			ImGui::SameLine(ImGui::GetContentRegionAvail().x);
			if (shade::ImGuiLayer::IconButton("##RemoveInput", u8"\xe8c4", 1, 1.0f))
			{
				GetNode()->As<animation::BlendTree2D>().RemoveInputPose(identifier);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("Remove input");
				ImGui::EndTooltip();
			}
		}
		if (endpoint.GetType() == NodeValueType::Vector2)
		{
			shade::ImGuiLayer::DrawFontIcon(u8"\xea02\xea03", 1, 0.65f); ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2.5f);
			ImGui::DragFloat(("##X" + std::to_string(identifier)).c_str(), &endpoint.As<NodeValueType::Vector2>().x, 0.002f, -FLT_MAX, FLT_MAX);
			ImGui::PopItemWidth();
			ImGui::SameLine();

			shade::ImGuiLayer::DrawFontIcon(u8"\xea04\xea01", 1, 0.65f); ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat(("##Y" + std::to_string(identifier)).c_str(), &endpoint.As<NodeValueType::Vector2>().y, 0.002f, -FLT_MAX, FLT_MAX);
			ImGui::PopItemWidth();

			if (identifier != 0)
			{
				rect.Max.y = ImGui::GetCursorScreenPos().y;
				rect.Max.x = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x + ImGui::GetStyle().CellPadding.x;
			}
			//ImGui::DragFloat2(std::string("YX##" + std::to_string(identifier)).c_str(), glm::value_ptr(endpoint.As<NodeValueType::Vector2>()), 0.001, -10, 10);
		}

		if (identifier != 0 && ((identifier % 2) == 0))
		{
			float alpha = ((identifier % 4) == 0) ? 0.0f : 0.2f;

			ImGui::GetWindowDrawList()->AddRect(rect.Min, rect.Max, ImGui::ColorConvertFloat4ToU32(ImVec4{ 0.5, 0.5, 0.5, 0.4 }), 4.f, 0, 1.5f);
			ImGui::GetWindowDrawList()->AddRectFilled(rect.Min, rect.Max, ImGui::ColorConvertFloat4ToU32(ImVec4{ 0.5, 0.5, 0.5, alpha }), 4.f);
		}

	}
	if (type == graphs::Connection::Type::Output)
	{
		ImGui::Text("Final pose");
	}

}

void graph_editor::BlendTree2DNodeDelegate::ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes)
{
	ImGui::Text("Blend Tree 2D");

	std::vector<glm::vec2>          points;

	for (std::size_t i = 1; i < GetNode()->GetEndpoints()[graphs::Connection::Type::Input].GetSize(); i += 2)
	{
		if (GetNode()->GetEndpoints()[graphs::Connection::Type::Input].At(i)->As<NodeValueType::Pose>())
		{
			points.push_back(GetNode()->GetEndpoints()[graphs::Connection::Type::Input].At(i + 1)->As<NodeValueType::Vector2>());
		}
	}

	const glm::vec2& inputs = GetNode()->GetEndpoints()[graphs::Connection::Type::Input].At(0)->As<NodeValueType::Vector2>();

	std::vector<float> weights = math::Calculate2DWeightsPolar(points, inputs);

	shade::GradientBand2DSpace("2D gradient", { ImGui::GetContentRegionAvail().x - 5.f, ImGui::GetContentRegionAvail().x - 5.f }, weights, points, inputs);
}

graph_editor::Vec2FloatDNodeDelegate::Vec2FloatDNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = VEC2_VALUE_COLOR;
}

void graph_editor::Vec2FloatDNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{

}

void graph_editor::Vec2FloatDNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	auto node = GetNode();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2.f);
	ImGui::DragFloat(("##X" + std::to_string(std::size_t(node))).c_str(), &node->GetEndpoint<graphs::Connection::Type::Output>(0)->As<NodeValueType::Vector2>().x, 0.002f, -FLT_MAX, FLT_MAX);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::DragFloat(("##Y" + std::to_string(std::size_t(node))).c_str(), &node->GetEndpoint<graphs::Connection::Type::Output>(0)->As<NodeValueType::Vector2>().y, 0.002f, -FLT_MAX, FLT_MAX);
	ImGui::PopItemWidth();
}
