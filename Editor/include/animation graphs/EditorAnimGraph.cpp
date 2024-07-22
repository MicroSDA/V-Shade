#include "shade_pch.h"
#include "EditorAnimGraph.h"

graph_editor::StateNodeDelegate::TransitionEstablish graph_editor::StateNodeDelegate::m_TransitionEstablish;

ImRect graph_editor::GraphNodePrototype::GetScaledRectangle(const InternalContext* context, const GraphVisualStyle* graphStyle)
{
	const ImVec2 scaledPosition = ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor;
	const ImVec2 pMin = (context->Offset + scaledPosition);
	//const ImVec2 pMax = (context->Offset + scaledPosition + (ImVec2{ Style.Size.x, graphStyle->HeaderHeight } *context->Scale.Factor));
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

	if (ImGui::BeginMenu("+ Blend"))
	{
		if (ImGui::MenuItem("Blend 2D"))
		{
			auto node = GetNode()->CreateNode<animation::BlendNode2D>();
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
}

void graph_editor::GraphNodePrototype::Draw(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	ImGui::PushID(GetNode()->GetNodeIdentifier());

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x , GetScreenPosition().y } *context->Scale.Factor);

	DrawBody(context, graphStyle, nodes);

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor);

	ImGui::InvisibleButton(std::format("##NodeBox_{}", std::size_t(this)).c_str(), Style.Size * context->Scale.Factor);

	if (ImGui::IsItemActive()) { this->SetSelected(true); }

	MoveNode(context->Scale.Factor);

	ImGui::PopID();
}

void graph_editor::GraphNodePrototype::DrawHeader(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	const ImVec2 scaledPosition = ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor;
	const ImVec2 pMin = (context->Offset + scaledPosition);
	const ImVec2 pMax = (context->Offset + scaledPosition + (ImVec2{ Style.Size.x, graphStyle->HeaderHeight } *context->Scale.Factor));

	context->DrawList->AddRectFilled(pMin, pMax, ImGui::ColorConvertFloat4ToU32(Style.HeaderColor), graphStyle->Rounding, ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersTopLeft);

	ImGui::SetCursorScreenPos(context->Offset + (graphStyle->Padding + ImVec2{ GetScreenPosition().x, GetScreenPosition().y }) * context->Scale.Factor);
	ImGui::PushStyleColor(ImGuiCol_Text, Style.HeaderTextColor);

	ImGui::SetWindowFontScale(ImGui::GetCurrentWindow()->FontWindowScale * 1.5f);

	ImGui::BeginGroup();
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0 ,0 ,0 ,1 });

		if (GetNode()->CanBeOpen())
		{
			if (shade::ImGuiLayer::IconButton(u8"\xe867", 1, 0.5f, context->Scale.Factor))
			{
				context->CurrentGraph = this;
			}

			ImGui::SameLine();
		}

		if (GetNode()->IsRenamable())
		{


			ImGui::PushItemWidth((Style.Size.x - 75.f) * context->Scale.Factor);

			std::string nodeName = GetNode()->GetName();
			if (ImGuiLayer::InputTextD("", nodeName))
			{
				GetNode()->SetName(nodeName);
			}

			ImGui::PopItemWidth();
		}
		else
		{
			ImGui::Text(GetNode()->GetName().c_str());
		}

		ImGui::SameLine((Style.Size.x - 45.f) * context->Scale.Factor);

		if (shade::ImGuiLayer::IconButton(u8"\xf127", 1, 0.3f, context->Scale.Factor))
		{
			GetNode()->GetGraphContext()->RemoveAllConnection(GetNode());
		}


		if (GetNode()->IsRemovable())
		{
			ImGui::SameLine();

			if (shade::ImGuiLayer::IconButton(u8"\xe85d", 1, 0.3f, context->Scale.Factor))
			{
				//GetEditor()->RemoveNode(GetNode()); // So there wee need to stop executing this node bacuse we are remove it, or make call back for removing !!
			}
		}

		ImGui::PopStyleColor();
	}
	ImGui::EndGroup();

	ImGui::SetWindowFontScale(ImGui::GetCurrentWindow()->FontWindowScale / 1.5f);

	ImGui::PopStyleColor(1);

	//return nodePosition.y + m_VisualStyle.HeaderHeight;
}

void graph_editor::GraphNodePrototype::DrawBorder(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	const ImVec2 scaledPosition = ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor;
	const ImVec2 pMin = (context->Offset + scaledPosition);
	const ImVec2 pMax = (context->Offset + scaledPosition + Style.Size * context->Scale.Factor);

	const ImVec4 color = IsSelected() ? ImVec4(
		Style.BorderColor.x * 1.6f,
		Style.BorderColor.y * 1.6f,
		Style.BorderColor.z * 1.6f,
		Style.BorderColor.w) : Style.BorderColor;

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


	/*if (ImGui::GetCursorScreenPos().y < GetScreenPosition().y + graphStyle->HeaderHeight * 2)
	{
		Style.Size *= 2.f;
	}*/




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

	auto& endpoints = node->GetEndpoints();
	auto inputs = endpoints[shade::graphs::Connection::Input].begin();
	auto outputs = endpoints[shade::graphs::Connection::Output].begin();
	auto connections = graphContext->Connections.find(node);
	bool needToOpenPopup = false;

	static std::function<void(graphs::BaseNode*)> Callback;

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

				ProcessEndpoint(endpoint, shade::graphs::Connection::Input, *inputs->first);

				const auto connections = node->GetGraphContext()->Connections.find(node);

				bool referConnection = false;
				// If connections exist at this endpoint
				if (connections != node->GetGraphContext()->Connections.end())
				{
					auto it = std::find_if(connections->second.begin(), connections->second.end(), [endpoint](const shade::graphs::Connection& c)
						{
							return c.InputEndpoint == endpoint;
						});

					if (it != connections->second.end())
					{
						// If connection with refer endpoint
						if (GetEditor()->GetPrototypedReferNode((*it).OutputNode))
						{
							referConnection = true;
						}
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
						Style.InputEndpointsColor,
						Style.InputEndpointsColorHovered))
					{
						if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && context->ConnectionEstablish.IsOutPutSelect)
						{
							context->ConnectionEstablish.IsInputSelect = true;
							context->ConnectionEstablish.InputNode = node;
							context->ConnectionEstablish.InputEndpointIdentifier = endpoint;
						}
						else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
						{
							Callback = [node, endpoint](graphs::BaseNode* pNode)
								{
									node->ConnectNodes(node, endpoint, pNode, 0);
								};

							needToOpenPopup = true;
						}
					}
				}

				if (context->ConnectionEstablish.IsInputSelect && context->ConnectionEstablish.InputNode == node)
				{
					context->ConnectionEstablish.InputEndpointScreenPosition = endpointScreenPosition;
				}

				if (connections != graphContext->Connections.end())
				{
					graphs::EndpointIdentifier endpoint = std::distance(endpoints[shade::graphs::Connection::Input].begin(), inputs);

					auto connection = std::find_if(connections->second.begin(), connections->second.end(), [endpoint](const graphs::Connection& connection)
						{
							return (connection.InputEndpoint == endpoint);
						});

					if (connection != connections->second.end())
					{
						connection->InputScreenPosition = glm::vec2(endpointScreenPosition.x, endpointScreenPosition.y);
					}
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

				ProcessEndpoint(endpoint, shade::graphs::Connection::Output, *outputs->first);

				if (ImGuiGraphNodeRender::DrawEndpoint(context->DrawList,
					context->Offset,
					graphStyle->EndpointRadius,
					context->Scale.Factor,
					endpointScreenPosition,
					Style.OutPutEndpointsColor,
					Style.OutPutEndpointsColorHovered))
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

				for (auto& graphConnections : graphContext->Connections)
				{
					graphs::EndpointIdentifier endpoint = std::distance(endpoints[shade::graphs::Connection::Output].begin(), outputs);

					auto connection = std::find_if(graphConnections.second.begin(), graphConnections.second.end(), [endpoint, node](const graphs::Connection& connection)
						{
							return (connection.OutputNode == node && connection.OutputEndpoint == endpoint);

						});

					if (connection != graphConnections.second.end())
					{
						connection->OutputScreenPosition = glm::vec2(endpointScreenPosition.x, endpointScreenPosition.y);
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
				if (context->CurrentGraph)
				{
					context->CurrentGraph->GetNode()->AddReferNode(referNode);
					Callback(referNode);
					Callback = {}; // reset
				}
			}

		}
		ImGui::EndPopup();
	}

}

