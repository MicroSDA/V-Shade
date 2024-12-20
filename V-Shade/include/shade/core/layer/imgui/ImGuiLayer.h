#pragma once
#include <shade/core/layer/Layer.h>
#include <shade/core/layer/imgui/ImGuiRender.h>
#include <shade/core/layer/imgui/ImGuiThemeEditor.h>
#include <ImGuizmo/ImGuizmo.h>
#include <ImGuizmo/ImSequencer.h>
#include <shade/core/layer/imgui/ImGuiGraph.h>
#include <shade/core/layer/imgui/ImGuiCurve.h>

namespace shade
{
	class SHADE_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		virtual ~ImGuiLayer();
		virtual void OnRenderBegin() override;
		virtual void OnRenderEnd() override;
		
		ImGuiContext* GetImGuiContext();
		void DrawImage(SharedPointer<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor);
		void DrawImage(SharedPointer<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor, std::uint32_t mip);
		void DrawImage(Asset<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor);
		void DrawImageLayerd(SharedPointer<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor, std::uint32_t layer, bool isGrayScale = false, const glm::vec2& clamp = glm::vec2{0.0, 1.0});

		template<typename T, typename ...Args>
		inline static T* GetGlobalValue(const std::string& name, Args && ...args)
		{
			if (m_sGlobalsValues[name] == nullptr)
			{
				m_sGlobalsValues[name] = new T(std::forward<Args>(args)...);
			}

			return reinterpret_cast<T*>(m_sGlobalsValues[name]);
		}
		static inline std::unordered_map<std::string, void*> m_sGlobalsValues;
	public:
		ImGuiViewport* m_Viewport;
		int m_WindowFlags;
		int m_DockSpaceFlags;

		struct SceneVeiwPort
		{
			ImVec4		ViewPort = { 1, 1, 1, 1 };
			glm::vec2	MousePosition = { 0,0 };
		} m_SceneViewPort;

