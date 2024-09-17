#include "shade_pch.h"
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
void shade::ImGuiLayer::DrawImage(Asset<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor)
{
	m_ImGuiRender->DrawImage(texture, size, borderColor);
}
void shade::ImGuiLayer::HelpMarker(const char* marker, const char* desc)
{
	//ImGui::SameLine();
	ImGui::TextDisabled(marker);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 45.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
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
bool shade::ImGuiLayer::DrawComboWithIndex(const char* title, std::uint32_t& selected, std::vector<std::string>& elements, ImGuiSelectableFlags selectedFlags, ImGuiComboFlags comboFlags)
{
	bool hasBeenSelected = false;
	if (ImGui::BeginCombo(title, elements[selected].c_str(), comboFlags)) // The second parameter is the label previewed before opening the combo.
	{
		for (std::uint32_t i = 0; i < elements.size(); ++i)
		{
			bool isSelected = (elements[selected] == elements[i]);
			if (ImGui::Selectable(elements[i].c_str(), isSelected, selectedFlags))
			{
				selected = i;
				hasBeenSelected = true;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}
	return hasBeenSelected;
}

void shade::ImGuiLayer::ToggleButtonIcon(const char* str_id, bool* v, const char8_t* c, std::size_t fontIndex, float scale)
{
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float size = ImGui::GetIO().Fonts->Fonts[fontIndex]->FontSize * scale;

	ImGui::InvisibleButton(str_id, ImVec2(size, size));
	if (ImGui::IsItemClicked())
		*v = !*v;

	ImU32 col_bg;
	if (ImGui::IsItemHovered())
		col_bg = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
	else
	{
		if (*v)
		{
			col_bg = col_bg = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);
		}
		else
		{
			col_bg = col_bg = ImGui::GetColorU32(ImGui::GetStyle().Colors[ImGuiCol_Button]);
		}
	}


	ImGui::SetCursorScreenPos(p);

	ImGui::PushStyleColor(ImGuiCol_Text, col_bg);
	ImGuiLayer::DrawFontIcon(c, fontIndex, scale);
	ImGui::PopStyleColor();
}

void shade::ImGuiLayer::ToggleButton(const char* str_id, bool* v)
{
	ImVec2 p = ImGui::GetCursorScreenPos();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	float height = ImGui::GetFrameHeight();
	float width = height * 1.55f;
	float radius = height * 0.50f;

	ImGui::InvisibleButton(str_id, ImVec2(width, height));
	if (ImGui::IsItemClicked())
		*v = !*v;

	float t = *v ? 1.0f : 0.0f;

	ImGuiContext& g = *GImGui;
	float ANIM_SPEED = 0.08f;
	if (g.LastActiveId == g.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
	{
		float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
		t = *v ? (t_anim) : (1.0f - t_anim);
	}

	ImU32 col_bg;
	if (ImGui::IsItemHovered())
		col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
	else
		col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

	draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
	draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}

bool shade::ImGuiLayer::DragFloatR(const char* id, float* v, float reset, float min, float max, float step, float length)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	const float radius = ImGui::GetFrameHeight() / 2.7f, thickness = 2.5f, trinagleLength = 5.5f, halfLength = trinagleLength / 2.f, halfThickness = thickness / 2.f;

	const bool isEdit = ImGui::DragFloat(id, v, step, min, max); ImGui::SameLine();

	const ImVec2 position = ImGui::GetCursorScreenPos(), center = position + ImVec2{ radius, ImGui::GetFrameHeight() / 2.f };
	const ImRect rect = { position, { position.x + radius * 2.f, position.y + ImGui::GetFrameHeight()} };

	const ImVec2 p1 = { center.x - halfLength, center.y - radius + trinagleLength + halfThickness },
		p2 = { center.x - halfLength, center.y - radius - trinagleLength + halfThickness },
		p3 = { center.x + halfLength, center.y - radius + halfThickness };

	const ImU32 bc = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Button]);
	const ImU32 bca = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
	const ImU32 bch = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);

	const bool isClick = ImGui::InvisibleButton(std::string(id + std::string("ResetButton")).c_str(), { radius * 2.f, ImGui::GetFrameHeight() });

	if (isClick) *v = reset;

	const ImU32 col = isClick ? bca : ImGui::IsItemHovered() ? bch : bc;

	drawList->AddCircle(center, radius, col, 0, thickness);
	drawList->AddTriangleFilled(p1, p2, p3, col);
	// Debug frame
	//drawList->AddRect(rect.Min, rect.Max, col);

	return isEdit || isClick;
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

void shade::ImGuiLayer::TextUTF8(const std::u8string& string)
{
	ImGui::TextUnformatted((char*)string.c_str(), (char*)string.c_str() + string.size());
}
void shade::ImGuiLayer::DrawFontIcon(const char8_t* c, std::size_t fontIndex, float scale, const char* hint)
{
	ImGui::GetIO().Fonts->Fonts[fontIndex]->Scale = scale;
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[fontIndex]);
	TextUTF8(c);
	ImGui::PopFont();
	ImGui::GetIO().Fonts->Fonts[fontIndex]->Scale = 1.f;

	if (hint && ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 45.0f);
		ImGui::TextUnformatted(hint);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

bool shade::ImGuiLayer::IconButton(const char* id, const char8_t* c, std::size_t fontIndex, float scale)
{
	const float fontSize = ImGui::GetIO().Fonts->Fonts[fontIndex]->FontSize;
	const float fontScale = ImGui::GetCurrentWindow()->FontWindowScale;
	const ImVec2 sp = ImGui::GetCursorScreenPos();

	const float buttonDim = ImGui::GetFrameHeight() * scale;
	const float coef = buttonDim / (fontSize * fontScale);

	ImGui::InvisibleButton(id, { buttonDim, buttonDim });

	const bool isClicked = ImGui::IsItemClicked();
	const bool isHoverd  = ImGui::IsItemHovered();
	
	ImVec4 color = isClicked ? ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] : isHoverd ? ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] : ImGui::GetStyle().Colors[ImGuiCol_Button];

	ImGui::SetCursorScreenPos(sp);

	ImGui::PushStyleColor(ImGuiCol_Text, color);
	shade::ImGuiLayer::DrawFontIcon(c, fontIndex, coef);
	ImGui::PopStyleColor();

	//ImDrawList* drawList = ImGui::GetWindowDrawList();
	//const ImRect buttonRect{ sp, sp + ImVec2{ buttonDim, buttonDim} };
	//drawList->AddRectFilled(buttonRect.Min, buttonRect.Max, ImGui::ColorConvertFloat4ToU32(color));

	return isClicked && isHoverd;
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

	//ImGuizmo::AllowAxisFlip();
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