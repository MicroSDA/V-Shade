﻿#include "shade_pch.h"
#include "ImGuiLayer.h"

#include <shade/core/application/Application.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>
#include <shade/platforms/render/vulkan/VulkanSwapChain.h>
#include <ImGui/misc/cpp/imgui_stdlib.cpp>

#define IMGUI_DEFINE_MATH_OPERATORS

#include <ImGui/imgui_internal.h>

shade::ImGuiLayer::ImGuiLayer() : Layer()
{
	m_ImGuiRender = ImGuiRender::Create();

	//SetupImGuiStyle(true, false);

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.DisplaySize = ImVec2((float)shade::Application::GetWindow()->GetWidth(), (float)shade::Application::GetWindow()->GetHeight());
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	//ImGui::StyleColorsDark();

	m_WindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	m_WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	m_WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	m_DockSpaceFlags = ImGuiDockNodeFlags_None;

	m_Viewport = ImGui::GetMainViewport();
}

shade::ImGuiLayer::~ImGuiLayer()
{
	//ImGui::DestroyContext(GetImGuiContext());
}

void shade::ImGuiLayer::OnRenderBegin()
{
	m_ImGuiRender->BeginRender();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
	ImGui::SetNextWindowPos(m_Viewport->WorkPos);
	ImGui::SetNextWindowSize(m_Viewport->WorkSize);
	ImGui::SetNextWindowViewport(m_Viewport->ID);

}

void shade::ImGuiLayer::OnRenderEnd()
{
	ImGui::Render();

	ImGuiIO& io = ImGui::GetIO();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}

	m_ImGuiRender->EndRender();

}

ImGuiContext* shade::ImGuiLayer::GetImGuiContext()
{
	return m_ImGuiRender->GetImGuiContext();
}

void shade::ImGuiLayer::DrawImage(SharedPointer<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor)
{
	m_ImGuiRender->DrawImage(texture, size, borderColor);
}

void shade::ImGuiLayer::DrawImage(SharedPointer<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor, std::uint32_t mip)
{
	m_ImGuiRender->DrawImage(texture, size, borderColor, mip);
}

bool shade::ImGuiLayer::InputTextCol(const char* title, std::string& str, float cw1, float cw2)
{
	bool isInput = false;

	std::string _title = std::string("##") + title;

	if (ImGui::BeginTable(_title.c_str(), 2, ImGuiTableFlags_SizingStretchProp, { 0, 0 }))
	{
		ImGui::TableNextColumn();
		{
			ImGui::Text(title); ImGui::Dummy({ cw1, 0 });
		}
		ImGui::TableNextColumn();
		{
			if (!cw2)
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			else
			{
				ImGui::SetNextItemWidth(cw2);
			}

			isInput = ImGui::InputText(_title.c_str(), &str, 0, InputTextCallback);
		}

		ImGui::EndTable();
	}
	return isInput;
}