void graph_editor::GraphNodePrototype::DrawConnections(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	auto& connections = nodes.begin()->second->GetNode()->GetGraphContext()->Connections;

	if (connections.find(GetNode()) != connections.end())
	{
		auto& nodeConnections = connections.at(GetNode());

		for (const auto& connection : nodeConnections)
		{
			if (nodes.find(std::size_t(connection.OutputNode)) == nodes.end())
			{
				ImGuiGraphNodeRender::DrawReferNodeConnection(context->DrawList, context->Offset, context->Scale.Factor, ImVec2{ connection.InputScreenPosition.x, connection.InputScreenPosition.y },
					ImVec2{ 30.f, 30.f },
					ImVec4{ 0.7f, 0.7f, 0.1f, 1.f }, 1.f);
			}
			else
			{
				ImGuiGraphNodeRender::DrawConnection(
					context->DrawList,
					context->Offset,
					context->Scale.Factor,
					ImVec2{ connection.InputScreenPosition.x,  connection.InputScreenPosition.y },
					ImVec2{ connection.OutputScreenPosition.x, connection.OutputScreenPosition.y },

					ImVec4{ 1, 1, 1, 1 },
					graphStyle->ConnectionThickness);
			}
		}
	}

	if (context->ConnectionEstablish.IsOutPutSelect && context->ConnectionEstablish.OutPutNode == GetNode())
	{
		auto& endpoints = GetNode()->GetEndpoints();
		const auto& endpoint = endpoints[EndpointPrototype::EndpointType::Output].At(context->ConnectionEstablish.OutPutEndpointIdentifier);

		ImGuiGraphNodeRender::DrawConnection(
			context->DrawList,
			context->Offset,
			context->Scale.Factor,
			ImGui::GetMousePos() - context->Offset,
			context->ConnectionEstablish.OutPutEndpointScreenPosition,
			Style.InputEndpointsColor,
			graphStyle->ConnectionThickness);
	}
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


void graph_editor::GraphEditor::InitializeRecursively(graphs::BaseNode* pNode, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	// TODO:: state machine
	if (dynamic_cast<state_machine::StateNode*>(pNode))
	{
		state_machine::StateNode* state = reinterpret_cast<state_machine::StateNode*>(pNode);
		nodes[std::size_t(pNode)] = new StateNodeDelegate(pNode, this);
		// Transitions nodes aka logic
		for (auto transition : state->GetTransitions())
		{
			InitializeRecursively(transition, nodes);
		}
		// Play animation nodes aka
		for (auto node : state->GetNodes())
		{
			InitializeRecursively(node, nodes);
		}
	}
	else if (dynamic_cast<state_machine::TransitionNode*>(pNode))
	{
		// Transition as graph node, contains nodes
		state_machine::TransitionNode* transition = reinterpret_cast<state_machine::TransitionNode*>(pNode);
		nodes[std::size_t(pNode)] = new TransitionNodeDelegate(pNode, this);

		for (auto node : transition->GetNodes())
		{
			InitializeRecursively(node, nodes);
		}
	}
	else if (dynamic_cast<state_machine::StateMachineNode*>(pNode))
	{
		// Transition as graph node, contains nodes
		state_machine::StateMachineNode* machine = reinterpret_cast<state_machine::StateMachineNode*>(pNode);
		nodes[std::size_t(pNode)] = new StateMachineNodeDeligate(pNode, this);

		// Aka states
		for (auto state : machine->GetNodes())
		{
			InitializeRecursively(state, nodes);
		}
	}
	else if (dynamic_cast<graphs::BaseNode*>(pNode))
	{
		// Probablu value node is not needed
		if (dynamic_cast<shade::graphs::ValueNode*>(pNode))
		{
			shade::graphs::ValueNode* value = reinterpret_cast<shade::graphs::ValueNode*>(pNode);
			nodes[std::size_t(pNode)] = new ValueNodeDelegate(pNode, this);
		}
		if (dynamic_cast<shade::animation::OutputPoseNode*>(pNode))
		{
			shade::animation::OutputPoseNode* value = reinterpret_cast<shade::animation::OutputPoseNode*>(pNode);
			nodes[std::size_t(pNode)] = new OutputPoseNodeDelegate(pNode, this);
		}
		if (dynamic_cast<shade::animation::BlendNode2D*>(pNode))
		{
			shade::animation::BlendNode2D* value = reinterpret_cast<shade::animation::BlendNode2D*>(pNode);
			nodes[std::size_t(pNode)] = new BlendNode2DNodeDelegate(pNode, this);
		}
		if (dynamic_cast<shade::animation::state_machine::OutputTransitionNode*>(pNode))
		{
			shade::animation::state_machine::OutputTransitionNode* value = reinterpret_cast<shade::animation::state_machine::OutputTransitionNode*>(pNode);
			nodes[std::size_t(pNode)] = new OutputTransitionNodeDelegate(pNode, this);
		}
		if (dynamic_cast<shade::animation::PoseNode*>(pNode))
		{
			shade::animation::PoseNode* value = reinterpret_cast<shade::animation::PoseNode*>(pNode);
			nodes[std::size_t(pNode)] = new PoseNodeDelegate(pNode, this);
		}
		if (dynamic_cast<shade::graphs::IntEqualsNode*>(pNode))
		{
			shade::graphs::IntEqualsNode* value = reinterpret_cast<shade::graphs::IntEqualsNode*>(pNode);
			nodes[std::size_t(pNode)] = new IntEqualsNodeDelegate(pNode, this);
		}
		if (dynamic_cast<shade::graphs::IntNode*>(pNode))
		{
			shade::graphs::IntNode* value = reinterpret_cast<shade::graphs::IntNode*>(pNode);
			nodes[std::size_t(pNode)] = new IntNodeDelegate(pNode, this);
		}
		if (dynamic_cast<shade::graphs::FloatEqualsNode*>(pNode))
		{
			shade::graphs::FloatEqualsNode* value = reinterpret_cast<shade::graphs::FloatEqualsNode*>(pNode);
			nodes[std::size_t(pNode)] = new FloatEqualsNodeDelegate(pNode, this);
		}
		if (dynamic_cast<shade::graphs::FloatNode*>(pNode))
		{
			shade::graphs::FloatNode* value = reinterpret_cast<shade::graphs::FloatNode*>(pNode);
			nodes[std::size_t(pNode)] = new FloatNodeDelegate(pNode, this);
		}
		if (dynamic_cast<shade::animation::BoneMaskNode*>(pNode))
		{
			shade::animation::BoneMaskNode* value = reinterpret_cast<shade::animation::BoneMaskNode*>(pNode);
			nodes[std::size_t(pNode)] = new BoneMaskNodeDelegate(pNode, this);
		}
	}
}

graph_editor::GraphNodePrototype* graph_editor::GraphEditor::GetPrototypedNode(graphs::BaseNode* pNode)
{
	auto node = m_Nodes.find(std::size_t(pNode));

	if (node != m_Nodes.end())
		return node->second;
	else
		return nullptr;
}

graph_editor::GraphNodePrototype* graph_editor::GraphEditor::GetPrototypedReferNode(graphs::BaseNode* pNode)
{
	auto node = m_ReferNodes.find(std::size_t(pNode));

	if (node != m_ReferNodes.end())
		return node->second;
	else
		return nullptr;
}

bool graph_editor::GraphEditor::RemoveNode(graphs::BaseNode* pNode)
{
	auto it = m_Nodes.find(std::size_t(pNode));

	if (it != m_Nodes.end())
	{
		m_Nodes.erase(it);
		pNode->GetParrentGraph()->RemoveNode(pNode);

		return true;
	}

	return false;
}

void graph_editor::GraphEditor::Initialize(AnimationGraph* pGraph)
{
	// Add to nodes !!
	m_Nodes[std::size_t(pGraph)] = new AnimationGraphDeligate(pGraph, this);

	m_Context.CurrentGraph = m_Nodes.at(std::size_t(pGraph));
	m_pRootGraph = pGraph;

	for (auto node : m_pRootGraph->GetNodes())
	{
		InitializeRecursively(node, m_Nodes);
	}

	for (auto [name, node] : pGraph->GetInputNodes())
	{
		InitializeRecursively(node, m_ReferNodes);
	}

	//m_pGraph->Initialize();
}

bool graph_editor::GraphEditor::Edit(const char* title, const ImVec2& size)
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
				ImVec2(300, ImGui::GetContentRegionAvail().y),
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
					m_Context.CurrentGraph->ProcessSideBar(&m_Context, m_Nodes, m_ReferNodes);
				}


				ImGui::EndChild();
			}

			ImGui::SameLine();

			const ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
			const ImVec2 windowPos = cursorPosition, scrollRegionLocalPos(0, 0);
			m_Context.CanvasSize = ImGui::GetContentRegionAvail();
			m_Context.CanvasRect = ImRect{ windowPos, windowPos + m_Context.CanvasSize };
			m_Context.Offset = cursorPosition + (m_Context.CanvasPosition * m_Context.Scale.Factor);

			if (ImGui::BeginChild("##ImGuiGraphPrototype::ScrollRegion",
				ImVec2(0, 0),
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
					if (ImGui::IsItemActive())
					{
						if (m_pSelectedNode != nullptr)
							m_pSelectedNode->SetSelected(false);

						m_pSelectedNode = nullptr;

						m_Context.ConnectionEstablish.Reset();
					}

					/*if (m_pSelectedNode)
					{
						if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && m_pSelectedNode->GetScaledRectangle(&m_Context, &m_VisualStyle).Contains(ImGui::GetIO().MousePos))
							ImGui::OpenPopup("##NodeEditPopup");

						if (ImGui::BeginPopup("##NodeEditPopup"))
						{
							m_pSelectedNode->ProcessPopup(&m_Context, m_Nodes);

							if (ImGui::Button("Close"))
								ImGui::CloseCurrentPopup();

							ImGui::EndPopup();
						}
					}*/

					DrawNodes();
					DrawConnections();
				}

				// Reset style back
				ImGui::GetStyle() = unscaledStyle;
				ImGui::EndChild();

			}

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

				m_Context.CurrentGraph->ProcessPopup(&m_Context, m_Nodes, search);
				PopupMenu();

				ImGui::EndPopup();
			}
			ImGui::End();
		}
	}
	ImGui::PopStyleColor();

	ImGuiLayer::BeginWindowOverlay("Path",
		ImGui::GetWindowViewport(), 123124,
		ImVec2{ m_Context.CanvasSize.x, ImGui::GetContentRegionAvail().y + 5.f },
		m_Context.CanvasRect.Min, 0.0f, [&]() {DrawPathRecursevly(m_Context.CurrentGraph); });

	if (m_Context.ConnectionEstablish.IsInputSelect && m_Context.ConnectionEstablish.IsOutPutSelect)
	{
		m_pRootGraph->ConnectNodes(
			m_Context.ConnectionEstablish.InputNode,
			m_Context.ConnectionEstablish.InputEndpointIdentifier,
			m_Context.ConnectionEstablish.OutPutNode,
			m_Context.ConnectionEstablish.OutPutEndpointIdentifier);
		m_Context.ConnectionEstablish.Reset();
	}

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
	std::size_t CurrentChannel = m_Context.CurrentGraph->GetNode()->GetNodes().size() - 1;

	//m_Context.DrawList->ChannelsSplit((m_Context.CurrentGraph->GetNode()->GetNodes().size()) ? m_Context.CurrentGraph->GetNode()->GetNodes().size() + 1 : 1);

	for (auto node : m_Context.CurrentGraph->GetNode()->GetNodes())
	{
		if (m_pSelectedNode != nullptr && node != m_pSelectedNode->GetNode())
		{
			m_Nodes.at(std::size_t(node))->SetSelected(false);
		}

		//m_Context.DrawList->ChannelsSetCurrent(CurrentChannel);

		GraphNodePrototype* prototype = GetPrototypedNode(node);

		prototype->Draw(&m_Context, &m_VisualStyle, m_Nodes);

		if (ImGui::IsItemActive()) { m_pSelectedNode = prototype; }

		//if (m_pSelectedNode && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		//{
		//	m_Path.push_back(m_Context.CurrentGraph); // Change container type
		//	m_Context.CurrentGraph = prototype;
		//}

		CurrentChannel--;
	}

	//for (auto node : m_Context.CurrentGraph->GetNode()->GetReferNodes())
	//{
	//	//m_Context.DrawList->ChannelsSetCurrent(CurrentChannel);

	//	GraphNodePrototype* prototype = GetPrototypedReferNode(node);

	//	prototype->Draw(&m_Context, &m_VisualStyle, m_Nodes);

	//	if (ImGui::IsItemActive()) { m_pSelectedNode = prototype; }

	//	if (m_pSelectedNode && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
	//	{
	//		m_Path.push_back(m_Context.CurrentGraph); // Change container type
	//		m_Context.CurrentGraph = prototype;
	//	}

	//	CurrentChannel--;
	//}

	//m_Context.DrawList->ChannelsMerge();

}

