#include "shade_pch.h"
#include "ImGuiLayer.h"

#include <shade/core/application/Application.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>
#include <shade/platforms/render/vulkan/VulkanSwapChain.h>
#include <ImGui/misc/cpp/imgui_stdlib.cpp>

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

	if (ImGui::BeginTable(_title.c_str(), 2, ImGuiTableFlags_SizingStretchProp, {0, 0}))
	{
		ImGui::TableNextColumn();
		{
			ImGui::Text(title); ImGui::Dummy({ cw1, 0 });
		}
		ImGui::TableNextColumn();
		{
			if(!cw2)
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