bool shade::ImGuiLayer::InputTextD(const char* title, std::string& str)
{
	std::string _title = std::string("##") + title;

	return ImGui::InputText(_title.c_str(), &str, 0, InputTextCallback);
}
bool shade::ImGuiLayer::ComboCol(const char* title, std::string& selected, std::vector<std::string>& elements, ImGuiSelectableFlags selectedFlags, ImGuiComboFlags comboFlags, const float& cw1, const float& cw2)
{
	std::string _title = std::string("##") + title;
	bool hasSelected = false;
	if (ImGui::BeginTable(_title.c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp, { 0, 0 }))
	{
		ImGui::TableNextColumn();
		{
			ImGui::Text(title); ImGui::Dummy({ cw1, 0 });
		}
		ImGui::TableNextColumn();
		{
			if (!cw2)
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			else
			{
				ImGui::SetNextItemWidth(cw2);
			}

			if (ImGui::BeginCombo(_title.c_str(), selected.c_str(), comboFlags)) // The second parameter is the label previewed before opening the combo.
			{
				for (auto& element : elements)
				{
					bool isSelected = (selected == element);
					if (ImGui::Selectable(element.c_str(), isSelected, selectedFlags))
					{
						selected = element;
						hasSelected = true;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		ImGui::EndTable();
	}

	return hasSelected;
}

bool shade::ImGuiLayer::DrawCombo(const char* title, std::string& selected, std::vector<std::string>& elements, ImGuiSelectableFlags selectedFlags, ImGuiComboFlags comboFlags)
{
	bool hasBeenSelected = false;
	if (ImGui::BeginCombo(title, selected.c_str(), comboFlags)) // The second parameter is the label previewed before opening the combo.
	{
		for (auto& element : elements)
		{
			bool isSelected = (selected == element);
			if (ImGui::Selectable(element.c_str(), isSelected, selectedFlags))
			{
				selected = element;
				hasBeenSelected = true;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	return hasBeenSelected;
}

bool shade::ImGuiLayer::DragFloat(const char* title, float* data, float step, float min, float max, float cw1, float cw2)
{
	std::string _title = std::string("##") + title;
	bool hasEdited = false;

	if (ImGui::BeginTable(_title.c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp, { 0, 0 }))
	{
		ImGui::TableNextColumn();
		{
			ImGui::Text(title); ImGui::Dummy({ cw1, 0 });
		}
		ImGui::TableNextColumn();
		{
			if (!cw2)
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			else
			{
				ImGui::SetNextItemWidth(cw2);
			}

			if (ImGui::DragFloat(_title.c_str(), data, step, min, max))
			{
				hasEdited = true;
			}
		}

		ImGui::EndTable();
	}

	return hasEdited;
}

bool shade::ImGuiLayer::DragFloat3(const char* title, float* data, float resetValue, float step, float min, float max, float cw1, float cw2)
{
	std::string _title = std::string("##") + title;

	bool hasEdited = false;

	const ImGuiStyle& style = ImGui::GetStyle();
	float width = (ImGui::GetContentRegionAvail().x / 3.0) - ((style.ItemSpacing.x));

	if (ImGui::BeginTable(_title.c_str(), 4, ImGuiTableFlags_BordersInnerV, { 0, 0 }))
	{
		ImGui::TableNextColumn();
		{
			ImGui::Text(title); ImGui::Dummy({ cw1, 0 });
		}
		ImGui::TableNextColumn();
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.753f, 0.000f, 0.099f, 0.709f }); ImGui::Button("X"); ImGui::PopStyleColor();
			ImGui::SameLine(); hasEdited += ImGui::DragFloat("##X", &data[0], step, min, max);
		}
		ImGui::TableNextColumn();
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.000f, 0.698f, 0.008f, 0.709f }); ImGui::Button("Y"); ImGui::PopStyleColor();
			ImGui::SameLine(); hasEdited += ImGui::DragFloat("##Y", &data[1], step, min, max);
		}
		ImGui::TableNextColumn();
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.257f, 0.542f, 0.852f, 0.709f }); ImGui::Button("Z"); ImGui::PopStyleColor();
			ImGui::SameLine(); hasEdited += ImGui::DragFloat("##Z", &data[2], step, min, max);
		}

		ImGui::EndTable();
	}

	return hasEdited;
}

bool shade::ImGuiLayer::DragInt(const char* title, int* data, int step, int min, int max, float cw1, float cw2, bool readOnly)
{
	std::string _title = std::string("##") + title;
	bool hasEdited = false;

	if (ImGui::BeginTable(_title.c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp, { 0, 0 }))
	{
		ImGui::TableNextColumn();
		{
			ImGui::Text(title); ImGui::Dummy({ cw1, 0 });
		}
		ImGui::TableNextColumn();
		{
			if (!cw2)
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			else
			{
				ImGui::SetNextItemWidth(cw2);
			}

			if (ImGui::DragInt(_title.c_str(), data, step, min, max, "%d", (readOnly) ? ImGuiSliderFlags_ReadOnly : 0))
			{
				hasEdited = true;
			}
		}

		ImGui::EndTable();
	}

	return hasEdited;
}