void graph_editor::GraphEditor::DrawConnections()
{
	if (m_Context.CurrentGraph)
	{
		for (auto& node : m_Context.CurrentGraph->GetNode()->GetNodes())
		{
			GetPrototypedNode(node)->DrawConnections(&m_Context, &m_VisualStyle, m_Nodes);
		}

		for (auto& node : m_Context.CurrentGraph->GetNode()->GetReferNodes())
		{
			auto& connections = m_ReferNodes.begin()->second->GetNode()->GetGraphContext()->Connections;

			if (connections.find(node) != connections.end())
			{
				auto& nodeConnections = connections.at(node);

				for (const auto& connection : nodeConnections)
				{
					const float size = 150.f;
					const ImVec2	p1 = m_Context.Offset + ImVec2{ connection.OutputScreenPosition.x - size, connection.OutputScreenPosition.y + size },
						p2 = m_Context.Offset + ImVec2{ connection.OutputScreenPosition.x - size, connection.OutputScreenPosition.y - size },
						p3 = m_Context.Offset + ImVec2{ connection.OutputScreenPosition.x, connection.OutputScreenPosition.y };

					m_Context.DrawList->AddTriangle(p1, p2, p3, ImGui::ColorConvertFloat4ToU32({ 1,1,1,1 }), 5.f);


					/*ImGuiGraphNodeRender::DrawConnection(
						m_Context.DrawList,
						m_Context.Offset,
						m_Context.Scale.Factor,
						ImVec2{ connection.InputScreenPosition.x,  connection.InputScreenPosition.y },
						ImVec2{ connection.OutputScreenPosition.x, connection.OutputScreenPosition.y },

						ImVec4{ 1, 1, 1, 1 },
						m_VisualStyle.ConnectionThickness);*/
				}
			}

			GetPrototypedReferNode(node)->DrawConnections(&m_Context, &m_VisualStyle, m_ReferNodes);
		}
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
		if (ImGui::MenuItem("Int"))  InitializeRecursively(m_Context.CurrentGraph->GetNode()->CreateNode<graphs::IntNode>(), m_Nodes);
		if (ImGui::MenuItem("Int equals"))  InitializeRecursively(m_Context.CurrentGraph->GetNode()->CreateNode<graphs::IntEqualsNode>(), m_Nodes);
		ImGui::BeginDisabled();
		ImGui::MenuItem("Int not equals");
		ImGui::MenuItem("Int higher");
		ImGui::MenuItem("Int lesser");
		ImGui::EndDisabled();

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("+ Float"))
	{

		if (ImGui::MenuItem("+ Float"))  InitializeRecursively(m_Context.CurrentGraph->GetNode()->CreateNode<graphs::FloatNode>(), m_Nodes);
		if (ImGui::MenuItem("Float equals"))  InitializeRecursively(m_Context.CurrentGraph->GetNode()->CreateNode<graphs::FloatEqualsNode>(), m_Nodes);

		ImGui::BeginDisabled();
		ImGui::MenuItem("Float not equals");
		ImGui::MenuItem("Float higher");
		ImGui::MenuItem("Float lesser");
		ImGui::EndDisabled();

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("+ Bool"))
	{
		ImGui::BeginDisabled();

		ImGui::MenuItem("Bool");
		ImGui::MenuItem("And");
		ImGui::MenuItem("Or");
		ImGui::MenuItem("Not");
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
			if (m_Context.CurrentGraph)
			{
				m_Context.CurrentGraph->GetNode()->AddReferNode(node);
			}
		}

	}
	ImGui::EndDisabled();

	ImGui::Dummy({ 0,15.f });
	ImGui::Separator();
	if (ImGui::Button("Close"))
		ImGui::CloseCurrentPopup();
}

