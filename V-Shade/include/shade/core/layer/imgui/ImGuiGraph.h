#pragma once
#include <shade/config/ShadeAPI.h>
#include <ImGui/imgui.h>

#define IMGUI_DEFINE_MATH_OPERATORS

#include <ImGui/imgui_internal.h>

namespace shade
{

	struct GraphViewContext
	{
		struct
		{
			float Rounding = 7.f;
			float NodeBorderWidth = 4.f;
			float HeaderHeight = 35.f;
			float EndpointRadius = 10.f;
			float ConnectionThickness = 5.f;
			ImVec4 NodeBorderColor = ImVec4{ 0.3f, 0.3f, 0.3f, 1.f };
			ImVec4 NodeBackgroundColor = ImVec4{ 0.2f, 0.2f, 0.2f, 1.f };
			ImVec4 HeaderTextColor = ImVec4{ 0.0f, 0.0f, 0.0f, 1.f };
			ImVec4 ConnectionColor = ImVec4{ 0.5, 0.7, 0.0, 1.0 };
			ImVec2 Padding = ImVec2{ 10.f, 5.f };
			ImVec2 CellPadding = ImVec2{ 15.f, 10.f };
		} Style;
		struct
		{
			float ZoomTarget = 1.f;
			float Zoom = 1.f;
			float MinZoom = 0.01f;
			float MaxZoom = 2.f;
			float ZoomLerp = 0.20f;
			float ZoomRatio = 0.1f;
		} Zoom;

		ImVec2 ViewPosition = ImVec2(0.f, 0.f);
	};


	struct SHADE_API ImGuiGraphNodeRender
	{
		static void Zoom(ImRect region, GraphViewContext& context);
		static void UpdateZoom(GraphViewContext& context, const ImGuiIO& io);
		static void UpdateView(GraphViewContext& context, const ImGuiIO& io);
		static ImVec2 CalculateMouseWorldPos(const ImGuiIO& io, const GraphViewContext& context);
		static void Grid(ImDrawList* drawList, ImVec2 windowPos, const GraphViewContext& context, const ImVec2& canvasSize, ImU32 gridColor, ImU32 gridColor2, float gridSize);
		static void DrawGridLines(const ImVec2& start, const ImVec2& canvasSize, const float gridSpace, const ImVec2& windowPos, const ImColor& gridColor, const ImColor& gridColor2, ImDrawList* drawList, int divx, int divy);	
		static void MoveNode(const ImGuiIO& io, const GraphViewContext& context, ImVec2& position);
		static void DrawBorder(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, bool isActive, const ImVec2& nodePosition, const ImVec2& nodeSize);
		static float DrawHeader(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, const char* title, const ImVec2& nodePosition, const ImVec2& nodeSize, const ImVec4& headerColor);
		static bool DrawInputEndpoint(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, const ImVec2& screenPosition, const ImVec4& color, const ImVec4& hoveredColor);
		static bool DrawOutputEndpoint(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, const ImVec2& screenPosition, const ImVec4& color, const ImVec4& hoveredColor);
		static void DrawConnection(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, const ImVec2& from, const ImVec2& till);
	};

	struct Endpoint
	{
		enum Type : std::uint32_t { Input, Output, MAX_ENUM };
		////////////////////////////
		std::string		Name;
		ImVec2			ScreenPosition;
	};

	template<typename EndpointIdentifier>
	using EndpointsDeligate = std::array<std::map<EndpointIdentifier, Endpoint>, Endpoint::MAX_ENUM>;
	
	template<typename NodeIdentifier, typename EndpointIdentifier>
	struct Connection
	{
		//////////////////////////////////////////////////
		NodeIdentifier			SourceNodeIdentifier;
		EndpointIdentifier		SourceEndpointIdentifier;
		NodeIdentifier			TargetNodeIdentifier;
		EndpointIdentifier		TargetEndpointIdentifier;
	};

	template<typename NodeIdentifier, typename EndpointIdentifier>
	using ConnectionDeligate = std::vector<Connection<NodeIdentifier, EndpointIdentifier>>;
	
	template<typename Graph, typename NodeIdentifier, typename EndpointIdentifier, typename Node>
	class ImGuiGraphDeligate;