bool shade::ImGuiLayer::SliderInt(const char* title, int* data, int min, int max, float cw1, float cw2)
{
	std::string _title = std::string("##") + title;
	bool hasEdited = false;

	if (ImGui::BeginTable(_title.c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp, { 0, 0 }))
	{
		ImGui::TableNextColumn();
		{
			ImGui::Text(title); ImGui::Dummy({ cw1, 0 });
		}
		ImGui::TableNextColumn();
		{
			if (!cw2)
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			else
			{
				ImGui::SetNextItemWidth(cw2);
			}

			if (ImGui::SliderInt(_title.c_str(), data, min, max))
			{
				hasEdited = true;
			}
		}

		ImGui::EndTable();
	}

	return hasEdited;
}

bool shade::ImGuiLayer::DrawButton(const char* title, const char* buttonTitle, float cw1, float cw2)
{
	std::string _title = std::string("##") + title;
	bool hasPresed = false;

	if (ImGui::BeginTable(_title.c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp, { 0, 0 }))
	{
		ImGui::TableNextColumn();
		{
			ImGui::Text(title); ImGui::Dummy({ cw1, 0 });
		}
		ImGui::TableNextColumn();
		{
			if (!cw2)
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			else
			{
				ImGui::SetNextItemWidth(cw2);
			}

			if (ImGui::Button(buttonTitle))
			{
				hasPresed = true;
			}
		}

		ImGui::EndTable();
	}

	return hasPresed;
}

bool shade::ImGuiLayer::DrawImGuizmo(glm::mat4& transform, const SharedPointer<Camera>& camera, ImGuizmo::OPERATION operation, const ImVec4& window)
{
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(window.x, window.y, window.z, window.w);

	auto cameraView = camera->GetView();
	auto cameraProjection = camera->GetProjection();
	// Flip Y back !
	cameraProjection[1][1] *= -1.0;

	ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), operation, ImGuizmo::WORLD, glm::value_ptr(transform), nullptr);

	if (ImGuizmo::IsUsing())
		return true;
	else
		return false;
}

bool shade::ImGuiLayer::ColorEdit3(const char* title, float* data, float cw1, float cw2)
{
	std::string _title = std::string("##") + title;
	bool hasEdited = false;

	if (ImGui::BeginTable(_title.c_str(), 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp, { 0, 0 }))
	{
		ImGui::TableNextColumn();
		{
			ImGui::Text(title); ImGui::Dummy({ cw1, 0 });
		}
		ImGui::TableNextColumn();
		{
			if (!cw2)
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			else
			{
				ImGui::SetNextItemWidth(cw2);
			}

			if (ImGui::ColorEdit3(_title.c_str(), data))
			{
				hasEdited = true;
			}
		}

		ImGui::EndTable();
	}

	return hasEdited;
}