void graph_editor::GraphEditor::DrawPathRecursevly(GraphNodePrototype* pNode)
{
	if (m_Nodes.find(std::size_t(pNode->GetNode())) != m_Nodes.end())
	{
		if (pNode->GetNode()->GetParrentGraph() != nullptr)
		{
			DrawPathRecursevly(m_Nodes.at(std::size_t(pNode->GetNode()->GetParrentGraph())));
		}

		if (ImGui::Button(m_Nodes.at(std::size_t(pNode->GetNode()))->Style.Title.c_str()))
		{
			m_Context.CurrentGraph = m_Nodes.at(std::size_t(pNode->GetNode()));
		}

		ImGui::SameLine();
	}
}


//
//
//editor_animation_graph::OutputPoseNodeDeligate::OutputPoseNodeDeligate(GraphNode::NodeIdentifier id, SharedPointer<GraphNode> node) :
//	BaseNodeDeligate(id, node)
//{
//	Style.Title = "Result";
//	Style.HeaderColor = ImVec4{ 0.7, 0.7, 0.7, 1.0 };
//	Style.Size = ImVec2{ 120.f, 100.f };
//}
//
//void editor_animation_graph::OutputPoseNodeDeligate::ProcessEndpoint(const GraphNode::EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint)
//{
//
//	auto& node = GetNode()->As<OutputPoseNode>();
//	const EndpointPrototype::EndpointType type = endpoint.GetType();
//
//	switch (endpoint.GetType()) // TODO: Rename -  GetConenctionType
//	{
//		case EndpointPrototype::Input:
//		{
//			endpoint.Style.Color = ImVec4{ 0.5f, 0.7f, 0.f, 1.f };
//			endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };
//
//			std::size_t hash = 0;
//			auto pose = node.GetEndpoint<GraphNode::Connection::Input>(endpointIdentifier)->As<NodeValueType::Pose>();
//			if (pose) hash = pose->GetAnimationHash();
//
//			ImGui::Text("Pose");
//			ImGuiLayer::HelpMarker("#", std::format("{:x}", hash).c_str());
//		}
//	}
//}
//
//void editor_animation_graph::OutputPoseNodeDeligate::ProcessBodyContent()
//{
//
//}
//
//editor_animation_graph::PoseNodeDeligate::PoseNodeDeligate(GraphNode::NodeIdentifier id, SharedPointer<GraphNode> node) :
//	BaseNodeDeligate(id, node)
//{
//	Style.Title = "Pose";
//}
//
//void editor_animation_graph::PoseNodeDeligate::ProcessSideBar()
//{
//	PoseNode& node = GetNode()->As<animation::PoseNode>();
//
//	float& start = node.GetAnimationData().Start;
//	float& end = node.GetAnimationData().End;
//	float& duration = node.GetAnimationData().Duration;
//	float& ticksPerSecond = node.GetAnimationData().TicksPerSecond;
//
//	ImGui::Text("Node: Pose"); ImGui::Separator();
//
//	if (ImGui::BeginChildEx("Node: Pose", std::size_t(&node), ImGui::GetContentRegionAvail(), true, 0))
//	{
//		const ImVec2 windowPosition = ImGui::GetWindowPos();
//		const ImVec2 windowSize = ImGui::GetWindowSize();
//
//		if (ImGui::BeginTable("Table", 2, ImGuiTableFlags_SizingStretchProp))
//		{
//			(!node.GetAnimationData().Animation) ? ImGui::BeginDisabled() : void();
//			ImGui::TableNextRow();
//			{
//				ImGui::TableNextColumn();
//				{
//					shade::ImGuiLayer::DrawFontIcon(u8"\xe889", 1, 0.5f);
//				}
//				ImGui::TableNextColumn();
//				{
//					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
//					ImGui::SliderFloat("##Timeline", &node.GetAnimationData().CurrentPlayTime, 0.f, duration);
//					ImGui::PopItemWidth();
//				}
//			}
//			ImGui::TableNextRow();
//			{
//				ImGui::TableNextColumn();
//				{
//					shade::ImGuiLayer::DrawFontIcon(u8"\xe84c", 1, 0.5f); ImGui::SameLine(); ImGui::Text("/"); ImGui::SameLine(); shade::ImGuiLayer::DrawFontIcon(u8"\xf11e", 1, 0.5f);
//				}
//				ImGui::TableNextColumn();
//				{
//					glm::vec2 startEnd = { start , end };
//					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
//					if (ImGui::DragFloat2("##StartEnd", glm::value_ptr(startEnd), 0.01f, 0.0f, duration))
//					{
//						start = startEnd.x; end = startEnd.y;
//					}
//					ImGui::PopItemWidth();
//				}
//			}
//			ImGui::TableNextRow();
//			{
//				ImGui::TableNextColumn();
//				{
//					shade::ImGuiLayer::DrawFontIcon(u8"\xea8b", 1, 0.5f);
//				}
//				ImGui::TableNextColumn();
//				{
//					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
//					ImGui::DragFloat("##Tiks", &ticksPerSecond, 0.01f, 0.0f);
//					ImGui::PopItemWidth();
//				}
//			}
//			(!node.GetAnimationData().Animation) ? ImGui::EndDisabled() : void();
//			ImGui::TableNextRow();
//			{
//				ImGui::TableNextColumn();
//				{
//					shade::ImGuiLayer::DrawFontIcon(u8"\xe823", 1, 0.5f);
//				}
//				ImGui::TableNextColumn();
//				{
//					if (ImGui::BeginTable("##SelectAnimationTable", 2, ImGuiTableFlags_SizingStretchProp))
//					{
//						ImGui::TableNextRow();
//						ImGui::TableNextColumn();
//						{
//							std::string buttonTitle = (!node.GetAnimationData().Animation) ? "Not set" : node.GetAnimationData().Animation->GetAssetData()->GetId();
//							ImGui::BeginDisabled();
//							ImGui::Button(buttonTitle.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0.f });
//							ImGui::EndDisabled();
//						}
//						ImGui::TableNextColumn();
//						{
//							if (ImGui::ArrowButton("##OpenPopup", ImGuiDir_Down))
//							{
//								m_IsAnimationPopupActive = (!m_IsAnimationPopupActive) ? true : false;
//							}
//						}
//						ImGui::EndTable();
//					}
//
//					if (m_IsAnimationPopupActive)
//					{
//						ImGuiLayer::BeginWindowOverlay("##AnimationSearchOverlay", ImGui::GetWindowViewport(), std::size_t(&node), ImVec2{ windowSize.x - 10.f, 0.f }, ImVec2{ windowPosition.x + 5.f, ImGui::GetCursorScreenPos().y + 5.f }, 0.3f,
//							[&]() mutable
//							{
//								ImGuiLayer::InputTextCol("Search", m_Search);
//
//								if (ImGui::BeginListBox("##SelectAnimation", ImVec2{ ImGui::GetContentRegionAvail().x, 0.f }))
//								{
//									for (const auto& assetData : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
//									{
//										if (assetData.second->GetType() == shade::AssetMeta::Type::Animation && assetData.first.find(m_Search) != std::string::npos)
//										{
//											if (ImGui::Selectable(assetData.first.c_str(), false))
//											{
//												shade::AssetManager::GetAsset<shade::Animation,
//													shade::BaseAsset::InstantiationBehaviour::Synchronous>(assetData.first,
//														shade::AssetMeta::Category::Secondary,
//														shade::BaseAsset::LifeTime::KeepAlive,
//														[&](auto& animation) mutable
//														{
//															node.ResetAnimationData(animation);
//														});
//
//												m_IsAnimationPopupActive = false;
//											}
//
//										}
//									}
//									ImGui::EndListBox();
//								}
//
//							});
//					}
//				}
//			}
//
//			ImGui::EndTable();
//		}
//		ImGui::EndChild();
//	}
//}
//
//void editor_animation_graph::PoseNodeDeligate::ProcessEndpoint(const GraphNode::EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint)
//{
//	auto& node = GetNode()->As<PoseNode>();
//	const EndpointPrototype::EndpointType type = endpoint.GetType();
//
//	switch (endpoint.GetType()) // TODO: Rename -  GetConenctionType
//	{
//	case EndpointPrototype::Output:
//	{
//		endpoint.Style.Color = Style.HeaderColor;
//		endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };
//
//		ImGui::Text("Pose");
//	}
//	}
//}
//
//void editor_animation_graph::PoseNodeDeligate::ProcessBodyContent()
//{
//	PoseNode& node = GetNode()->As<animation::PoseNode>();
//	float& start = node.GetAnimationData().Start;
//	float& end = node.GetAnimationData().End;
//	float& duration = node.GetAnimationData().Duration;
//	float& currentTime = node.GetAnimationData().CurrentPlayTime;
//
//	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
//	
//	{
//
//		// Calculate the normalized value between 0 and 1
//		float result = (currentTime - start) / (end - start);
//		// Clamp the result to make sure it stays between 0 and 1
//		result = glm::clamp(result, 0.0f, 1.0f);
//
//		ImGui::ProgressBar(result, ImVec2(0.f, 0.f), std::format("{:.1f}", currentTime).c_str());
//	}
//
//	ImGui::PopItemWidth();
//
//	ImGui::Spacing();
//	ImGui::Spacing();
//
//	std::string animation = (!node.GetAnimationData().Animation) ? "  Not select" : node.GetAnimationData().Animation->GetAssetData()->GetId();
//	shade::ImGuiLayer::DrawFontIcon(u8"\xe823", 1, 0.5f); 
//	ImGui::SameLine(); ImGui::Text(animation.c_str());
//
//}
//
//editor_animation_graph::BlendNodeDeligate::BlendNodeDeligate(GraphNode::NodeIdentifier id, SharedPointer<GraphNode> node):
//	BaseNodeDeligate(id, node)
//{
//	Style.Title = "Blend";
//}
//
//void editor_animation_graph::BlendNodeDeligate::ProcessBodyContent()
//{
//	
//}
//
//void editor_animation_graph::BlendNodeDeligate::ProcessEndpoint(const GraphNode::EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint)
//{
//	auto& node = GetNode()->As<BlendNode2D>();
//	const EndpointPrototype::EndpointType type = endpoint.GetType();
//
//	switch (type)
//	{
//	case EndpointPrototype::Input:
//	{
//		if (endpointIdentifier == 0) // Blend Weight
//		{
//			ImGui::Text("Blend Weight");
//			endpoint.Style.Color = ImVec4{ 0.3, 0.5, 1.0, 1.0 };
//			endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };
//		}
//		if (endpointIdentifier == 1)
//		{
//			endpoint.Style.Color = ImVec4{ 0.6, 0.4, 0.0, 1.0 };
//			endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };
//			ImGui::Text("Bone Mask");
//		}
//		if (endpointIdentifier > 1)
//		{
//			endpoint.Style.Color = Style.HeaderColor;
//			endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };
//
//			 
//			std::size_t hash = 0;
//			auto pose = node.GetEndpoint<GraphNode::Connection::Input>(endpointIdentifier)->As<NodeValueType::Pose>();
//			if (pose) hash = pose->GetAnimationHash();
//
//			ImGui::Text("Pose");  ImGui::SameLine();
//			ImGuiLayer::HelpMarker("#", std::format("{:x}", hash).c_str());
//		}
//		break;
//	}
//	case EndpointPrototype::Output:
//	{
//		endpoint.Style.Color = Style.HeaderColor;
//		endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };
//
//		ImGui::Text("Pose"); break;
//	}
//	}
//}
//

