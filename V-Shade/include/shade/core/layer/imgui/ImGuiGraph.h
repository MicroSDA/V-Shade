#pragma once
#include <shade/config/ShadeAPI.h>
#include <ImGui/imgui.h>

#define IMGUI_DEFINE_MATH_OPERATORS

#include <ImGui/imgui_internal.h>

namespace shade
{
	struct SHADE_API ImGuiGraphNodeRender
	{
		static void DrawGrid(
			ImDrawList* drawList,
			const ImVec2& windowPosition,
			const ImVec2& canvasSize,
			const ImVec2& canvasPosition,
			float scaleFactor,
			ImU32 gridColor,
			ImU32 gridColor2,
			float gridSize);

		static void DrawGridLines(const ImVec2& start, const ImVec2& canvasSize, const float gridSpace, const ImVec2& windowPos, const ImColor& gridColor, const ImColor& gridColor2, ImDrawList* drawList, int divx, int divy);
		static bool DrawEndpoint(ImDrawList* drawList, const ImVec2& offset, float radius, float scaleFactor, const ImVec2& screenPosition, const ImVec4& color, const ImVec4& hoveredColor);
		static void DrawConnection(ImDrawList* drawList, const ImVec2& offset, float scaleFactor, const ImVec2& from, const ImVec2& till, const ImVec4& connectionColor, float thickness);
		static bool DrawReferNodeConnection(ImDrawList* drawList,const ImVec2& offset,float scaleFactor,const ImVec2& point,const ImVec2& fontSizePx,const ImVec4& connectionColor,float iconSize);
	};


	template<typename Graph, typename NodeIdentifier, typename EndpointIdentifier, typename Node>
	class ImGuiGraphPrototype;

	struct EndpointPrototype
	{
		struct VisualStyle
		{
			ImVec4 Color		= ImVec4{ 0.0f, 0.f, 0.f, 1.f };
			ImVec4 ColorHovered = ImVec4{ 0.2f, 0.2f, 0.2f, 1.f };
			float Radius		= 5.f;
		};

		enum EndpointType : std::uint8_t { Input = 0, Output = 1, MAX_ENUM };

		EndpointPrototype(EndpointType type, const std::string& name = std::string("Endpoint name")) :
			m_Type(type), m_Name(name)
		{}
		~EndpointPrototype() = default;
		/////////////////////////////////////////////////////////////////////////////////////////////////
		SHADE_INLINE		EndpointType GetType()							const	{ return m_Type; }
		SHADE_INLINE const	std::string& GetName()							const	{ return m_Name; }
		SHADE_INLINE const	ImVec2 GetScreenPosition()						const	{ return m_ScreenPosition; }
		SHADE_INLINE		void SetScreenPosition(const ImVec2& position)			{ m_ScreenPosition = position; }
		/////////////////////////////////////////////////////////////////////////////////////////////////
		VisualStyle Style;
	private:
		std::string		m_Name;
		EndpointType	m_Type;
		ImVec2			m_ScreenPosition;

	};

	template<typename EndpointIdentifier>
	using EndpointsPrototype = std::array<std::map<EndpointIdentifier, EndpointPrototype>, EndpointPrototype::MAX_ENUM>;

	template<typename NodeIdentifier, typename EndpointIdentifier>
	struct ConnectionPrototype
	{
		NodeIdentifier			InputNodeIdentifier;
		EndpointIdentifier		InputEndpointIdentifier;
		NodeIdentifier			OutputNodeIdentifier;
		EndpointIdentifier		OutputEndpointIdentifier;
	};

	template<typename NodeIdentifier, typename EndpointIdentifier>
	using ConnectionsPrototype = std::vector<ConnectionPrototype<NodeIdentifier, EndpointIdentifier>>;

	template<typename NodeIdentifier, typename EndpointIdentifier, typename Node>
	class GraphNodePrototype
	{
	public:
		struct VisualStyle
		{
			ImVec4 HeaderColor = ImVec4{ 0.5f, 0.7f, 0.f, 1.f };
			ImVec4 HeaderTextColor = ImVec4{ 0.0f, 0.0f, 0.f, 1.f };

			ImVec4 InputEndpointsColor = ImVec4{ 0.5f, 0.7f, 0.f, 1.f };
			ImVec4 InputEndpointsColorHovered = ImVec4{ InputEndpointsColor.x * 2.f,InputEndpointsColor.y * 2.f, InputEndpointsColor.z * 2.f, InputEndpointsColor.w };