void shade::ImGuiLayer::ShowExampleAppCustomNodeGraph(bool* opened)
{
	ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Example: Custom Node Graph", opened))
	{
		ImGui::End();
		return;
	}

	// Dummy
	struct Node
	{
		int     ID;
		char    Name[32];
		ImVec2  Pos, Size;
		float   Value;
		ImVec4  Color;
		int     InputsCount, OutputsCount;

		Node(int id, const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count) { ID = id; strcpy(Name, name); Pos = pos; Value = value; Color = color; InputsCount = inputs_count; OutputsCount = outputs_count; }

		ImVec2 GetInputSlotPos(int slot_no) const { return ImVec2(Pos.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)InputsCount + 1)); }
		ImVec2 GetOutputSlotPos(int slot_no) const { return ImVec2(Pos.x + Size.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)OutputsCount + 1)); }
	};
	struct NodeLink
	{
		int     InputIdx, InputSlot, OutputIdx, OutputSlot;

		NodeLink(int input_idx, int input_slot, int output_idx, int output_slot) { InputIdx = input_idx; InputSlot = input_slot; OutputIdx = output_idx; OutputSlot = output_slot; }
	};

	// State
	static ImVector<Node> nodes;
	static ImVector<NodeLink> links;
	static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
	static bool inited = false;
	static bool show_grid = true;
	static int node_selected = -1;

	// Initialization
	ImGuiIO& io = ImGui::GetIO();
	if (!inited)
	{
		nodes.push_back(Node(0, "MainTex", ImVec2(40, 50), 0.5f, ImColor(255, 100, 100), 1, 1));
		nodes.push_back(Node(1, "BumpMap", ImVec2(40, 150), 0.42f, ImColor(200, 100, 200), 1, 1));
		nodes.push_back(Node(2, "Combine", ImVec2(270, 80), 1.0f, ImColor(0, 200, 100), 2, 2));
		links.push_back(NodeLink(0, 0, 2, 0));
		links.push_back(NodeLink(1, 0, 2, 1));
		inited = true;
	}

	// Draw a list of nodes on the left side
	bool open_context_menu = false;
	int node_hovered_in_list = -1;
	int node_hovered_in_scene = -1;
	ImGui::BeginChild("node_list", ImVec2(100, 0));
	ImGui::Text("Nodes");
	ImGui::Separator();
	for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
	{
		Node* node = &nodes[node_idx];
		ImGui::PushID(node->ID);
		if (ImGui::Selectable(node->Name, node->ID == node_selected))
			node_selected = node->ID;
		if (ImGui::IsItemHovered())
		{
			node_hovered_in_list = node->ID;
			open_context_menu |= ImGui::IsMouseClicked(1);
		}
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginGroup();

	const float NODE_SLOT_RADIUS = 4.0f;
	const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

	// Create our child canvas
	ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", scrolling.x, scrolling.y);
	ImGui::SameLine(ImGui::GetWindowWidth() - 100);
	ImGui::Checkbox("Show grid", &show_grid);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(60, 60, 70, 200));
	ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
	ImGui::PopStyleVar(); // WindowPadding
	ImGui::PushItemWidth(120.0f);

	const ImVec2 offset = ImGui::GetCursorScreenPos() + scrolling;
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	// Display grid
	if (show_grid)
	{
		ImU32 GRID_COLOR = IM_COL32(200, 200, 200, 40);
		float GRID_SZ = 64.0f;
		ImVec2 win_pos = ImGui::GetCursorScreenPos();
		ImVec2 canvas_sz = ImGui::GetWindowSize();
		for (float x = fmodf(scrolling.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
			draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
		for (float y = fmodf(scrolling.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
			draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
	}

	// Display links
	draw_list->ChannelsSplit(2);
	draw_list->ChannelsSetCurrent(0); // Background
	for (int link_idx = 0; link_idx < links.Size; link_idx++)
	{
		NodeLink* link = &links[link_idx];
		Node* node_inp = &nodes[link->InputIdx];
		Node* node_out = &nodes[link->OutputIdx];
		ImVec2 p1 = offset + node_inp->GetOutputSlotPos(link->InputSlot);
		ImVec2 p2 = offset + node_out->GetInputSlotPos(link->OutputSlot);
		draw_list->AddBezierCubic(p1, p1 + ImVec2(+50, 0), p2 + ImVec2(-50, 0), p2, IM_COL32(200, 200, 100, 255), 3.0f);
	}

	// Display nodes
	for (int node_idx = 0; node_idx < nodes.Size; node_idx++)
	{
		Node* node = &nodes[node_idx];
		ImGui::PushID(node->ID);
		ImVec2 node_rect_min = offset + node->Pos;

		// Display node contents first
		draw_list->ChannelsSetCurrent(1); // Foreground
		bool old_any_active = ImGui::IsAnyItemActive();
		ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
		ImGui::BeginGroup(); // Lock horizontal position
		ImGui::Text("%s", node->Name);
		ImGui::SliderFloat("##value", &node->Value, 0.0f, 1.0f, "Alpha %.2f");
		ImGui::ColorEdit3("##color", &node->Color.x);
		ImGui::EndGroup();

		// Save the size of what we have emitted and whether any of the widgets are being used
		bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
		node->Size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
		ImVec2 node_rect_max = node_rect_min + node->Size;

		// Display node box
		draw_list->ChannelsSetCurrent(0); // Background
		ImGui::SetCursorScreenPos(node_rect_min);
		ImGui::InvisibleButton("node", node->Size);
		if (ImGui::IsItemHovered())
		{
			node_hovered_in_scene = node->ID;
			open_context_menu |= ImGui::IsMouseClicked(1);
		}
		bool node_moving_active = ImGui::IsItemActive();
		if (node_widgets_active || node_moving_active)
			node_selected = node->ID;
		if (node_moving_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			node->Pos = node->Pos + io.MouseDelta;

		ImU32 node_bg_color = (node_hovered_in_list == node->ID || node_hovered_in_scene == node->ID || (node_hovered_in_list == -1 && node_selected == node->ID)) ? IM_COL32(75, 75, 75, 255) : IM_COL32(60, 60, 60, 255);
		draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
		draw_list->AddRect(node_rect_min, node_rect_max, IM_COL32(100, 100, 100, 255), 4.0f);
		for (int slot_idx = 0; slot_idx < node->InputsCount; slot_idx++)
			draw_list->AddCircleFilled(offset + node->GetInputSlotPos(slot_idx), NODE_SLOT_RADIUS, IM_COL32(150, 150, 150, 150));
		for (int slot_idx = 0; slot_idx < node->OutputsCount; slot_idx++)
			draw_list->AddCircleFilled(offset + node->GetOutputSlotPos(slot_idx), NODE_SLOT_RADIUS, IM_COL32(150, 150, 150, 150));

		ImGui::PopID();
	}
	draw_list->ChannelsMerge();

	// Open context menu
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) || !ImGui::IsAnyItemHovered())
		{
			node_selected = node_hovered_in_list = node_hovered_in_scene = -1;
			open_context_menu = true;
		}
	if (open_context_menu)
	{
		ImGui::OpenPopup("context_menu");
		if (node_hovered_in_list != -1)
			node_selected = node_hovered_in_list;
		if (node_hovered_in_scene != -1)
			node_selected = node_hovered_in_scene;
	}

	// Draw context menu
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopup("context_menu"))
	{
		Node* node = node_selected != -1 ? &nodes[node_selected] : NULL;
		ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
		if (node)
		{
			ImGui::Text("Node '%s'", node->Name);
			ImGui::Separator();
			if (ImGui::MenuItem("Rename..", NULL, false, false)) {}
			if (ImGui::MenuItem("Delete", NULL, false, false)) {}
			if (ImGui::MenuItem("Copy", NULL, false, false)) {}
		}
		else
		{
			if (ImGui::MenuItem("Add")) { nodes.push_back(Node(nodes.Size, "New node", scene_pos, 0.5f, ImColor(100, 100, 200), 2, 2)); }
			if (ImGui::MenuItem("Paste", NULL, false, false)) {}
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	// Scrolling
	if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
		scrolling = scrolling + io.MouseDelta;

	ImGui::PopItemWidth();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::EndGroup();

	ImGui::End();
}

bool shade::ImGuiLayer::DrawGraphEditor(bool isOpend, const ImVec2& size)
{

	return false;
}


shade::ImGuiGraph::Node* shade::ImGuiGraph::m_spActiveNode = nullptr;

bool shade::ImGuiGraph::Show(const char* title, const ImVec2& size)
{
	// Need to remove it from here
	static ImGuiGraph::ViewContext	context;
	static std::vector<Node>		nodes = { Node({200, 300}, "Blend 2D") };

	ImGuiStyle unscaledStyle = ImGui::GetStyle();

	//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	//ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.f);
	//ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
	//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.2, 0.2, 0.2, 1 });

	ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
	{
		if (ImGui::Begin(title))
		{
			const ImVec2 windowPos = ImGui::GetCursorScreenPos(), canvasSize = ImGui::GetContentRegionAvail(), scrollRegionLocalPos(0, 0);

			ImRect canvas(windowPos, windowPos + canvasSize);

			ImVec2 offset = ImGui::GetCursorScreenPos() + (context.ViewPosition * context.Zoom.Zoom);

			//ImGui::PushClipRect(canvasRegion.Min, canvasRegion.Max, true);
			if (ImGui::BeginChild("#ImGuiGraph::ScrollRegion", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse))
			{
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				if (ImGui::IsItemActive()) m_spActiveNode = nullptr;

				drawList->ChannelsSplit(2);
				{
					Zoom(canvas, context);

					//drawList->ChannelsSetCurrent(0);

					Grid(drawList, windowPos, context, canvasSize, ImGui::ColorConvertFloat4ToU32(ImVec4{ 0.3, 0.3, 0.3, 1.0 }), ImGui::ColorConvertFloat4ToU32(ImVec4{ 0.5, 0.5, 0.5, 1.0 }), 64.f);

					//drawList->ChannelsSetCurrent(1); // Background

					ImGui::GetStyle().ScaleAllSizes(context.Zoom.Zoom);
					ImGui::SetWindowFontScale(context.Zoom.Zoom);
					{
						DrawNodes(drawList, offset, context, nodes);
					}
					ImGui::GetStyle() = unscaledStyle;

					//draw_list->ChannelsMerge();
					//drawList->ChannelsSetCurrent(0); // Background
					//DrawConnections();
				}

				ImGui::EndChild();
			}
			//ImGui::PopClipRect();

		}ImGui::End();
	}
	ImGui::PopStyleColor();
	//ImGui::PopStyleVar(5);

	return false;
}

void shade::ImGuiGraph::Zoom(ImRect region, ImGuiGraph::ViewContext& context)
{
	ImGuiIO& io = ImGui::GetIO(); // todo add as arg
	if (region.Contains(io.MousePos))
		UpdateZoom(context, io);

	UpdateView(context, io);
}

void shade::ImGuiGraph::UpdateZoom(ImGuiGraph::ViewContext& context, const ImGuiIO& io)
{
	if (io.MouseWheel <= -std::numeric_limits<float>::epsilon())
		context.Zoom.ZoomTarget *= 1.0f - context.Zoom.ZoomRatio;

	if (io.MouseWheel >= std::numeric_limits<float>::epsilon())
		context.Zoom.ZoomTarget *= 1.0f + context.Zoom.ZoomRatio;
}

void shade::ImGuiGraph::UpdateView(ImGuiGraph::ViewContext& context, const ImGuiIO& io)
{
	ImVec2 mouseWPosPre = CalculateMouseWorldPos(io, context);

	context.Zoom.ZoomTarget = ImClamp(context.Zoom.ZoomTarget, context.Zoom.MinZoom, context.Zoom.MaxZoom);
	context.Zoom.Zoom		= ImClamp(context.Zoom.Zoom, context.Zoom.MinZoom, context.Zoom.MaxZoom);

	context.Zoom.Zoom = ImLerp(context.Zoom.Zoom, context.Zoom.ZoomTarget, context.Zoom.ZoomLerp);

	ImVec2 mouseWPosPost = CalculateMouseWorldPos(io, context);

	if (ImGui::IsMousePosValid())
		context.ViewPosition += mouseWPosPost - mouseWPosPre;
}

ImVec2 shade::ImGuiGraph::CalculateMouseWorldPos(const ImGuiIO& io, const ImGuiGraph::ViewContext& context)
{
	return (io.MousePos - ImGui::GetCursorScreenPos()) / context.Zoom.Zoom;
}

void shade::ImGuiGraph::Grid(ImDrawList* drawList, ImVec2 windowPos, const ImGuiGraph::ViewContext& context, const ImVec2 canvasSize, ImU32 gridColor, ImU32 gridColor2, float gridSize)
{
	const float gridSpace	= gridSize * context.Zoom.Zoom;
	const int divx			= static_cast<int>(-context.ViewPosition.x / gridSize);
	const int divy			= static_cast<int>(-context.ViewPosition.y / gridSize);

	DrawGridLines(context.ViewPosition * context.Zoom.Zoom, canvasSize, gridSpace, windowPos, gridColor, gridColor2, drawList, divx, divy);
}

void shade::ImGuiGraph::DrawGridLines(const ImVec2& start, const ImVec2& canvasSize, const float gridSpace, const ImVec2& windowPos, const ImColor& gridColor, const ImColor& gridColor2, ImDrawList* drawList, int divx, int divy)
{
	for (float coord = fmodf(start.x, gridSpace); coord < canvasSize.x; coord += gridSpace, divx++)
		drawList->AddLine(ImVec2(coord, 0.0f) + windowPos, ImVec2(coord, canvasSize.y) + windowPos, !(divx % 10) ? gridColor2 : gridColor);

	for (float coord = fmodf(start.y, gridSpace); coord < canvasSize.y; coord += gridSpace, divy++)
		drawList->AddLine(ImVec2(0.0f, coord) + windowPos, ImVec2(canvasSize.x, coord) + windowPos, !(divy % 10) ? gridColor2 : gridColor);
}

void shade::ImGuiGraph::DrawNodes(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, std::vector<ImGuiGraph::Node>& nodes)
{
	ImGui::PushStyleColor(ImGuiCol_ChildBg, context.Style.NodeBackgroundColor);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, context.Style.Rounding);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, context.Style.Padding * context.Zoom.Zoom);
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.f, 10.f });
	for (auto& node : nodes)
	{
		ImGui::SetCursorScreenPos(offset + node.Position * context.Zoom.Zoom);

		if (ImGui::BeginChild(node.Name.c_str(), 
			node.Size * context.Zoom.Zoom,
			true, 
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove ))
		{
		
			if (ImGui::IsItemActive()) { m_spActiveNode = &node; }

			MoveNode(ImGui::GetIO(), context, node);

			DrawNode(drawList, offset, context, (m_spActiveNode == &node), node);

		} ImGui::EndChild();
	}

	ImGui::PopStyleColor(1); ImGui::PopStyleVar(2);
}

