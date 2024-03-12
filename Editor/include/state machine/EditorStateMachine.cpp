#include "shade_pch.h"
#include "EditorStateMachine.h"

//void graph_editor::GraphPrototype::Intialize()
//{
//	/*for (auto* node : GetGraph()->GetNodes())
//	{
//		if(dynamic_cast<node>)
//	}*/
//}
//
//void graph_editor::GraphEditor::InitializeRecursively(graphs::BaseNode* pNode)
//{
//	// TODO:: state machine
//	if (dynamic_cast<shade::state_machine::State*>(pNode))
//	{
//		shade::state_machine::State* state = reinterpret_cast<shade::state_machine::State*>(pNode);
//		m_GraphNodes[std::size_t(pNode)] = new StateNodeDelegate(pNode);
//		// Transitions nodes
//		for (auto transition : state->GetTransitions())
//		{
//			InitializeRecursively(transition);
//		}
//		// Play animation nodes
//		for (auto node : state->GetNodes())
//		{
//			InitializeRecursively(node);
//		}
//	}
//	else if (dynamic_cast<shade::state_machine::Transition*>(pNode))
//	{
//		// Transition as graph node, contains nodes
//		shade::state_machine::Transition* transition = reinterpret_cast<shade::state_machine::Transition*>(pNode);
//		m_GraphNodes[std::size_t(pNode)] = new TransitionNodeDelegate(pNode);
//
//		for (auto node : transition->GetNodes())
//		{
//			InitializeRecursively(node);
//		}
//	}
//	else if (dynamic_cast<graphs::BaseGraph*>(pNode))
//	{
//
//	}
//	else if (dynamic_cast<graphs::BaseNode*>(pNode))
//	{
//		if (dynamic_cast<shade::graphs::ValueNode*>(pNode))
//		{
//			shade::graphs::ValueNode* value = reinterpret_cast<shade::graphs::ValueNode*>(pNode);
//			m_GraphNodes[std::size_t(pNode)] = new ValueNodeDelegate(pNode);
//		}
//	}	
//}
//
//void graph_editor::GraphEditor::Initialize(graphs::Graph* pRootGraph)
//{
//	m_pRootGraph = pRootGraph;
//
//	for (auto node : m_pRootGraph->GetNodes())
//	{
//		InitializeRecursively(node);
//	}
//	m_pRootGraph->Initialize();
//}
//
//bool graph_editor::GraphEditor::Edit(const char* title, const ImVec2& size)
//{
//	// Save current style 
//	ImGuiStyle unscaledStyle = ImGui::GetStyle();
//	ImGui::PushStyleColor(ImGuiCol_WindowBg, m_VisualStyle.BackgroundColor);
//	ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
//
//	{
//		if (ImGui::Begin(title, (bool*)0))
//		{
//			if (ImGui::BeginChild("##ImGuiGraphPrototype::ContentSideBar",
//				ImVec2(300, ImGui::GetContentRegionAvail().y),
//				true,
//				ImGuiWindowFlags_NoScrollbar |
//				ImGuiWindowFlags_NoMove |
//				ImGuiWindowFlags_NoScrollWithMouse))
//			{
//				/*if (m_ActiveNode)
//				{
//					m_ActiveNode->ProcessSideBar();
//				}
//				else
//				{
//					ProcessSideBar();
//				}*/
//
//				ImGui::EndChild();
//			}
//
//			ImGui::SameLine();
//
//			const ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
//			const ImVec2 windowPos = cursorPosition, scrollRegionLocalPos(0, 0);
//			m_Context.CanvasSize = ImGui::GetContentRegionAvail();
//			m_Context.CanvasRect = ImRect{ windowPos, windowPos + m_Context.CanvasSize };
//			m_Context.Offset = cursorPosition + (m_Context.CanvasPosition * m_Context.Scale.Factor);
//
//
//			if (ImGui::BeginChild("##ImGuiGraphPrototype::ScrollRegion",
//				ImVec2(0, 0),
//				true,
//				ImGuiWindowFlags_NoScrollbar |
//				ImGuiWindowFlags_NoMove |
//				ImGuiWindowFlags_NoScrollWithMouse))
//			{
//
//				m_Context.DrawList = ImGui::GetWindowDrawList();
//				// Set all elements scale inside canvas 
//				ImGui::GetStyle().ScaleAllSizes(m_Context.Scale.Factor);
//				ImGui::SetWindowFontScale(m_Context.Scale.Factor);
//
//				ProcessScale();
//
//				ImGuiGraphNodeRender::DrawGrid(m_Context.DrawList,
//					windowPos,
//					m_Context.CanvasSize,
//					m_Context.CanvasPosition,
//					m_Context.Scale.Factor,
//					ImGui::ColorConvertFloat4ToU32(m_VisualStyle.GridColorSmall),
//					ImGui::ColorConvertFloat4ToU32(m_VisualStyle.GridColorBig),
//					m_VisualStyle.GridSize);
//				{
//					if (ImGui::IsItemActive())
//					{
//						/*m_ActiveNode = nullptr;
//						m_ConnectionEstablish.Reset();*/
//					}
//
//					DrawNodes();
//					//DrawAllConnections();
//				}
//
//				ImGui::EndChild();
//
//			}
//
//			if (m_Context.CanvasRect.Contains(ImGui::GetIO().MousePos))
//			{
//				// Scrolling
//				if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
//					m_Context.CanvasPosition += ImGui::GetIO().MouseDelta / m_Context.Scale.Factor;
//
//				if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
//					ImGui::OpenPopup("##GraphEditorPopup");
//			}
//
//
//			if (ImGui::BeginPopup("##GraphEditorPopup"))
//			{
//
//				//this->PopupMenu();
//
//				ImGui::EndPopup();
//			}
//
//		}ImGui::End();
//	}
//	ImGui::PopStyleColor();
//
//	// Reset style back
//	ImGui::GetStyle() = unscaledStyle;
//
//	return false;
//	
//}
//
//
//
//void graph_editor::GraphEditor::ProcessScale()
//{
//	ImGuiIO& io = ImGui::GetIO(); // todo add as arg
//
//	if (m_Context.CanvasRect.Contains(io.MousePos))
//	{
//		if (io.MouseWheel <= -std::numeric_limits<float>::epsilon())
//			m_Context.Scale.TargetFactor *= 1.0f - m_Context.Scale.RatioFactor;
//
//		if (io.MouseWheel >= std::numeric_limits<float>::epsilon())
//			m_Context.Scale.TargetFactor *= 1.0f + m_Context.Scale.RatioFactor;
//	}
//
//	const ImVec2 deltaMouseWorldPossition = CalculateMouseWorldPos(io.MousePos);
//
//	m_Context.Scale.TargetFactor = ImClamp(m_Context.Scale.TargetFactor, m_Context.Scale.MinFactor, m_Context.Scale.MaxFactor);
//	m_Context.Scale.Factor = ImClamp(m_Context.Scale.Factor, m_Context.Scale.MinFactor, m_Context.Scale.MaxFactor);
//	m_Context.Scale.Factor = ImLerp(m_Context.Scale.Factor, m_Context.Scale.TargetFactor, m_Context.Scale.LerpFactor);
//
//	const ImVec2 mouseWorldPossition = CalculateMouseWorldPos(io.MousePos);
//
//	if (ImGui::IsMousePosValid())
//		m_Context.CanvasPosition += mouseWorldPossition - deltaMouseWorldPossition;
//}
//
//ImVec2 graph_editor::GraphEditor::CalculateMouseWorldPos(const ImVec2& mousePosition)
//{
//	return (mousePosition - ImGui::GetCursorScreenPos()) / m_Context.Scale.Factor;
//}
//
//void graph_editor::GraphEditor::DrawNodes()
//{
//	std::size_t CurrentChannel = m_pRootGraph->GetNodes().size() - 1;
//
//	m_Context.DrawList->ChannelsSplit((m_pRootGraph->GetNodes().size()) ? m_pRootGraph->GetNodes().size() + 1 : 1);
//
//	for (auto node : m_pRootGraph->GetNodes())
//	{
//		m_Context.DrawList->ChannelsSetCurrent(CurrentChannel);
//
//		GraphNodePrototype* prototype = m_GraphNodes.at(std::size_t(node));
//
//		ImGui::PushID(prototype->GetNode()->GetNodeIdentifier());
//
//		ImGui::SetCursorScreenPos(m_Context.Offset + ImVec2 { node->GetScreenPosition().x , node->GetScreenPosition().y } * m_Context.Scale.Factor);
//
//		DrawNode((m_pRootGraph == node), prototype);
//
//		//ImGui::SetCursorScreenPos(m_Context.Offset + node->GetScreenPosition() * m_Context.Scale.Factor);
//
//		//ImGui::InvisibleButton(std::format("##NodeBox_{}", std::size_t(node)).c_str(), node->Style.Size * m_Context.Scale.Factor);
//
//		//if (ImGui::IsItemActive()) { m_ActiveNode = node; }
//
//		/*if (!(m_ConnectionEstablish.IsOutPutSelect))
//			MoveNode(m_Context.Scale.Factor, node->GetScreenPosition());*/
//
//		ImGui::PopID();
//		CurrentChannel--;
//
//	}
//
//	m_Context.DrawList->ChannelsMerge();
//}
//
//void graph_editor::GraphEditor::DrawNode(bool isSelected, GraphNodePrototype* node)
//{
//	const ImVec2 nodeRectMin = m_Context.Offset + node->GetScreenPosition() * m_Context.Scale.Factor;
//	const ImVec2 nodeRectMax = nodeRectMin + node->Style.Size * m_Context.Scale.Factor;
//
//	const ImVec4 bckColor = (isSelected) ? ImVec4(
//		node->Style.BackgroundColor.x * 1.5f,
//		node->Style.BackgroundColor.y * 1.5f,
//		node->Style.BackgroundColor.z * 1.5f,
//		node->Style.BackgroundColor.w) : node->Style.BackgroundColor;
//
//	m_Context.DrawList->AddRectFilled(nodeRectMin, nodeRectMax,
//		ImGui::ColorConvertFloat4ToU32(bckColor), m_VisualStyle.Rounding);
//
//	// TEST
//	node->ProcessBodyContent();
//
//	/*DrawHeader(node->Style.Title.c_str(), node->GetScreenPosition(), node->Style.Size, node->Style.HeaderColor, node->Style.HeaderTextColor);
//	DrawBorder(isActive, node->GetScreenPosition(), node->Style.Size, node->Style.BorderColor);*/
//
//	//const ImVec2 endOfPrevRegion = DrawEndpoints(m_VisualStyle.HeaderHeight, nodeIndex, node);
//
//	//const ImVec2 newScreenPosition = m_Context.Offset + ImVec2{ node->GetScreenPosition().x * m_Context.Scale.Factor, endOfPrevRegion.y };
//	//const ImVec2 avalibleRegion{ nodeRectMax - newScreenPosition };
//
//	//ImGui::SetCursorScreenPos(newScreenPosition);
//
//	//ImGui::BeginGroup();
//	//{
//	//	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, m_VisualStyle.Padding * m_Context.Scale.Factor);
//	//	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, m_VisualStyle.CellPadding * m_Context.Scale.Factor);
//	//	ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4{ 0.f, 0.f, 0.f, 0.f });
//
//	//	if (ImGui::BeginTableEx("##BodyContentTable", std::size_t(node) + 2000, 1, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuterV, { avalibleRegion.x, 0 }))
//	//	{
//	//		ImGui::TableNextRow();
//	//		{
//	//			ImGui::TableNextColumn();
//	//			{
//	//				node->ProcessBodyContent();
//	//			}
//	//		}
//	//		ImGui::EndTable();
//	//	}
//	//	ImGui::PopStyleVar(2);
//	//	ImGui::PopStyleColor();
//	//}
//	//ImGui::EndGroup();
//
//	//ImGui::Dummy({ 0, 0 });
//
//	//node->Style.Size.y += ImGui::GetCursorScreenPos().y - nodeRectMax.y;
//}
//