		template<typename Callback, typename ...Args>
		void ShowWindowBar(const char* title, bool *p_open, Callback callback, Args && ...args)
		{
			if (ImGui::Begin(title, p_open, ImGuiWindowFlags_MenuBar))
				std::invoke(callback, std::forward<Args>(args)...);
			ImGui::End();
		}
		template<typename Callback, typename ...Args>
		static void ShowWindowBarOverlay(const char* title, ImGuiViewport* veiwport, Callback callback, Args&& ...args)
		{
			ImGui::SetNextWindowViewport(veiwport->ID);
			ImGui::SetNextWindowBgAlpha(0.5f); // Transparent background
			ImGui::SetNextWindowPos(ImVec2{ ImGui::GetWindowPos().x + 5.0f, ImGui::GetWindowPos().y + 50.0f }, ImGuiCond_Always);
			//ImGui::SetNextWindowSize(ImVec2{ ImGui::GetWindowSize().x - 20.0f,0}, ImGuiCond_Always);

			if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_Tooltip))
			{
				std::invoke(callback, std::forward<Args>(args)...);
			}
			ImGui::End();
		}

		template<typename Callback, typename ...Args>
		static void BeginWindowOverlay(const char* title, ImGuiViewport* veiwport, ImGuiID id, const ImVec2& size, const ImVec2& position, float alpha, Callback callback, Args&& ...args)
		{
			ImGui::SetNextWindowViewport(veiwport->ID);
			ImGui::SetNextWindowBgAlpha(alpha); 

			float y = (veiwport->Size.y < (position.y + size.y)) ? position.y - (size.y + ImGui::GetStyle().IndentSpacing + ImGui::GetFrameHeight() + 7.f) : position.y;
			
			ImGui::SetNextWindowPos(ImVec2{ position.x, y }, ImGuiCond_Always);
			ImGui::SetNextWindowSize(size, ImGuiCond_Always);

			ImGui::PushID(id);

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyleColorVec4(ImGuiCol_ChildBg));
			bool isOpen = ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
			ImGui::PopStyleColor();

			if (isOpen)
			{
				std::invoke(callback, std::forward<Args>(args)...);
				ImGui::End();
			}
			ImGui::PopID();
			
		}

		template<typename Component, typename Callback, typename EditCallback, typename ...Args>
		void DrawComponent(const char* title, ecs::Entity& entity, Callback callback, EditCallback editCallback, Args&& ...args)
		{
			if (entity.IsValid())
			{
				if (entity.HasComponent<Component>())
				{

					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { ImGui::GetStyle().FramePadding.x , ImGui::GetStyle().FramePadding.y * 2.5f });
					bool isTreeOpen = ImGui::TreeNodeEx(title, ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed);
					ImGui::PopStyleVar();
					// If component has not been deleted
					if (!std::invoke(editCallback, isTreeOpen))
					{
						if (isTreeOpen)
							std::invoke(callback, std::forward<Args>(args)...);
					}
					if (isTreeOpen)
						ImGui::TreePop();
				}
			}
		}
		template<typename Comp>
		bool EditComponent(ecs::Entity& entity, const ImVec2& size, bool isTreeNodeOpend)
		{
			float buttonSize = ImGui::GetFrameHeight();

			ImGui::AlignTextToFramePadding();
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - buttonSize - ImGui::GetStyle().FramePadding.x + (isTreeNodeOpend ? ImGui::GetStyle().IndentSpacing : 0.0f));

			// Create id for component
			std::string id = typeid(Comp).name();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
			
			if (ImGui::Button(std::string("...##"+ id).c_str()))
			{
				ImGui::OpenPopup(std::string("##EditComponentPopup" + id).c_str());
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			if (ImGui::BeginPopup(std::string("##EditComponentPopup" + id).c_str()))
			{
				if (ImGui::MenuItem("Remove component"))
				{
					entity.RemoveComponent<Comp>();
					ImGui::EndPopup();
					return true;
				}
				else
				{
					ImGui::EndPopup();
					return false;
				}
			}

			return false;
		}
		template<typename Comp, typename Call, typename ...Args>
		inline void AddComponent(const char* title, const bool& isMenu, ecs::Entity& entity, Call callback, Args && ...args)
		{
			if (entity.IsValid())
			{
				if (!entity.HasComponent<Comp>())
				{
					if (isMenu)
					{
						if (ImGui::BeginMenu(title))
						{
							std::invoke(callback, std::forward<Args>(args)...);
							ImGui::EndMenu();
						}
					}
					else
					{
						if (ImGui::MenuItem(title))
							std::invoke(callback, std::forward<Args>(args)...);
					}
				}
			}
		}
		template<typename Callback>
		void DrawModal(const char* name, bool& isOpen, Callback callback)
		{
			if (isOpen)
			{
				ImGui::OpenPopup(name);
				ImVec2 center = ImGui::GetMainViewport()->GetCenter();
				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
				if (ImGui::BeginPopupModal(name, NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysAutoResize))
				{
					std::invoke(callback);

					//ImGui::SameLine();
					ImGui::Dummy({0, ImGui::GetContentRegionAvail().y - 25.f});
					ImGui::Separator();
					if (ImGui::Button("Close"))
					{
						ImGui::CloseCurrentPopup();
						isOpen = false;
					}

					ImGui::EndPopup();
				}
			}
		}
		static void HelpMarker(const char* marker, const char* desc);
		
		static bool InputTextCol(const char* title, std::string& str, float cw1 = 0.0f, float cw2 = 0.0);
		static bool InputTextD(const char* title, std::string& str);
		static bool ComboCol(const char* id, std::string& selected, std::vector<std::string>& elements, ImGuiSelectableFlags selectedFlags, ImGuiComboFlags comboFlags, const float& cw1 = 0.0f, const float& cw2 = 0.0);
		static bool DrawCombo(const char* id, std::string& selected, std::vector<std::string>& elements, ImGuiSelectableFlags selectedFlags, ImGuiComboFlags comboFlags);
		static bool DrawComboWithIndex(const char* id, std::uint32_t& selected, std::vector<std::string>& elements, ImGuiSelectableFlags selectedFlags, ImGuiComboFlags comboFlags);

		static bool ToggleButton(const char* str_id, bool* v, const char8_t* c = nullptr);
		static void ToggleButtonIcon(const char* str_id, bool* v, const char8_t* c, std::size_t fontIndex, float scale);


		bool DragFloatR(const char* id, float* v, float reset, float min = 0.f, float max = FLT_MAX, float step = 0.01f, float length = 0.f);

		bool DragFloat(const char* title, float* data, float step = 0.01f, float min = 0.0, float max = FLT_MAX, float cw1 = 0.0f, float cw2 = 0.0);
		bool DragFloat3(const char* title, float* data, float resetValue, float step = 0.01f, float min = 0.0, float max = FLT_MAX, float cw1 = 0.0f, float cw2 = 0.0);
		bool DragInt(const char* title, int* data, int step = 1, int min = 0, int max = INT_MAX, float cw1 = 0.0f, float cw2 = 0.0, bool readOnly = false);
		bool SliderInt(const char* title, int* data, int min = 0, int max = INT_MAX, float cw1 = 0.0f, float cw2 = 0.0);
		bool DrawImGuizmo(glm::mat4& transform, const SharedPointer<Camera>& camera, ImGuizmo::OPERATION operation, const ImVec4& window);
		bool ColorEdit3(const char* title, float* data, float cw1 = 0.0f, float cw2 = 0.0);
		bool DrawButton(const char* title, const char* buttonTitle, float cw1 = 0.0f, float cw2 = 0.0f);



		static void TextUTF8(const std::u8string& string);
		static void DrawFontIcon(const char8_t* c, std::size_t fontIndex, float size, const char* hint = nullptr);
		static bool IconButton(const char* id, const char8_t* c, std::size_t fontIndex, float scale = 1.f);
        // Dummy data structure provided for the example.
        // Note that we storing links as indices (not ID) to make example code shorter.
		static void ShowExampleAppCustomNodeGraph(bool* opened);

	private:
		SharedPointer<ImGuiRender> m_ImGuiRender;
		
	};
}