void shade::ImGuiGraph::DrawNode(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, bool isActive, ImGuiGraph::Node& node)
{
	ImGui::PushStyleColor(ImGuiCol_Text, context.Style.HeaderTextColor);

	if (isActive) DrawBorder(drawList, offset, context, node);

	DrawHeader(drawList, offset, context, node);

	const float enpointY = (context.Style.HeaderHeight + 25.f) * context.Zoom.Zoom;

	DrawEndpoints(drawList, offset, context, enpointY, node);

	//drawList->AddCircleFilled();

	ImVec2 const	p1 = offset;
	ImVec2 const	p4 = ImGui::GetMousePos();
	ImVec2 const	p2 = p1 + ImVec2(+50, 0);
	ImVec2 const	p3 = p4 + ImVec2(-50, 0);

	//drawList->AddBezierCubic(p1, p2, p3, p4, ImGui::ColorConvertFloat4ToU32(ImVec4{ 0.4, 0.8, 0.2, 1.0 }), 5.f);

	ImGui::PopStyleColor(1);
}

float shade::ImGuiGraph::DrawHeader(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, ImGuiGraph::Node& node)
{
	ImGui::SetWindowFontScale(1.5);
	
	ImGui::BeginGroup();
	{
		ImGui::Text(node.Name.c_str());
	}
	ImGui::EndGroup();

	ImGui::SetWindowFontScale(1);

	const ImVec2 scaledPosition = node.Position * context.Zoom.Zoom;
	const ImVec2 pMin = (offset + scaledPosition);
	const ImVec2 pMax = (offset + scaledPosition + (ImVec2 { node.Size.x, context.Style.HeaderHeight } * context.Zoom.Zoom));

	drawList->AddRectFilled(pMin, pMax, ImGui::ColorConvertFloat4ToU32(node.Style.HeaderColor), context.Style.Rounding, ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersTopLeft);

	//return pMax.y;
	return node.Position.y + context.Style.HeaderHeight;
}