	template<typename NodeIdentifier, typename EndpointIdentifier, typename Node>
	struct GraphNodeDeligate
	{
		GraphNodeDeligate(NodeIdentifier id, Node ptr) : m_Identifier(id), m_NodeValue(ptr) {}
		~GraphNodeDeligate() = default;
		//////////////////////////////////////////////////////////////////////////
		SHADE_INLINE const	Node& GetNode() const	{ return m_NodeValue; }
		SHADE_INLINE		Node& GetNode()			{ return m_NodeValue; }
		//////////////////////////////////////////////////////////////////////////
		std::string				Title				= "Node";
		ImVec2					Size				= ImVec2{ 200, 200 };
		ImVec2					ScreenPosition		= ImVec2{ 0, 0 };
		struct
		{
			ImVec4 HeaderColor						= ImVec4{ 0.5, 0.7, 0.0, 1.0 };

			ImVec4 InputEndpointsColor				= ImVec4{ 0.5, 0.7, 0.0, 1.0 };
			ImVec4 InputEndpointsColorHovered		= ImVec4{ InputEndpointsColor.x * 2.f,InputEndpointsColor.y * 2.f, InputEndpointsColor.z * 2.f, InputEndpointsColor.w };

			ImVec4 OutPutEndpointsColor				= ImVec4{ 0.5, 0.7, 0.0, 1.0 };
			ImVec4 OutPutEndpointsColorHovered		= ImVec4{ OutPutEndpointsColor.x * 2.f,OutPutEndpointsColor.y * 2.f, OutPutEndpointsColor.z * 2.f, OutPutEndpointsColor.w };

		} Style;

		virtual ConnectionDeligate<NodeIdentifier, EndpointIdentifier> MergeConnections() const = 0;
		virtual EndpointsDeligate<NodeIdentifier> MergeEndpoints() const = 0;

		virtual void ProcessBodyConent() {};
		virtual void ProcessEndpoint(const EndpointIdentifier& endpoint, Endpoint::Type type) {};

		SHADE_INLINE ConnectionDeligate<NodeIdentifier, EndpointIdentifier>& GetConnections() 
		{
			return m_Connections;  
		};
		SHADE_INLINE EndpointsDeligate<NodeIdentifier>& GetEndpoints() 
		{ 
			return m_Endpoints; 
		};
		SHADE_INLINE void SetConnections(ConnectionDeligate<NodeIdentifier, EndpointIdentifier> connections) 
		{ 
			m_Connections = std::move(connections); 
		};
		SHADE_INLINE void SetEndpoints(EndpointsDeligate<NodeIdentifier> endpoints) 
		{ 
			m_Endpoints = std::move(endpoints);
		};
		SHADE_INLINE NodeIdentifier GetNodeIdentifier() const
		{
			return m_Identifier;
		};
	private:
		NodeIdentifier												m_Identifier;
		Node														m_NodeValue;
		ConnectionDeligate<NodeIdentifier, EndpointIdentifier>		m_Connections;
		EndpointsDeligate<NodeIdentifier>							m_Endpoints;
	};

	template<typename Graph, typename NodeIdentifier, typename EndpointIdentifier, typename Node>
	class ImGuiGraphDeligate
	{

		Graph m_Graph;

	public:
		ImGuiGraphDeligate(Graph graph) : m_Graph(graph) {}
		~ImGuiGraphDeligate() { for (auto& [key, value] : m_Nodes) { delete value; } }
		template<typename DeligateType, typename... Args>
		void EmpalceNode(NodeIdentifier identifier, Node node)
		{
			m_Nodes.emplace(identifier, new DeligateType(identifier, node));
		}
		std::size_t Count() const { return m_Nodes.size(); }
		inline bool Show(const char* title, const ImVec2& size)
		{
			// Need to remove it from here
			static GraphViewContext	context;

			ImGuiStyle unscaledStyle = ImGui::GetStyle();

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{ 0.2, 0.2, 0.2, 1 });

			ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
			{
				if (ImGui::Begin(title))
				{
					const ImVec2 windowPos = ImGui::GetCursorScreenPos(), canvasSize = ImGui::GetContentRegionAvail(), scrollRegionLocalPos(0, 0);

					ImRect canvas(windowPos, windowPos + canvasSize);

					ImVec2 offset = ImGui::GetCursorScreenPos() + (context.ViewPosition * context.Zoom.Zoom);


					if (ImGui::BeginChild("#ImGuiGraph::ScrollRegion", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse))
					{
						
						ImGui::GetStyle().ScaleAllSizes(context.Zoom.Zoom);
						ImGui::SetWindowFontScale(context.Zoom.Zoom);

						ImDrawList* drawList = ImGui::GetWindowDrawList();

						ImGuiGraphNodeRender::Zoom(canvas, context);
						ImGuiGraphNodeRender::Grid(drawList, windowPos, context, canvasSize, ImGui::ColorConvertFloat4ToU32(ImVec4{ 0.3, 0.3, 0.3, 1.0 }), ImGui::ColorConvertFloat4ToU32(ImVec4{ 0.5, 0.5, 0.5, 1.0 }), 64.f);

						{
							if (ImGui::IsItemActive())
							{
								m_ActiveNodeIdentifier = 0u;
								m_NewConnection.Status = 0;
							}

							DrawNodes(drawList, offset, context, m_Nodes);
							DrawAllConnections(drawList, offset, context, m_Nodes);
						}

						ImGui::EndChild();
					}

				}ImGui::End();
			}
			ImGui::PopStyleColor();