			ImVec4 OutPutEndpointsColor = ImVec4{ 0.5f, 0.7f, 0.f, 1.f };
			ImVec4 OutPutEndpointsColorHovered = ImVec4{ OutPutEndpointsColor.x * 2.f,OutPutEndpointsColor.y * 2.f, OutPutEndpointsColor.z * 2.f, OutPutEndpointsColor.w };

			ImVec4 BorderColor = ImVec4{ 0.4f, 0.4f, 0.4f, 1.f };
			ImVec4 BackgroundColor = ImVec4{ 0.15f, 0.15f, 0.15f, 1.f };
			ImVec4 TextColor = ImVec4{ 0.0f, 0.0f, 0.0f, 1.f };

			ImVec2 Size = ImVec2{ 230.f, 200.f };
			std::string	Title = "Node title";
		};

		GraphNodePrototype(NodeIdentifier id, Node node) : m_Identifier(id), m_NodeValue(node) {}
		~GraphNodePrototype() = default;

		//////////////////////////////////////////////////////////////////////////
		SHADE_INLINE const	Node& GetNode()								const { return m_NodeValue; }
		SHADE_INLINE		Node& GetNode() { return m_NodeValue; }
		SHADE_INLINE		void SetScreenPosition(const ImVec2& position) { m_ScreenPosition = position; }
		SHADE_INLINE const	ImVec2& GetScreenPosition()					const { return m_ScreenPosition; }
		SHADE_INLINE		ImVec2& GetScreenPosition() { return m_ScreenPosition; }

		//////////////////////////////////////////////////////////////////////////

		virtual ConnectionsPrototype<NodeIdentifier, EndpointIdentifier> ReceiveConnections() const = 0;
		virtual EndpointsPrototype<NodeIdentifier> ReceiveEndpoints() const = 0;
		virtual void ProcessSideBar() {};
		virtual void ProcessBodyContent() {};
		virtual void ProcessEndpoint(const EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint) {};

		SHADE_INLINE ConnectionsPrototype<NodeIdentifier, EndpointIdentifier>& GetConnections()
		{
			return m_Connections;
		};
		SHADE_INLINE EndpointsPrototype<NodeIdentifier>& GetEndpoints()
		{
			return m_Endpoints;
		};
		SHADE_INLINE void SetConnections(ConnectionsPrototype<NodeIdentifier, EndpointIdentifier> connections)
		{
			m_Connections = std::move(connections);
		};
		SHADE_INLINE void SetEndpoints(EndpointsPrototype<NodeIdentifier> endpoints)
		{
			m_Endpoints = std::move(endpoints);
		};
		SHADE_INLINE NodeIdentifier GetNodeIdentifier() const
		{
			return m_Identifier;
		};

		VisualStyle													Style;
	private:
		NodeIdentifier												m_Identifier;
		Node														m_NodeValue;

		ConnectionsPrototype<NodeIdentifier, EndpointIdentifier>	m_Connections;
		EndpointsPrototype<NodeIdentifier>							m_Endpoints;

		ImVec2														m_ScreenPosition = ImVec2{ 0, 0 };
	};

	template<typename Graph, typename NodeIdentifier, typename EndpointIdentifier, typename Node>
	class ImGuiGraphPrototype
	{
	public:
		struct VisualStyle
		{
			ImVec4 BackgroundColor = ImVec4{ 0.1f, 0.1f, 0.1f, 1.f };
			ImVec4 GridColorSmall = ImVec4{ 0.2f, 0.2f, 0.2f, 1.f };
			ImVec4 GridColorBig = ImVec4{ 0.4, 0.4, 0.4, 1.f };
			float  GridSize = 64.f;

			float Rounding = 7.f;
			float NodeBorderWidth = 4.f;
			float HeaderHeight = 35.f;
			float EndpointRadius = 10.f;
			float ConnectionThickness = 5.f;

			ImVec2 Padding = ImVec2{ 10.f, 5.f };
			ImVec2 CellPadding = ImVec2{ 15.f, 10.f };
		};

		struct VisualScale
		{
			float TargetFactor = 1.f;
			float Factor = 1.f;
			float MinFactor = 0.01f;
			float MaxFactor = 2.f;
			float LerpFactor = 0.2f;
			float RatioFactor = 0.1f;
		};