//
//editor_animation_graph::ValueNodeDeligate::ValueNodeDeligate(GraphNode::NodeIdentifier id, SharedPointer<GraphNode> node) :
//	BaseNodeDeligate(id, node)
//{
//	Style.Title = "Float";
//	Style.HeaderColor = ImVec4{ 0.3, 0.5, 1.0, 1.0 };
//	Style.Size = ImVec2{ 100, 100 };
//}
//
//void editor_animation_graph::ValueNodeDeligate::ProcessBodyContent()
//{
//	auto& node = GetNode()->As<animation::BlendNode2D>();
//	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
//	ImGui::DragFloat("##value", &node.GetEndpoint<GraphNode::Connection::Output>(0)->As<NodeValueType::Float>(), 0.001, 0.0, 1.0);
//	ImGui::PopItemWidth();
//}
//
//void editor_animation_graph::ValueNodeDeligate::ProcessEndpoint(const GraphNode::EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint)
//{
//	auto& node = GetNode()->As<animation::ValueNode>();
//	const EndpointPrototype::EndpointType type = endpoint.GetType();
//
//	switch (type)
//	{
//	case EndpointPrototype::Output:
//	{
//		endpoint.Style.Color = Style.HeaderColor;
//		endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };
//	}
//	}
//}
//void editor_animation_graph::GraphDeligate::ProcessSideBar()
//{
//	AnimationGraph& graph = GetGraph().Get();
//
//	//ImGui::ColorEdit4("Connection", (float*)&this->m_VisualStyle.ConnectionColor);
//
//	ImGui::Text("Animation graph");
//	ImGui::Separator();
//
//	if (ImGui::BeginChildEx("Node: Pose", std::size_t(&graph), ImGui::GetContentRegionAvail(), true, 0))
//	{
//		const ImVec2 windowPosition = ImGui::GetWindowPos();
//		const ImVec2 windowSize = ImGui::GetWindowSize();
//
//		if (ImGui::BeginTable("Table", 2, ImGuiTableFlags_SizingStretchProp))
//		{
//			ImGui::TableNextRow();
//			{
//				ImGui::TableNextColumn();
//				{
//					shade::ImGuiLayer::DrawFontIcon(u8"\xf29a", 1, 0.7f); 
//				}
//				ImGui::TableNextColumn();
//				{
//					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
//					
//					if (ImGui::BeginTable("##SelectSkeletonTable", 2, ImGuiTableFlags_SizingStretchProp))
//					{
//						ImGui::TableNextRow();
//						ImGui::TableNextColumn();
//						{
//							std::string buttonTitle = (!graph.GetSkeleton()) ? "Not set" : graph.GetSkeleton()->GetAssetData()->GetId();
//							ImGui::BeginDisabled();
//							ImGui::Button(buttonTitle.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0.f });
//							ImGui::EndDisabled();
//						}
//						ImGui::TableNextColumn();
//						{
//							if (ImGui::ArrowButton("##OpenPopup", ImGuiDir_Down))
//							{
//								m_IsSkeletonPopupActive = (!m_IsSkeletonPopupActive) ? true : false;
//							}
//						}
//						ImGui::EndTable();
//					}
//
//					if (m_IsSkeletonPopupActive)
//					{
//						ImGuiLayer::BeginWindowOverlay("##SkeletonSearchOverlay", ImGui::GetWindowViewport(), std::size_t(&graph), ImVec2{ windowSize.x - 10.f, 0.f }, ImVec2{ windowPosition.x + 5.f, ImGui::GetCursorScreenPos().y + 5.f }, 0.3f,
//							[&]() mutable
//							{
//								ImGuiLayer::InputTextCol("Search", m_Search);
//
//								if (ImGui::BeginListBox("##SelectAnimation", ImVec2{ ImGui::GetContentRegionAvail().x, 0.f }))
//								{
//									for (const auto& assetData : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
//									{
//										if (assetData.second->GetType() == shade::AssetMeta::Type::Skeleton && assetData.first.find(m_Search) != std::string::npos)
//										{
//											if (ImGui::Selectable(assetData.first.c_str(), false))
//											{
//												shade::AssetManager::GetAsset<shade::Skeleton,
//													shade::BaseAsset::InstantiationBehaviour::Synchronous>(assetData.first,
//														shade::AssetMeta::Category::Secondary,
//														shade::BaseAsset::LifeTime::KeepAlive,
//														[&](auto& skeleton) mutable
//														{
//															graph.SetSkeleton(skeleton);
//														});
//
//												m_IsSkeletonPopupActive = false;
//											}
//										}
//									}
//									ImGui::EndListBox();
//								}
//
//							});
//					}
//				}
//			}
//
//			ImGui::EndTable();
//		}
//
//		ImGui::EndChild();
//	}
//}

