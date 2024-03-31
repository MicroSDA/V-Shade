#include "shade_pch.h"
#include "EditorAnimGraph.h"

void graph_editor::GraphPrototype::Intialize()
{
	/*for (auto* node : GetGraph()->GetNodes())
	{
		if(dynamic_cast<node>)
	}*/
}

ImRect graph_editor::GraphNodePrototype::GetScaledRectangle(const InternalContext* context, const GraphVisualStyle* graphStyle)
{
	const ImVec2 scaledPosition = ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor;
	const ImVec2 pMin = (context->Offset + scaledPosition);
	//const ImVec2 pMax = (context->Offset + scaledPosition + (ImVec2{ Style.Size.x, graphStyle->HeaderHeight } *context->Scale.Factor));
	const ImVec2 pMax = (context->Offset + scaledPosition + (Style.Size * context->Scale.Factor));

	return ImRect{ pMin, pMax };
}

void graph_editor::GraphNodePrototype::Draw(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	ImGui::PushID(GetNode()->GetNodeIdentifier());

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x , GetScreenPosition().y } *context->Scale.Factor);

	DrawBody(context, graphStyle, nodes);

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor);

	ImGui::InvisibleButton(std::format("##NodeBox_{}", std::size_t(this)).c_str(), Style.Size * context->Scale.Factor);

	if (ImGui::IsItemActive()) { this->SetSelected(true); } //else {this->SetSelected(false); }

	//if (!(m_ConnectionEstablish.IsOutPutSelect))
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
		ImGui::Text(Style.Title.c_str());
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
					ProcessBodyContent();
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

void graph_editor::GraphNodePrototype::DrawFooter(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
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

	if (ImGui::BeginTableEx("##EndpointsTable", std::size_t(this) + 1000, (outputCount) ? 2 : 1,
		ImGuiTableFlags_BordersOuterV |
		ImGuiTableFlags_SizingStretchProp,
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

				// table cutting circle !!
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
					Style.InputEndpointsColor,
					Style.InputEndpointsColorHovered))
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
}