		struct InternalContext
		{
			VisualScale Scale;
			ImRect		CanvasRect;
			ImVec2		CanvasSize = ImVec2(0.f, 0.f);
			ImVec2		CanvasPosition = ImVec2(0.f, 0.f);
			ImVec2		Offset = ImVec2(0.f, 0.f);
			ImDrawList* DrawList = nullptr;
		};

	public:
		ImGuiGraphPrototype(Graph graph) : m_GraphValue(graph) {}
		virtual ~ImGuiGraphPrototype()
		{
			for (auto& [key, value] : m_Nodes) { delete value; }
		}
		// Copy constructor
		ImGuiGraphPrototype(const ImGuiGraphPrototype&) = delete;
		// Move constructor
		ImGuiGraphPrototype(ImGuiGraphPrototype&&) = delete;
		// Copy assignment operator
		ImGuiGraphPrototype& operator=(const ImGuiGraphPrototype&) = delete;
		// Move assignment operator
		ImGuiGraphPrototype& operator=(ImGuiGraphPrototype&&) = delete;

	public:
		template<typename NodeDeligate, typename... Args>
		SHADE_INLINE void EmplaceNode(NodeIdentifier identifier, Node node)
		{
			m_Nodes.emplace(identifier, new NodeDeligate(identifier, node));
		}
		SHADE_INLINE std::size_t GetCount() const
		{
			return m_Nodes.size();
		}
		SHADE_INLINE const	Graph& GetGraph() const { return m_GraphValue; }
		SHADE_INLINE		Graph& GetGraph() { return m_GraphValue; }
	public:
		bool Show(const char* title, const ImVec2& size)
		{
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
						if (m_ActiveNode)
						{
							m_ActiveNode->ProcessSideBar();
						}
						else
						{
							ProcessSideBar();
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
								m_ActiveNode = nullptr;
								m_ConnectionEstablish.Reset();
							}

							DrawNodes();
							DrawAllConnections();
						}

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

			// Reset style back
			ImGui::GetStyle() = unscaledStyle;