void graph_editor::StateNodeDelegate::Draw(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	static GraphNodePrototype* pLastSelectedNode = nullptr;
	//ImGui::IsKeyPressed(ImGuiKey_LeftCtrl)

	DrawTransition(context, graphStyle, nodes); // Draw Transitions!!

	ImGui::PushID(GetNode()->GetNodeIdentifier());

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x , GetScreenPosition().y } *context->Scale.Factor);

	DrawBody(context, graphStyle, nodes);

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor);

	if (ImGui::InvisibleButton(std::format("##NodeBox_{}", std::size_t(this)).c_str(), Style.Size * context->Scale.Factor))
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


	if (m_TransitionEstablish.To && m_TransitionEstablish.From)
	{
		auto tr = m_TransitionEstablish.From->As<animation::state_machine::StateNode>().AddTransition((animation::state_machine::StateNode*)m_TransitionEstablish.To);
		
		GetEditor()->InitializeRecursively(tr, GetEditor()->GetNodes());

		m_TransitionEstablish.Reset();
	}

	if (IsSelected()) { pLastSelectedNode = this; }
	
	if (ImGui::IsItemActive()) { this->SetSelected(true); } //else {this->SetSelected(false); }

	//if (!(m_ConnectionEstablish.IsOutPutSelect))
	MoveNode(context->Scale.Factor);

	ImGui::PopID();
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

	ImGui::Dummy({ 0, 0 });
}