void graph_editor::GraphNodePrototype::DrawConnections(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
{
	auto& connections = nodes.begin()->second->GetNode()->GetGraphContext()->Connections;

	if (connections.find(GetNode()) != connections.end())
	{
		auto& nodeConnections = connections.at(GetNode());

		for (const auto& connection : nodeConnections)
		{
			ImGuiGraphNodeRender::DrawConnection(
				context->DrawList,
				context->Offset,
				context->Scale.Factor,
				ImVec2{ connection.InputScreenPosition.x, connection.InputScreenPosition.y },
				ImVec2{ connection.OutputScreenPosition.x, connection.OutputScreenPosition.y },

				ImVec4{ 1, 1, 1, 1 },
				graphStyle->ConnectionThickness);
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


void graph_editor::GraphEditor::InitializeRecursively(graphs::BaseNode* pNode)
{
	// TODO:: state machine
	if (dynamic_cast<state_machine::StateNode*>(pNode))
	{
		state_machine::StateNode* state = reinterpret_cast<state_machine::StateNode*>(pNode);
		m_Nodes[std::size_t(pNode)] = new StateNodeDelegate(pNode);
		// Transitions nodes aka logic
		for (auto transition : state->GetTransitions())
		{
			InitializeRecursively(transition);
		}
		// Play animation nodes aka
		for (auto node : state->GetNodes())
		{
			InitializeRecursively(node);
		}
	}
	else if (dynamic_cast<state_machine::TransitionNode*>(pNode))
	{
		// Transition as graph node, contains nodes
		state_machine::TransitionNode* transition = reinterpret_cast<state_machine::TransitionNode*>(pNode);
		m_Nodes[std::size_t(pNode)] = new TransitionNodeDelegate(pNode);

		for (auto node : transition->GetNodes())
		{
			InitializeRecursively(node);
		}
	}
	else if (dynamic_cast<state_machine::StateMachineNode*>(pNode))
	{
		// Transition as graph node, contains nodes
		state_machine::StateMachineNode* machine = reinterpret_cast<state_machine::StateMachineNode*>(pNode);
		m_Nodes[std::size_t(pNode)] = new StateMachineNodeDeligate(pNode);

		// Aka states
		for (auto state : machine->GetNodes())
		{
			InitializeRecursively(state);
		}
	}
	else if (dynamic_cast<graphs::BaseNode*>(pNode))
	{
		if (dynamic_cast<shade::graphs::ValueNode*>(pNode))
		{
			shade::graphs::ValueNode* value = reinterpret_cast<shade::graphs::ValueNode*>(pNode);
			m_Nodes[std::size_t(pNode)] = new ValueNodeDelegate(pNode);
		}
		if (dynamic_cast<shade::animation::OutputPoseNode*>(pNode))
		{
			shade::animation::OutputPoseNode* value = reinterpret_cast<shade::animation::OutputPoseNode*>(pNode);
			m_Nodes[std::size_t(pNode)] = new OutputPoseNodeDelegate(pNode);
		}
		if (dynamic_cast<shade::animation::BlendNode2D*>(pNode))
		{
			shade::animation::BlendNode2D* value = reinterpret_cast<shade::animation::BlendNode2D*>(pNode);
			m_Nodes[std::size_t(pNode)] = new BlendNode2DNodeDelegate(pNode);
		}
		if (dynamic_cast<shade::animation::state_machine::OutputTransitionNode*>(pNode))
		{
			shade::animation::state_machine::OutputTransitionNode* value = reinterpret_cast<shade::animation::state_machine::OutputTransitionNode*>(pNode);
			m_Nodes[std::size_t(pNode)] = new OutputTransitionNodeDelegate(pNode);
		}
		if (dynamic_cast<shade::animation::PoseNode*>(pNode))
		{
			shade::animation::PoseNode* value = reinterpret_cast<shade::animation::PoseNode*>(pNode);
			m_Nodes[std::size_t(pNode)] = new PoseNodeDelegate(pNode);
		}
		if (dynamic_cast<shade::graphs::IntEqualsNode*>(pNode))
		{
			shade::graphs::IntEqualsNode* value = reinterpret_cast<shade::graphs::IntEqualsNode*>(pNode);
			m_Nodes[std::size_t(pNode)] = new IntEqualsNodeDelegate(pNode);
		}
		if (dynamic_cast<shade::graphs::IntNode*>(pNode))
		{
			shade::graphs::IntNode* value = reinterpret_cast<shade::graphs::IntNode*>(pNode);
			m_Nodes[std::size_t(pNode)] = new IntEqualsNodeDelegate(pNode);
		}
	}
}

void graph_editor::GraphEditor::Initialize(AnimationGraph* pGraph)
{
	// Add to nodes !!
	m_Nodes[std::size_t(pGraph)] = new AnimationGraphDeligate(pGraph);

	m_Context.CurrentGraph = m_Nodes.at(std::size_t(pGraph));
	m_pRootGraph = pGraph;

	for (auto node : m_pRootGraph->GetNodes())
	{
		InitializeRecursively(node);
	}

	for (auto input : pGraph->GetInputNodes())
	{
		InitializeRecursively(input);
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
					m_pSelectedNode->ProcessSideBar();
				}
				else
				{
					m_Context.CurrentGraph->ProcessSideBar();

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
						m_pSelectedNode = nullptr;
					}

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

				if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
					ImGui::OpenPopup("##GraphEditorPopup");
			}


			if (ImGui::BeginPopup("##GraphEditorPopup"))
			{

				this->PopupMenu();

				ImGui::EndPopup();
			}

		}ImGui::End();
	}
	ImGui::PopStyleColor();

	ImGuiLayer::BeginWindowOverlay("Path",
		ImGui::GetWindowViewport(), 123124,
		ImVec2{ m_Context.CanvasSize.x, ImGui::GetContentRegionAvail().y + 5.f },
		m_Context.CanvasRect.Min, 0.0f, [&]() {DrawPathRecursevly(m_Context.CurrentGraph); });

	if (m_Context.ConnectionEstablish.IsInputSelect && m_Context.ConnectionEstablish.IsOutPutSelect)
	{
		m_pRootGraph->ConnectNodes(
			m_Context.ConnectionEstablish.InputNode->GetNodeIdentifier(),
			m_Context.ConnectionEstablish.InputEndpointIdentifier,
			m_Context.ConnectionEstablish.OutPutNode->GetNodeIdentifier(),
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
		//m_Context.DrawList->ChannelsSetCurrent(CurrentChannel);

		GraphNodePrototype* prototype = m_Nodes.at(std::size_t(node));

		prototype->Draw(&m_Context, &m_VisualStyle, m_Nodes);

		if (ImGui::IsItemActive()) { m_pSelectedNode = prototype; }

		if (m_pSelectedNode && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			m_Path.push_back(m_Context.CurrentGraph); // Change container type
			m_Context.CurrentGraph = m_Nodes.at(std::size_t(node));
		}

		CurrentChannel--;
	}

	for (auto node : m_Context.CurrentGraph->GetNode()->GetReferNodes())
	{
		//m_Context.DrawList->ChannelsSetCurrent(CurrentChannel);

		GraphNodePrototype* prototype = m_Nodes.at(std::size_t(node));

		prototype->Draw(&m_Context, &m_VisualStyle, m_Nodes);

		if (ImGui::IsItemActive()) { m_pSelectedNode = prototype; }

		if (m_pSelectedNode && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			m_Path.push_back(m_Context.CurrentGraph); // Change container type
			m_Context.CurrentGraph = m_Nodes.at(std::size_t(node));
		}

		CurrentChannel--;
	}

	//m_Context.DrawList->ChannelsMerge();

}

void graph_editor::GraphEditor::DrawConnections()
{
	if (m_Context.CurrentGraph)
	{
		for (auto& node : m_Context.CurrentGraph->GetNode()->GetNodes())
		{
			m_Nodes.at(std::size_t(node))->DrawConnections(&m_Context, &m_VisualStyle, m_Nodes);
		}
		for (auto& node : m_Context.CurrentGraph->GetNode()->GetReferNodes())
		{
			m_Nodes.at(std::size_t(node))->DrawConnections(&m_Context, &m_VisualStyle, m_Nodes);
		}
	}

}

void graph_editor::GraphEditor::PopupMenu()
{
	for (const auto node : m_pRootGraph->As<AnimationGraph>().GetInputNodes())
	{
		if (ImGui::Selectable("Add Node")) 
		{ 
			if (m_Context.CurrentGraph)
			{ 
				m_Context.CurrentGraph->GetNode()->AddReferNode(node);
			} 
		}
	}

	if (ImGui::Button("OK"))
		ImGui::CloseCurrentPopup();

	//ImGui::BeginMenuitem("##GraphEditorPopup")
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
//editor_animation_graph::BoneMaskNodeDeligate::BoneMaskNodeDeligate(GraphNode::NodeIdentifier id, SharedPointer<GraphNode> node)
//	:
//	BaseNodeDeligate(id, node)
//{
//	Style.Title = "Bone Mask";
//	Style.HeaderColor = ImVec4{ 0.6, 0.4, 0.0, 1.0 };
//	Style.Size = ImVec2{ 150, 100 };
//}
//
//void editor_animation_graph::BoneMaskNodeDeligate::ProcessSideBar()
//{
//	BoneMaskNode& node = GetNode()->As<animation::BoneMaskNode>();
//	ImGui::Text("Node: Bone Mask");
//
//	if (ImGui::BeginChildEx("Node: Bone Mask", std::size_t(&node), ImGui::GetContentRegionAvail(), true, 0))
//	{
//		ImGuiLayer::InputTextCol("Search", m_Search);
//
//		if (ImGui::BeginTable("##BoneMaskTable", 2, ImGuiTableFlags_SizingFixedFit))
//		{
//			for (auto& [key, value] : node.GetBoneMask().Weights)
//			{
//				if (value.first.find(m_Search) != std::string::npos)
//				{
//					ImGui::TableNextRow();
//					{
//						ImGui::TableNextColumn();
//
//						shade::ImGuiLayer::DrawFontIcon(u8"\xf2d8", 1, 0.6f); ImGui::SameLine(); //ImGui::Text(value.first.c_str());
//
//						ImGui::Text(value.first.c_str());
//
//						ImGui::TableNextColumn();
//						ImGui::PushID(key);
//						ImGui::PushItemWidth(50.f);
//						ImGui::DragFloat("##", &value.second, 0.001f, 0.f, 1.f);
//						ImGui::PopItemWidth();
//						ImGui::PopID();
//					}
//				}
//			}
//
//			ImGui::EndTable();
//		}
//		ImGui::EndChild();
//	}
//
//}
//
//void editor_animation_graph::BoneMaskNodeDeligate::ProcessBodyContent()
//{
//	auto& node = GetNode()->As<BoneMaskNode>();
//
//	shade::ImGuiLayer::DrawFontIcon(u8"\xf29a", 1, 0.6f); ImGui::SameLine(); ImGui::Text(node.GetGraphContext()->As<animation::AnimationGraphContext>().Skeleton->GetAssetData()->GetId().c_str());
//}
//
//void editor_animation_graph::BoneMaskNodeDeligate::ProcessEndpoint(const GraphNode::EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint)
//{
//	auto& node = GetNode()->As<BoneMaskNode>();
//	const EndpointPrototype::EndpointType type = endpoint.GetType();
//
//	switch (endpoint.GetType()) // TODO: Rename -  GetConenctionType
//	{
//	case EndpointPrototype::Output:
//	{
//		endpoint.Style.Color = Style.HeaderColor;
//		endpoint.Style.ColorHovered = ImVec4{ endpoint.Style.Color.x * 1.5f, endpoint.Style.Color.y * 1.5f, endpoint.Style.Color.z * 1.5f, endpoint.Style.Color.w };
//
//		//ImGui::Text("Mask");
//	}
//	}
//}
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
	DrawTransition(context, graphStyle, nodes); // Draw Transitions!!

	ImGui::PushID(GetNode()->GetNodeIdentifier());

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x , GetScreenPosition().y } *context->Scale.Factor);

	DrawBody(context, graphStyle, nodes);

	ImGui::SetCursorScreenPos(context->Offset + ImVec2{ GetScreenPosition().x, GetScreenPosition().y } *context->Scale.Factor);

	ImGui::InvisibleButton(std::format("##NodeBox_{}", std::size_t(this)).c_str(), Style.Size * context->Scale.Factor);

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
					ProcessBodyContent();
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

graph_editor::OutputPoseNodeDelegate::OutputPoseNodeDelegate(graphs::BaseNode* pNode) : GraphNodePrototype(pNode)
{
	Style.HeaderColor = ImVec4{ 0.7, 0.7, 0.7, 1.0 };
	Style.Size = ImVec2{ 120.f, 100.f };
	Style.Title = "Out Pose";
}

graph_editor::BlendNode2DNodeDelegate::BlendNode2DNodeDelegate(graphs::BaseNode* pNode) : GraphNodePrototype(pNode)
{
	Style.Title = "Blend";
}

graph_editor::OutputTransitionNodeDelegate::OutputTransitionNodeDelegate(graphs::BaseNode* pNode) : GraphNodePrototype(pNode)
{
	Style.HeaderColor = ImVec4{ 0.7, 0.2, 0.2, 1.0 };
	Style.Size = ImVec2{ 200.f, 150.f };
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
			Style.InputEndpointsColor = ImVec4(1.f, 0.f, 0.f, 1.f);
			break;
		}
		if (identifier == 2)
		{
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

graph_editor::PoseNodeDelegate::PoseNodeDelegate(graphs::BaseNode* pNode) : GraphNodePrototype(pNode)
{
	Style.Title = "Pose";
}

void graph_editor::PoseNodeDelegate::ProcessBodyContent()
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

void graph_editor::PoseNodeDelegate::ProcessSideBar()
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

void graph_editor::StateMachineNodeDeligate::ProcessSideBar()
{
	for (auto state : GetNode()->As<state_machine::StateMachineNode>().GetNodes())
	{
		if (ImGui::Button(std::to_string((int)state).c_str()))
		{
			GetNode()->SetRootNode(state);
		}
	}
}

void graph_editor::AnimationGraphDeligate::ProcessSideBar()
{
	ImGui::Text("AnimGraph");
}

graph_editor::IntEqualsNodeDelegate::IntEqualsNodeDelegate(graphs::BaseNode* pNode) : GraphNodePrototype(pNode)
{
	Style.HeaderColor	= ImVec4 { 0.7, 0.2, 0.2, 1.0 };
	Style.Size			= ImVec2 { 200.f, 150.f };
	Style.Title			= "IntEquals";
}

void graph_editor::IntEqualsNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	if (type == graphs::Connection::Type::Input)
	{
		switch (endpoint.GetType())
		{
		case NodeValueType::Int:
		{
			ImGui::Text("Input");

			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragInt(std::to_string(identifier).c_str(), &endpoint.As<NodeValueType::Int>(), 0.1, 0, 1);
			ImGui::PopItemWidth();

			Style.InputEndpointsColor = ImVec4(1.f, 0.f, 0.f, 1.f);
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
			ImGui::Text("Is");
			Style.InputEndpointsColor = ImVec4(1.f, 0.f, 0.f, 1.f);
			break;
		}
		}
	}
}

graph_editor::IntNodeDelegate::IntNodeDelegate(graphs::BaseNode* pNode) : GraphNodePrototype(pNode)
{
	Style.HeaderColor = ImVec4{ 0.7, 0.2, 0.2, 1.0 };
	Style.Size = ImVec2{ 200.f, 150.f };
	Style.Title = "Int";
}

void graph_editor::IntNodeDelegate::ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint)
{
	ImGui::Text("Is");
	Style.InputEndpointsColor = ImVec4(1.f, 0.f, 0.f, 1.f);
}
