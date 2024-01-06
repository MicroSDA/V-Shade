#pragma once

#include <shade/core/layer/Layer.h>
#include <shade/core/layer/imgui/ImGuiRender.h>
#include <shade/core/layer/imgui/ImGuiThemeEditor.h>
#include <ImGuizmo/ImGuizmo.h>
#include <ImGuizmo/ImSequencer.h>

namespace shade
{
	class SHADE_API ImGuiGraph
	{
	public:

		struct Node
		{
			ImVec2 Size			= ImVec2 {250, 250};
			std::string Name	= "Node";
			ImVec2 Position		= ImVec2 { 0, 0 };
			struct
			{

				ImVec4 HeaderColor = ImVec4{ 0.4, 0.8, 0.2, 1.0 };
			} Style;

		};

		struct ViewContext
		{
			struct
			{
				float Rounding				= 7.f;
				float NodeBorderWidth		= 3.f;
				float HeaderHeight			= 30.f;
				float EndpointRadius		= 7.f;
				float ConnectionThickness	= 5.f;
				ImVec4 NodeBorderColor		= ImVec4{ 0.7f, 0.7f, 0.7f, 1.f };
				ImVec4 NodeBackgroundColor  = ImVec4{ 0.2f, 0.2f, 0.2f, 1.f };
				ImVec4 HeaderTextColor		= ImVec4{ 0.0f, 0.0f, 0.0f, 1.f };
				ImVec4 ConnectionColor		= ImVec4{ 0.4f, 0.8f, 0.2f, 1.f };
				ImVec2 Padding				= ImVec2{ 10.f, 5.f };
			} Style;
			struct 
			{
				float ZoomTarget		= 1.f;
				float Zoom				= 1.f;
				float MinZoom			= 0.01f;
				float MaxZoom			= 2.f;
				float ZoomLerp			= 0.20f;
				float ZoomRatio			= 0.1f;
			} Zoom;

			ImVec2 ViewPosition			= ImVec2(0.f, 0.f);
		};
	public:
		static bool Show(const char* title, const ImVec2& size);
	private:
		static void Zoom(ImRect region, ImGuiGraph::ViewContext& context);
		static void UpdateZoom(ImGuiGraph::ViewContext& context, const ImGuiIO& io);
		static void UpdateView(ImGuiGraph::ViewContext& context, const ImGuiIO& io);
		static ImVec2 CalculateMouseWorldPos(const ImGuiIO& io, const ImGuiGraph::ViewContext& context);

		static void Grid(ImDrawList* drawList, ImVec2 windowPos, const ImGuiGraph::ViewContext& context, const ImVec2 canvasSize, ImU32 gridColor, ImU32 gridColor2, float gridSize);
		static void DrawGridLines(
			const ImVec2& start, 
			const ImVec2& canvasSize, 
			const float gridSpace,
			const ImVec2& windowPos,
			const ImColor& gridColor, const ImColor& gridColor2,
			ImDrawList* drawList, int divx, int divy);


		static void DrawNodes(ImDrawList* drawList,const ImVec2& offset, const ImGuiGraph::ViewContext& context, std::vector<ImGuiGraph::Node>& nodes);
		static void DrawNode(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context,  bool isActive, ImGuiGraph::Node& node);
		static float DrawHeader(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, ImGuiGraph::Node& node);
		static void DrawFooter(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, ImGuiGraph::Node& node);
		static void DrawEndpoints(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, float yOffset, ImGuiGraph::Node& node);
		static ImVec2 DrawInputEndpoint(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, float yOffset, ImGuiGraph::Node& node);
		static ImVec2 DrawOutputEndpoint(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, float yOffset, ImGuiGraph::Node& node);
		static void DrawConnection(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, const ImVec2& from, const ImVec2& till);
		static void DrawBorder(ImDrawList* drawList, const ImVec2& offset, const ImGuiGraph::ViewContext& context, ImGuiGraph::Node& node);

		static void MoveNode(const ImGuiIO& io, const ImGuiGraph::ViewContext& context, ImGuiGraph::Node& node);
	private:
		static Node* m_spActiveNode;

	};
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
	protected:
		ImGuiViewport* m_Viewport;
		int m_WindowFlags;
		int m_DockSpaceFlags;

		struct SceneVeiwPort
		{
			ImVec4		ViewPort = { 1, 1, 1, 1 };
			glm::vec2	MousePosition = { 0,0 };
		} m_SceneViewPort;

		template<typename Callback, typename ...Args>
		void ShowWindowBar(const char* title, Callback callback, Args && ...args)
		{
			if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_MenuBar))
				std::invoke(callback, std::forward<Args>(args)...);
			ImGui::End();
		}
		template<typename Callback, typename ...Args>
		void ShowWindowBarOverlay(const char* title, ImGuiViewport* veiwport, Callback callback, Args && ...args)
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
		static void HelpMarker(const char* desc)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(desc);
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}
		bool InputTextCol(const char* title, std::string& str, float cw1 = 0.0f, float cw2 = 0.0);
		bool InputTextD(const char* title, std::string& str);
		bool ComboCol(const char* id, std::string& selected, std::vector<std::string>& elements, ImGuiSelectableFlags selectedFlags, ImGuiComboFlags comboFlags, const float& cw1 = 0.0f, const float& cw2 = 0.0);
		bool DrawCombo(const char* id, std::string& selected, std::vector<std::string>& elements, ImGuiSelectableFlags selectedFlags, ImGuiComboFlags comboFlags);

		bool DragFloat(const char* title, float* data, float step = 0.01f, float min = 0.0, float max = FLT_MAX, float cw1 = 0.0f, float cw2 = 0.0);
		bool DragFloat3(const char* title, float* data, float resetValue, float step = 0.01f, float min = 0.0, float max = FLT_MAX, float cw1 = 0.0f, float cw2 = 0.0);
		bool DragInt(const char* title, int* data, int step = 1, int min = 0, int max = INT_MAX, float cw1 = 0.0f, float cw2 = 0.0, bool readOnly = false);
		bool SliderInt(const char* title, int* data, int min = 0, int max = INT_MAX, float cw1 = 0.0f, float cw2 = 0.0);
		bool DrawImGuizmo(glm::mat4& transform, const SharedPointer<Camera>& camera, ImGuizmo::OPERATION operation, const ImVec4& window);
		bool ColorEdit3(const char* title, float* data, float cw1 = 0.0f, float cw2 = 0.0);
		bool DrawButton(const char* title, const char* buttonTitle, float cw1 = 0.0f, float cw2 = 0.0f);

        // Dummy data structure provided for the example.
        // Note that we storing links as indices (not ID) to make example code shorter.
		static void ShowExampleAppCustomNodeGraph(bool* opened);

		bool DrawGraphEditor(bool isOpend, const ImVec2& size);

	private:
		SharedPointer<ImGuiRender> m_ImGuiRender;
	};
}