			ImGui::GetStyle() = unscaledStyle;

			return false;
		}

		virtual bool Connect(const Connection<NodeIdentifier, EndpointIdentifier>& coonection) = 0;
	private:

		std::map<NodeIdentifier, GraphNodeDeligate<NodeIdentifier, EndpointIdentifier, Node>*> m_Nodes;
		NodeIdentifier m_ActiveNodeIdentifier = 0u; // but this is wrong !!

	public:
		struct NewConnection
		{
			enum _Status : int // uint ?
			{
				Empty = (1 << 0),
				OutputSelect = (1 << 1),
				InputSelect = (1 << 2),
			};

			int Status = 0;
			Connection<NodeIdentifier, EndpointIdentifier> Connection;
		};

		NewConnection m_NewConnection;

		void DrawNodes(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, std::map<NodeIdentifier, GraphNodeDeligate<NodeIdentifier, EndpointIdentifier, Node>*>& nodes)
		{
			std::size_t CurrentChannel = 0;

			drawList->ChannelsSplit((nodes.size()) ? nodes.size() + 1: 1);

			for (auto& [id, node] : nodes)
			{
				drawList->ChannelsSetCurrent(CurrentChannel);

				node->SetConnections(node->MergeConnections());
				node->SetEndpoints(node->MergeEndpoints());

				ImGui::PushID(id);

				ImGui::SetCursorScreenPos(offset + node->ScreenPosition * context.Zoom.Zoom);

				DrawNode(drawList, offset, context, (m_ActiveNodeIdentifier == id), CurrentChannel, node);

				ImGui::SetCursorScreenPos(offset + node->ScreenPosition * context.Zoom.Zoom);
				ImGui::InvisibleButton("NodeBox", node->Size * context.Zoom.Zoom);

				if (ImGui::IsItemActive()) { m_ActiveNodeIdentifier = id; }

				if(!(m_NewConnection.Status & NewConnection::_Status::OutputSelect))
					ImGuiGraphNodeRender::MoveNode(ImGui::GetIO(), context, node->ScreenPosition);

				ImGui::PopID();
				CurrentChannel++;
				
			}

			drawList->ChannelsMerge();
		}