void shade::ImGuiGraph::DrawFooter(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, ImGuiGraph::Node& node)
{

}

void shade::ImGuiGraph::DrawEndpoints(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, float yOffset, ImGuiGraph::Node& node)
{
	const ImVec2 e1 = DrawInputEndpoint(drawList, offset, context, yOffset, node);
	const ImVec2 e2 = DrawOutputEndpoint(drawList, offset, context, yOffset, node);
	
	DrawConnection(drawList, {0, 0}, context, e1, ImGui::GetMousePos());
	DrawConnection(drawList, {0, 0}, context, e2, ImGui::GetMousePos());
}

ImVec2 shade::ImGuiGraph::DrawInputEndpoint(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, float yOffset, ImGuiGraph::Node& node)
{
	const ImVec2 scaledPosition = node.Position * context.Zoom.Zoom;

	ImVec2 pInput = (offset + scaledPosition);

	pInput.y += yOffset;

	drawList->AddCircleFilled(pInput, context.Style.EndpointRadius * context.Zoom.Zoom, ImGui::ColorConvertFloat4ToU32(node.Style.HeaderColor));

	return pInput;
}

void shade::ImGuiGraph::DrawConnection(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, const ImVec2& from, const ImVec2& till)
{
	ImVec2 const	p1 = offset + from;
	ImVec2 const	p4 = offset + till;
	ImVec2 const	p2 = p1 + ImVec2(+50, 0);
	ImVec2 const	p3 = p4 + ImVec2(-50, 0);

	drawList->AddBezierCubic(p1, p2, p3, p4, ImGui::ColorConvertFloat4ToU32(context.Style.ConnectionColor), context.Style.ConnectionThickness * context.Zoom.Zoom);
}