bool IsPointOnLine(const glm::vec2& A, const glm::vec2& B, const glm::vec2& P, float thickness) {
	glm::vec2 lineDirection = glm::normalize(B - A);
	glm::vec2 pointDirection = glm::normalize(P - A);

	float dotProduct = glm::dot(pointDirection, lineDirection);
	float distance = glm::length(P - A);

	float projectionLength = dotProduct * distance;

	// Check if the projection is within the bounds of the line segment
	if (projectionLength >= 0.0f && projectionLength <= glm::length(B - A)) {
		// Calculate the distance between the point and the line
		float distanceToLine = glm::length(P - (A + lineDirection * projectionLength));

		// Check if the distance is within the thickness
		if (distanceToLine <= thickness) {
			return true; // Point is within the line with thickness
		}
	}

	return false; // Point is not within the line with thickness
}
void graph_editor::StateNodeDelegate::DrawTransition(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	auto state = reinterpret_cast<animation::state_machine::StateNode*>(GetNode());

	for (auto transition : reinterpret_cast<animation::state_machine::StateNode*>(GetNode())->GetTransitions())
	{
		auto* source = this;
		auto* destination = nodes.at(std::size_t(transition->GetTransitionData().DestinationState));

		const ImRect sourceRect = source->GetScaledRectangle(context, graphStyle);
		const ImRect destinationRect = destination->GetScaledRectangle(context, graphStyle);

		ImVec2 sPosition = sourceRect.GetCenter();
		ImVec2 ePosition = GetClosestPointOnRectBorder(destinationRect, sPosition);

		ImVec2 arrowDir = ImVec2(ePosition - sPosition);

		const float factor = 0.95;
		ImVec2 sEPosition = { sPosition.x + factor * arrowDir.x, sPosition.y + factor * arrowDir.y };

		float arrowLength = sqrt(ImLengthSqr(arrowDir));
		if (arrowLength > 0.0) arrowDir /= arrowLength;

		const ImVec2 orthogonal = ImVec2{ arrowDir.y, -arrowDir.x };

		const float thickness = 3.f * context->Scale.Factor;

		const ImVec2 arrowHeadSize = ImVec2{ thickness, thickness + (1.f * context->Scale.Factor) };

		ImVec2 arrowEndPoint1 = sEPosition - arrowDir * arrowHeadSize.x + orthogonal * arrowHeadSize.y;
		ImVec2 arrowEndPoint2 = sEPosition - arrowDir * arrowHeadSize.x - orthogonal * arrowHeadSize.y;

		bool selected = IsPointOnLine(glm::vec2(sPosition.x, sPosition.y), glm::vec2(ePosition.x, ePosition.y), glm::vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y), thickness);

		ImVec4 color = (selected) ? source->Style.BorderHovered : source->Style.BorderColor;

		context->DrawList->AddLine(sPosition, sEPosition, ImGui::ColorConvertFloat4ToU32(color), thickness);
		context->DrawList->AddTriangleFilled(ePosition, arrowEndPoint1, arrowEndPoint2, ImGui::ColorConvertFloat4ToU32(color));

		if (selected && ImGui::IsMouseDoubleClicked(0))
		{
			nodes.at(std::size_t(transition))->Style.Title = "Transition : " + source->Style.Title + " ---> " + destination->Style.Title;

			context->CurrentGraph = nodes.at(std::size_t(transition));
		}
	}
}

graph_editor::OutputPoseNodeDelegate::OutputPoseNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = ImVec4{ 0.7, 0.7, 0.7, 1.0 };
	Style.Size = ImVec2{ 150.f, 100.f };
	Style.Title = "Out Pose";
}

graph_editor::BlendNode2DNodeDelegate::BlendNode2DNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.Title = "Blend";
}

void graph_editor::BlendNode2DNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	switch (endpoint.GetType())
	{
	case NodeValueType::Float:
	{
		ImGui::Text("Blend weight");
		Style.InputEndpointsColor = ImVec4{ 0.3, 0.5, 1.0, 1.0 };
		/*ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::DragFloat("##value", &endpoint.As<NodeValueType::Float>(), 0.001, 0.0, 1.0);
		ImGui::PopItemWidth();*/
		break;
	}
	case NodeValueType::Pose:
	{
		Style.InputEndpointsColor = ImVec4{ 0.5f, 0.69f ,0.f, 1.f };
		ImGui::Text("Pose");
		break;
	}
	case NodeValueType::BoneMask:
	{
		Style.InputEndpointsColor = ImVec4{ 0.3, 0.5, 1.0, 1.0 };
		ImGui::Text("BoneMask");
		break;
	}
	}
}

graph_editor::OutputTransitionNodeDelegate::OutputTransitionNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = ImVec4{ 0.7, 0.2, 0.2, 1.0 };
	Style.Size = ImVec2{ 250.f, 250.f };
	Style.Title = "Transition";
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
			Style.InputEndpointsColor = { 0.8, 0.2, 0.2, 1.0 };
			break;
		}
		if (identifier == 2)
		{
			Style.InputEndpointsColor = { 0.8, 0.2, 0.2, 1.0 };
			ImGui::Checkbox("Sync", &GetNode()->As<shade::animation::state_machine::OutputTransitionNode>().IsSync());
			break;
		}

	}
	case NodeValueType::Float:
	{
		Style.InputEndpointsColor = ImVec4{ 0.3, 0.5, 1.0, 1.0 };

		ImGui::Text("Duration"); ImGui::SameLine();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::DragFloat("##value", &endpoint.As<NodeValueType::Float>(), 0.001, 0.0, 1.0);
		ImGui::PopItemWidth();
		break;
	}
	}
}