			return false;
		}
	private:
		void DrawNodes()
		{
			std::size_t CurrentChannel = m_Nodes.size() - 1;

			m_Context.DrawList->ChannelsSplit((m_Nodes.size()) ? m_Nodes.size() + 1 : 1);

			for (auto& [id, node] : m_Nodes)
			{
				m_Context.DrawList->ChannelsSetCurrent(CurrentChannel);

				node->SetConnections(node->ReceiveConnections());
				node->SetEndpoints(node->ReceiveEndpoints());

				ImGui::PushID(id);

				ImGui::SetCursorScreenPos(m_Context.Offset + node->GetScreenPosition() * m_Context.Scale.Factor);

				DrawNode((m_ActiveNode == node), CurrentChannel, node);

				ImGui::SetCursorScreenPos(m_Context.Offset + node->GetScreenPosition() * m_Context.Scale.Factor);

				ImGui::InvisibleButton(std::format("##NodeBox_{}", std::size_t(node)).c_str(), node->Style.Size * m_Context.Scale.Factor);

				if (ImGui::IsItemActive()) { m_ActiveNode = node; }

				if (!(m_ConnectionEstablish.IsOutPutSelect))
					MoveNode(m_Context.Scale.Factor, node->GetScreenPosition());

				ImGui::PopID();
				CurrentChannel--;

			}

			m_Context.DrawList->ChannelsMerge();
		}
		void DrawNode(bool isActive, std::size_t nodeIndex, GraphNodePrototype<NodeIdentifier, EndpointIdentifier, Node>* node)
		{
			const ImVec2 nodeRectMin = m_Context.Offset + node->GetScreenPosition() * m_Context.Scale.Factor;
			const ImVec2 nodeRectMax = nodeRectMin + node->Style.Size * m_Context.Scale.Factor;

			const ImVec4 bckColor = (isActive) ? ImVec4(
				node->Style.BackgroundColor.x * 1.5f,
				node->Style.BackgroundColor.y * 1.5f,
				node->Style.BackgroundColor.z * 1.5f,
				node->Style.BackgroundColor.w) : node->Style.BackgroundColor;

			m_Context.DrawList->AddRectFilled(nodeRectMin, nodeRectMax,
				ImGui::ColorConvertFloat4ToU32(bckColor), m_VisualStyle.Rounding);

			DrawHeader(node->Style.Title.c_str(), node->GetScreenPosition(), node->Style.Size, node->Style.HeaderColor, node->Style.HeaderTextColor);
			DrawBorder(isActive, node->GetScreenPosition(), node->Style.Size, node->Style.BorderColor);

			const ImVec2 endOfPrevRegion = DrawEndpoints(m_VisualStyle.HeaderHeight, nodeIndex, node);

			const ImVec2 newScreenPosition = m_Context.Offset + ImVec2{ node->GetScreenPosition().x * m_Context.Scale.Factor, endOfPrevRegion.y };
			const ImVec2 avalibleRegion{ nodeRectMax - newScreenPosition };

			ImGui::SetCursorScreenPos(newScreenPosition);

			ImGui::BeginGroup();
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, m_VisualStyle.Padding * m_Context.Scale.Factor);
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, m_VisualStyle.CellPadding * m_Context.Scale.Factor);
				ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4{ 0.f, 0.f, 0.f, 0.f });

				if (ImGui::BeginTableEx("##BodyContentTable", std::size_t(node) + 2000, 1, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuterV, { avalibleRegion.x, 0 }))
				{
					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn();
						{
							node->ProcessBodyContent();
						}
					}
					ImGui::EndTable();
				}
				ImGui::PopStyleVar(2);
				ImGui::PopStyleColor();
			}
			ImGui::EndGroup();

			ImGui::Dummy({ 0, 0 });

			node->Style.Size.y += ImGui::GetCursorScreenPos().y - nodeRectMax.y;
		}
		void DrawAllConnections()
		{
			for (auto& [id, node] : m_Nodes)
				DrawConnections(node);
		}
		void DrawConnections(GraphNodePrototype<NodeIdentifier, EndpointIdentifier, Node>* node)
		{
			auto& endpoints   = node->GetEndpoints();
			auto& connections = node->GetConnections();

			for (const auto& connection : connections)
			{
				const auto& inputEndpoint = endpoints[EndpointPrototype::EndpointType::Input].at(connection.InputEndpointIdentifier);
				// It should be there !!
				const auto& outputNode = m_Nodes.at(connection.OutputNodeIdentifier);

				const auto& outputEndpoint = outputNode->GetEndpoints()[EndpointPrototype::EndpointType::Output].at(connection.OutputEndpointIdentifier);

				ImGuiGraphNodeRender::DrawConnection(
					m_Context.DrawList,
					m_Context.Offset,
					m_Context.Scale.Factor,
					inputEndpoint.GetScreenPosition(),
					outputEndpoint.GetScreenPosition(),
					inputEndpoint.Style.Color,
					m_VisualStyle.ConnectionThickness);
			}

			if (m_ConnectionEstablish.IsOutPutSelect && m_ConnectionEstablish.Connection.OutputNodeIdentifier == node->GetNodeIdentifier())
			{
				const auto& endpoint = endpoints[EndpointPrototype::EndpointType::Output].at(m_ConnectionEstablish.Connection.OutputEndpointIdentifier);

				ImGuiGraphNodeRender::DrawConnection(
					m_Context.DrawList,
					m_Context.Offset,
					m_Context.Scale.Factor,
					ImGui::GetMousePos() - m_Context.Offset,
					endpoint.GetScreenPosition(),
					endpoint.Style.Color,
					m_VisualStyle.ConnectionThickness);
			}

		}
		ImVec2 DrawEndpoints(float yOffset, std::size_t nodeIndex, GraphNodePrototype<NodeIdentifier, EndpointIdentifier, Node>* node)
		{
			auto& endpoints = node->GetEndpoints();
			auto& connections = node->GetConnections();

			const std::size_t outputCount = endpoints[EndpointPrototype::EndpointType::Output].size();
			const ImVec2 cellPaddingZoomed = m_VisualStyle.CellPadding * m_Context.Scale.Factor;

			ImVec2 endOfRegion = ImVec2{ 0.f, 0.f };

			ImGui::SetCursorScreenPos(m_Context.Offset + ImVec2{ node->GetScreenPosition().x, node->GetScreenPosition().y + yOffset } * m_Context.Scale.Factor);
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPaddingZoomed);
			ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4{ 0.f, 0.f, 0.f, 0.f });

			//ImGuiTableFlags_SizingStretchProp|
			if (ImGui::BeginTableEx("##EndpointsTable", std::size_t(node) + 1000, (outputCount) ? 2 : 1,
				ImGuiTableFlags_BordersOuterV |
				ImGuiTableFlags_SizingStretchProp,
				{ node->Style.Size.x * m_Context.Scale.Factor, 0 }))
			{
				auto inputsIt = endpoints[EndpointPrototype::EndpointType::Input].begin();
				auto outputIt = endpoints[EndpointPrototype::EndpointType::Output].begin();

				while (inputsIt != endpoints[EndpointPrototype::EndpointType::Input].end() ||
					outputIt != endpoints[EndpointPrototype::EndpointType::Output].end())
				{
					ImGui::TableNextRow();
					{
						if (inputsIt != endpoints[EndpointPrototype::EndpointType::Input].end())
						{
							ImGui::TableNextColumn();

							const ImVec2 deltaPosition = ImGui::GetCursorScreenPos() - m_Context.Offset;
							{
								node->ProcessEndpoint(inputsIt->first, inputsIt->second);
							}

							inputsIt->second.SetScreenPosition(ImVec2{ deltaPosition.x - cellPaddingZoomed.x, deltaPosition.y + cellPaddingZoomed.y });

							endOfRegion.y = (inputsIt->second.GetScreenPosition().y > endOfRegion.y) ? inputsIt->second.GetScreenPosition().y : endOfRegion.y;

							inputsIt++;
						}
						else
						{
							ImGui::TableNextColumn();
							ImGui::Dummy(ImVec2{ ImGui::GetContentRegionAvail().x, 0 });
						}
						if (outputIt != endpoints[EndpointPrototype::EndpointType::Output].end())
						{
							ImGui::TableNextColumn();
							const ImVec2 deltaPosition = ImGui::GetCursorScreenPos() - m_Context.Offset;
							{
								node->ProcessEndpoint(outputIt->first, outputIt->second);
							}
							outputIt->second.SetScreenPosition(ImVec2{ deltaPosition.x + ImGui::GetContentRegionAvail().x + cellPaddingZoomed.x, deltaPosition.y + cellPaddingZoomed.y });

							endOfRegion.y = (outputIt->second.GetScreenPosition().y > endOfRegion.y) ? outputIt->second.GetScreenPosition().y : endOfRegion.y;

							outputIt++;
						}
						else
						{
							ImGui::TableNextColumn();
							//ImGui::Dummy(ImVec2{ ImGui::GetContentRegionAvail().x, 0 });
						}
					}
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			for (auto& [id, endpoint] : endpoints[EndpointPrototype::EndpointType::Input])
			{
				if (ImGuiGraphNodeRender::DrawEndpoint(m_Context.DrawList,
					m_Context.Offset,
					m_VisualStyle.EndpointRadius,
					m_Context.Scale.Factor,
					endpoint.GetScreenPosition(),
					endpoint.Style.Color,
					endpoint.Style.ColorHovered))
				{
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_ConnectionEstablish.IsOutPutSelect)
					{
						m_ConnectionEstablish.IsInputSelect = true;
						m_ConnectionEstablish.Connection.InputNodeIdentifier = node->GetNodeIdentifier();
						m_ConnectionEstablish.Connection.InputEndpointIdentifier = id;

						this->Connect(m_ConnectionEstablish.Connection);
						m_ConnectionEstablish.Reset();
					}
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !m_ConnectionEstablish.IsOutPutSelect)
					{
						m_ConnectionEstablish.Connection.InputNodeIdentifier = node->GetNodeIdentifier();
						m_ConnectionEstablish.Connection.InputEndpointIdentifier = id;

						this->Disconnect(m_ConnectionEstablish.Connection);
						m_ConnectionEstablish.Reset();
					}
				}
			}
			for (auto& [id, endpoint] : endpoints[EndpointPrototype::EndpointType::Output])
			{
				if (ImGuiGraphNodeRender::DrawEndpoint(m_Context.DrawList,
					m_Context.Offset,
					m_VisualStyle.EndpointRadius,
					m_Context.Scale.Factor,
					endpoint.GetScreenPosition(),
					endpoint.Style.Color,
					endpoint.Style.ColorHovered))
				{
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						m_ConnectionEstablish.IsOutPutSelect = true;
						m_ConnectionEstablish.Connection.OutputNodeIdentifier = node->GetNodeIdentifier();
						m_ConnectionEstablish.Connection.OutputEndpointIdentifier = id;
					}
				}
			}

			ImGui::Dummy({ 0, 0 });
			return ImGui::GetCursorScreenPos() - m_Context.Offset;
		}
		SHADE_INLINE ImVec2 CalculateMouseWorldPos(const ImVec2& mousePosition)
		{
			return (mousePosition - ImGui::GetCursorScreenPos()) / m_Context.Scale.Factor;
		}
		void ProcessScale() // private
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

		SHADE_INLINE void MoveNode(float scaleFactor, ImVec2& position) // Move to cpp
		{
			if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				ImVec2 delta = ImGui::GetIO().MouseDelta / scaleFactor;

				if (fabsf(delta.x) >= 0.1f || fabsf(delta.y) >= 0.1f)
				{
					position.x += delta.x;
					position.y += delta.y;
				}
			}
		}

		float DrawHeader(const char* title,
			const ImVec2& nodePosition,
			const ImVec2& nodeSize,
			const ImVec4& headerColor,
			const ImVec4& headerTextColor)
		{
			const ImVec2 scaledPosition = nodePosition * m_Context.Scale.Factor;
			const ImVec2 pMin = (m_Context.Offset + scaledPosition);
			const ImVec2 pMax = (m_Context.Offset + scaledPosition + (ImVec2{ nodeSize.x, m_VisualStyle.HeaderHeight } *m_Context.Scale.Factor));

			m_Context.DrawList->AddRectFilled(pMin, pMax, ImGui::ColorConvertFloat4ToU32(headerColor), m_VisualStyle.Rounding, ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersTopLeft);

			ImGui::SetCursorScreenPos(m_Context.Offset + (m_VisualStyle.Padding + nodePosition) * m_Context.Scale.Factor);
			ImGui::PushStyleColor(ImGuiCol_Text, headerTextColor);

			ImGui::SetWindowFontScale(ImGui::GetCurrentWindow()->FontWindowScale * 1.5f);

			ImGui::BeginGroup();
			{
				ImGui::Text(title);
			}
			ImGui::EndGroup();

			ImGui::SetWindowFontScale(ImGui::GetCurrentWindow()->FontWindowScale / 1.5f);

			ImGui::PopStyleColor(1);

			return nodePosition.y + m_VisualStyle.HeaderHeight;
		}
		SHADE_INLINE void DrawBorder(bool isActive, const ImVec2& nodePosition, const ImVec2& nodeSize, const ImVec4& borderColor)
		{
			const ImVec2 scaledPosition = nodePosition * m_Context.Scale.Factor;
			const ImVec2 pMin = (m_Context.Offset + scaledPosition);
			const ImVec2 pMax = (m_Context.Offset + scaledPosition + nodeSize * m_Context.Scale.Factor);

			const ImVec4 color = (isActive) ? ImVec4(
				borderColor.x * 1.6f,
				borderColor.y * 1.6f,
				borderColor.z * 1.6f,
				borderColor.w) : borderColor;

			m_Context.DrawList->AddRect(pMin, pMax, ImGui::ColorConvertFloat4ToU32(color), m_VisualStyle.Rounding, 0, m_VisualStyle.NodeBorderWidth);
		}
	private:
		Graph			m_GraphValue;
		InternalContext m_Context;
	protected:
		VisualStyle		m_VisualStyle;
	protected:
		virtual bool Connect(const ConnectionPrototype<NodeIdentifier, EndpointIdentifier>& connection) = 0;
		virtual bool Disconnect(const ConnectionPrototype<NodeIdentifier, EndpointIdentifier>& connection) = 0;

		virtual void PopupMenu() {};
		virtual void ProcessSideBar() {};
	private:

		std::map<NodeIdentifier, GraphNodePrototype<NodeIdentifier, EndpointIdentifier, Node>*> m_Nodes;
		GraphNodePrototype<NodeIdentifier, EndpointIdentifier, Node>* m_ActiveNode = nullptr;

		struct ConnectionEstablish
		{
			bool IsInputSelect = false;
			bool IsOutPutSelect = false;
			ConnectionPrototype<NodeIdentifier, EndpointIdentifier> Connection;

			void Reset() { IsInputSelect = false; IsOutPutSelect = false; }
		};

		ConnectionEstablish m_ConnectionEstablish;

	};

}