		inline void DrawNode(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, bool isActive, std::size_t nodeIndex, GraphNodeDeligate<NodeIdentifier, EndpointIdentifier, Node>* node)
		{
			const ImVec2 nodeRectMin = offset + node->ScreenPosition * context.Zoom.Zoom;
			const ImVec2 nodeRectMax = nodeRectMin + node->Size * context.Zoom.Zoom;

			ImVec4 bckColor = (isActive) ? ImVec4(
				context.Style.NodeBackgroundColor.x * 1.5f,
				context.Style.NodeBackgroundColor.y * 1.5f,
				context.Style.NodeBackgroundColor.z * 1.5f,
				context.Style.NodeBackgroundColor.w) : context.Style.NodeBackgroundColor;

			drawList->AddRectFilled(nodeRectMin, nodeRectMax,
				ImGui::ColorConvertFloat4ToU32(bckColor), context.Style.Rounding);

			ImGuiGraphNodeRender::DrawHeader(drawList, offset, context, node->Title.c_str(), node->ScreenPosition, node->Size, node->Style.HeaderColor);
			ImGuiGraphNodeRender::DrawBorder(drawList, offset, context, isActive, node->ScreenPosition, node->Size);

			const ImVec2 endOfPrevRegion = DrawEndpoints(drawList, offset, context, context.Style.HeaderHeight, nodeIndex, node);

			const ImVec2 newScreenPosition = offset + ImVec2{ node->ScreenPosition.x * context.Zoom.Zoom, endOfPrevRegion.y };
			const ImVec2 avalibleRegion { nodeRectMax - newScreenPosition };

			ImGui::SetCursorScreenPos(newScreenPosition);

			ImGui::BeginGroup();
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, context.Style.Padding * context.Zoom.Zoom);
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,  context.Style.CellPadding * context.Zoom.Zoom);
				
				if (ImGui::BeginTableEx("##BodyContentTable", std::size_t(node) + 2000, 1, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersOuterV, { avalibleRegion.x, 0}))
				{
					ImGui::TableNextRow();
					{
						ImGui::TableNextColumn(); 
						{ 
							node->ProcessBodyConent(); 
						}
					}
					ImGui::EndTable();
				}
				ImGui::PopStyleVar(2);
			}
			ImGui::EndGroup();

			ImGui::Dummy({ 0, 0 });

			node->Size.y += ImGui::GetCursorScreenPos().y - nodeRectMax.y;
		}
		inline void DrawAllConnections(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, std::map<NodeIdentifier, GraphNodeDeligate<NodeIdentifier, EndpointIdentifier, Node>*>& nodes)
		{
			for (auto& [id, node] : nodes)
			{
				DrawConnections(drawList, offset, context, node);
			}
		}
		inline void DrawConnections(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, GraphNodeDeligate<NodeIdentifier, EndpointIdentifier, Node>* node)
		{
			// Need to revork
			auto& endpoints = node->GetEndpoints();
			auto& connections = node->GetConnections();

			for (const auto& conenction : connections)
			{
				if (endpoints[Endpoint::Type::Output].find(conenction.SourceEndpointIdentifier) != endpoints[Endpoint::Type::Output].end())
				{
					const auto sourceEndpoint = endpoints[Endpoint::Type::Output].at(conenction.SourceEndpointIdentifier);

					if (m_NewConnection.Status & NewConnection::_Status::OutputSelect && m_NewConnection.Connection.SourceNodeIdentifier == node->GetNodeIdentifier())
					{
						/*	NodeIdentifier			SourceNodeIdentifier;
							EndpointIdentifier		SourceEndpointIdentifier;
							NodeIdentifier			TargetNodeIdentifier;
							EndpointIdentifier		TargetEndpointIdentifier;*/
						ImGuiGraphNodeRender::DrawConnection(drawList, offset, context, sourceEndpoint.ScreenPosition, ImGui::GetMousePos() - offset);
					}

					if (m_Nodes.find(conenction.TargetNodeIdentifier) != m_Nodes.end())
					{
						const auto targetNode = m_Nodes.at(conenction.TargetNodeIdentifier);

						const auto& targetEndpoints = targetNode->GetEndpoints()[Endpoint::Type::Input];

						if (targetEndpoints.find(conenction.TargetEndpointIdentifier) != targetEndpoints.end())
						{
							const auto targetEndpoint = targetEndpoints.at(conenction.TargetEndpointIdentifier);

							if (m_NewConnection.Status & NewConnection::_Status::InputSelect && m_NewConnection.Connection.TargetNodeIdentifier == conenction.TargetEndpointIdentifier)
							{
								/*	NodeIdentifier			SourceNodeIdentifier;
									EndpointIdentifier		SourceEndpointIdentifier;
									NodeIdentifier			TargetNodeIdentifier;
									EndpointIdentifier		TargetEndpointIdentifier;*/
								ImGuiGraphNodeRender::DrawConnection(drawList, offset, context, sourceEndpoint.ScreenPosition, targetEndpoint.ScreenPosition);
							}

							ImGuiGraphNodeRender::DrawConnection(drawList, offset, context, sourceEndpoint.ScreenPosition, targetEndpoint.ScreenPosition);
						}
					}
				}
			}
		}
		inline ImVec2 DrawEndpoints(ImDrawList* drawList, const ImVec2& offset, const GraphViewContext& context, float yOffset, std::size_t nodeIndex, GraphNodeDeligate<NodeIdentifier, EndpointIdentifier, Node>* node)
		{
			auto& endpoints = node->GetEndpoints();
			auto& connections = node->GetConnections();

			const std::size_t outputCount		= endpoints[Endpoint::Type::Output].size();
			const ImVec2 cellPaddingZoomed		= context.Style.CellPadding * context.Zoom.Zoom;

			ImVec2 endOfRegion					= ImVec2{ 0.f, 0.f };

			ImGui::SetCursorScreenPos(offset + ImVec2{ node->ScreenPosition.x, node->ScreenPosition.y + yOffset } * context.Zoom.Zoom);
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPaddingZoomed);
			ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, ImVec4{0.f, 0.f, 0.f, 0.f});

			//ImGuiTableFlags_SizingStretchProp|
			if (ImGui::BeginTableEx("##EndpointsTable", std::size_t(node) + 1000, (outputCount) ? 2 : 1,
				ImGuiTableFlags_BordersOuterV |
				ImGuiTableFlags_SizingStretchProp,
				{ node->Size.x * context.Zoom.Zoom, 0}))
			{
				auto inputsIt = endpoints[Endpoint::Type::Input].begin();
				auto outputIt = endpoints[Endpoint::Type::Output].begin();

			
				while (	inputsIt != endpoints[Endpoint::Type::Input].end() ||
						outputIt != endpoints[Endpoint::Type::Output].end())
				{
					ImGui::TableNextRow();
					{
						if (inputsIt != endpoints[Endpoint::Type::Input].end())
						{
							ImGui::TableNextColumn();

							const ImVec2 deltaPosition = ImGui::GetCursorScreenPos() - offset;
							{
								node->ProcessEndpoint(inputsIt->first, Endpoint::Type::Input);
							}

							inputsIt->second.ScreenPosition = ImVec2{ deltaPosition.x - cellPaddingZoomed.x, deltaPosition.y + cellPaddingZoomed.y };

							endOfRegion.y = (inputsIt->second.ScreenPosition.y > endOfRegion.y) ? inputsIt->second.ScreenPosition.y : endOfRegion.y;

							inputsIt++;
						}
						else
						{
							ImGui::TableNextColumn();
						}
						if (outputIt != endpoints[Endpoint::Type::Output].end())
						{
							
							ImGui::TableNextColumn();

							const ImVec2 deltaPosition = ImGui::GetCursorScreenPos() - offset;
							{
								node->ProcessEndpoint(outputIt->first, Endpoint::Type::Output);
							}
							outputIt->second.ScreenPosition = ImVec2{ deltaPosition.x + ImGui::GetContentRegionAvail().x + cellPaddingZoomed.x, deltaPosition.y};

							endOfRegion.y = (outputIt->second.ScreenPosition.y > endOfRegion.y) ? outputIt->second.ScreenPosition.y : endOfRegion.y;

							outputIt++;
						}
						else
						{
							ImGui::TableNextColumn();
						}
					}		
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			for (auto& [id, endpoint] : endpoints[Endpoint::Type::Input])
			{
				if (ImGuiGraphNodeRender::DrawInputEndpoint(drawList,
					offset,
					context,
					endpoint.ScreenPosition,
					node->Style.InputEndpointsColor,
					node->Style.InputEndpointsColorHovered))
				{
					if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && m_NewConnection.Status & NewConnection::_Status::OutputSelect)
					{
						m_NewConnection.Status									|= NewConnection::_Status::InputSelect;
						m_NewConnection.Connection.TargetNodeIdentifier			= node->GetNodeIdentifier();
						m_NewConnection.Connection.TargetEndpointIdentifier		= id;

						this->Connect(m_NewConnection.Connection);
						/*struct TryTOConnect
						{
							NodeIdentifier from;
							EndpointIdentifier fromEdpoint;
							NodeIdentifier to;
							EndpointIdentifier toEdpoint;
						};*/
					}
				}
			}
			for (auto& [id, endpoint] : endpoints[Endpoint::Type::Output])
			{
				if (ImGuiGraphNodeRender::DrawOutputEndpoint(drawList, offset, context, endpoint.ScreenPosition, node->Style.OutPutEndpointsColor, node->Style.OutPutEndpointsColorHovered))
				{
					if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
					{
						m_NewConnection.Status									|= NewConnection::_Status::OutputSelect;
						m_NewConnection.Connection.SourceNodeIdentifier			= node->GetNodeIdentifier();
						m_NewConnection.Connection.SourceEndpointIdentifier		= id;
						/*struct TryTOConnect
						{
							NodeIdentifier from;
							EndpointIdentifier fromEdpoint;
							NodeIdentifier to;
							EndpointIdentifier toEdpoint;
						};*/
					}
					
				}
			}

			ImGui::Dummy({0, 0});
			return ImGui::GetCursorScreenPos() - offset;
		}
	};

}