graph_editor::PoseNodeDelegate::PoseNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.Title = "Pose";
}

void graph_editor::PoseNodeDelegate::ProcessBodyContent(const InternalContext* context)
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

void graph_editor::PoseNodeDelegate::ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes)
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

void graph_editor::StateMachineNodeDeligate::ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes)
{
	for (auto state : GetNode()->As<state_machine::StateMachineNode>().GetNodes())
	{
		if (ImGui::Button(std::to_string((int)state).c_str()))
		{
			GetNode()->SetRootNode(state);
		}
	}
}

void graph_editor::StateMachineNodeDeligate::ProcessPopup(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, const std::string& search)
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
}

// TODO: Add GraphStyle !!
void graph_editor::AnimationGraphDeligate::ProcessSideBar(const InternalContext* pContext, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes)
{
	ImGui::Text("Inputs:"); ImGui::Separator();

	InternalContext   context;
	GraphVisualStyle  style;
	context.DrawList = ImGui::GetWindowDrawList();
	style.Rounding = 0.f;
	ImVec2 offset{ 5, 30 };
	//ImVec2 offset = ImGui::GetCursorScreenPos();

	auto inputs = GetNode()->As<AnimationGraph>().GetInputNodes();
	for (auto [name, input] : GetNode()->As<AnimationGraph>().GetInputNodes())
	{
		ImGui::PushID(GetNode()->GetNodeIdentifier());

		auto node = referNodes.at(std::size_t(input));
		auto nodeStyle = node->Style;

		node->Style.Title = name;

		node->Style.Size = ImVec2{ ImGui::GetContentRegionAvail().x, 75.f };

		context.Offset = ImGui::GetWindowPos() - referNodes.at(std::size_t(input))->GetScreenPosition() + offset;

		node->DrawHeader(&context, &style, referNodes);

		ImGui::BeginGroup();
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, style.Padding * context.Scale.Factor);
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, style.CellPadding * context.Scale.Factor);
			ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4{ 0.f, 0.f, 0.f, 0.f });

			if (ImGui::BeginTableEx("##BodyContentTable", std::size_t(this) + 2000, 1, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuterV))
			{
				ImGui::TableNextRow();
				{
					ImGui::TableNextColumn();
					{
						node->ProcessBodyContent(&context);
					}
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleVar(2);
			ImGui::PopStyleColor();
		}
		ImGui::EndGroup();

		node->DrawBorder(&context, &style, referNodes);


		offset.y += node->Style.Size.y + 15.f;

		ImGui::PopID();

		node->Style = nodeStyle;
	}

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
			CreateReferNode<graphs::IntNode>("Int :")->Style.HeaderColor = { 0, 0.5, 0.5, 1 };
		}
		if (ImGui::Selectable("+ Float"))
		{
			CreateReferNode<graphs::FloatNode>("Float :")->Style.HeaderColor = { 0, 0.5, 0.5, 1 };
		}
		ImGui::BeginDisabled();
		ImGui::Selectable("+ Bool");
		ImGui::Selectable("+ String");
		ImGui::EndDisabled();

		ImGui::Dummy(ImVec2{ 0, 10.f });

		if (ImGui::Button("Close"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

bool graph_editor::AnimationGraphDeligate::DeleteReferNode(const std::string& name)
{
	return false;
}

graph_editor::IntEqualsNodeDelegate::IntEqualsNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = ImVec4{ 0.2, 0.4, 0.99, 1.0 };
	Style.Size = ImVec2{ 200.f, 150.f };
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

			Style.InputEndpointsColor = Style.HeaderColor = { 0.2, 0.4, 0.99, 1.0 };
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
			Style.OutPutEndpointsColor = { 0.8, 0.2, 0.2, 1.0 };
			break;
		}
		}
	}
}

graph_editor::IntNodeDelegate::IntNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = ImVec4{ 0.2, 0.4, 0.99, 1.0 };

	Style.OutPutEndpointsColor = ImVec4{ 0.2, 0.4, 0.99, 1.0 };
	Style.InputEndpointsColor = ImVec4{ 0.2, 0.4, 0.99, 1.0 };

	Style.Size = ImVec2{ 125.f, 100.f };
	Style.Title = "Int";

}

void graph_editor::IntNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{

}

void graph_editor::IntNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	auto node = GetNode();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::DragInt("", &node->GetEndpoint<shade::graphs::Connection::Output>(0)->As<NodeValueType::Int>(), 0.1, -INT32_MAX, INT32_MAX);
	ImGui::PopItemWidth();
}

graph_editor::FloatEqualsNodeDelegate::FloatEqualsNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{

	Style.HeaderColor = ImVec4{ 0.2, 0.4, 0.99, 1.0 };

	Style.Size = ImVec2{ 200.f, 150.f };
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

			Style.InputEndpointsColor = Style.HeaderColor = { 0.2, 0.4, 0.99, 1.0 };
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
			Style.OutPutEndpointsColor = { 0.8, 0.2, 0.2, 1.0 };
			break;
		}
		}
	}
}

graph_editor::FloatNodeDelegate::FloatNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.HeaderColor = ImVec4{ 0.2, 0.4, 0.99, 1.0 };

	Style.OutPutEndpointsColor = ImVec4{ 0.2, 0.4, 0.99, 1.0 };
	Style.InputEndpointsColor = ImVec4{ 0.2, 0.4, 0.99, 1.0 };

	Style.Size = ImVec2{ 125.f, 100.f };
	Style.Title = "Float";
}

void graph_editor::FloatNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{

}

void graph_editor::FloatNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	auto node = GetNode();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::DragFloat("", &node->GetEndpoint<shade::graphs::Connection::Output>(0)->As<NodeValueType::Float>(), 0.1, -FLT_MAX, FLT_MAX);
	ImGui::PopItemWidth();
}

graph_editor::BoneMaskNodeDelegate::BoneMaskNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
{
	Style.Title = "Bone Mask";
	Style.HeaderColor = ImVec4{ 0.6, 0.4, 0.0, 1.0 };
	Style.Size = ImVec2{ 180, 120 };
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

void graph_editor::BoneMaskNodeDelegate::ProcessBodyContent(const InternalContext* context)
{
	auto& node = GetNode()->As<BoneMaskNode>();

	shade::ImGuiLayer::DrawFontIcon(u8"\xf29a", 1, 0.6f); ImGui::SameLine(); ImGui::Text(node.GetGraphContext()->As<animation::AnimationGraphContext>().Skeleton->GetAssetData()->GetId().c_str());
}

void graph_editor::BoneMaskNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	ImGui::Text("Result");
	Style.OutPutEndpointsColor = { 0.8, 0.2, 0.2, 1.0 };
}