ImVec2 shade::ImGuiGraph::DrawOutputEndpoint(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, float yOffset, ImGuiGraph::Node& node)
{
	const ImVec2 scaledPosition = node.Position * context.Zoom.Zoom;

	ImVec2 pOutput = offset + scaledPosition + ImVec2{ node.Size.x * context.Zoom.Zoom, yOffset };

	drawList->AddCircleFilled(pOutput, context.Style.EndpointRadius * context.Zoom.Zoom, ImGui::ColorConvertFloat4ToU32(node.Style.HeaderColor));

	return pOutput;
}

void shade::ImGuiGraph::DrawBorder(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, ImGuiGraph::Node& node)
{
	const ImVec2 scaledPosition = node.Position * context.Zoom.Zoom;
	const ImVec2 pMin = (offset + scaledPosition);
	const ImVec2 pMax = (offset + scaledPosition + node.Size * context.Zoom.Zoom);

	drawList->AddRect(pMin, pMax, ImGui::ColorConvertFloat4ToU32(context.Style.NodeBorderColor), context.Style.Rounding, 0, context.Style.NodeBorderWidth);
}

void shade::ImGuiGraph::MoveNode(const ImGuiIO& io, const ImGuiGraph::ViewContext& context, ImGuiGraph::Node& node)
{
	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
	{
		ImVec2 delta = io.MouseDelta / context.Zoom.Zoom;

		if (fabsf(delta.x) >= 0.1f || fabsf(delta.y) >= 0.1f)
		{
			node.Position.x += delta.x;
			node.Position.y += delta.y;
		}
	}
}
