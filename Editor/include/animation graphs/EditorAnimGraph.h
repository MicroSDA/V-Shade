#pragma once
#include <shade/core/animation/graphs/AnimationGraph.h>
#include <shade/core/layer/imgui/ImGuiLayer.h>

namespace graph_editor
{
	using namespace shade;
	using namespace animation;
	//using namespace graphs;

	struct GraphVisualStyle
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

	class GraphNodePrototype;

	struct InternalContext
	{
		VisualScale Scale;
		ImRect		CanvasRect;
		ImVec2		CanvasSize = ImVec2(0.f, 0.f);
		ImVec2		CanvasPosition = ImVec2(0.f, 0.f);
		ImVec2		Offset = ImVec2(0.f, 0.f);
		ImDrawList* DrawList = nullptr;
		mutable GraphNodePrototype* CurrentGraph = nullptr;

		struct 
		{
			graphs::BaseNode* InputNode  = nullptr;
			graphs::BaseNode* OutPutNode = nullptr;

			graphs::EndpointIdentifier InputEndpointIdentifier  = ~0;
			graphs::EndpointIdentifier OutPutEndpointIdentifier = ~0;

			bool IsInputSelect = false;
			bool IsOutPutSelect = false;

			ImVec2 InputEndpointScreenPosition  = ImVec2{ 0.f, 0.f };
			ImVec2 OutPutEndpointScreenPosition = ImVec2{ 0.f, 0.f };

			void Reset() { IsInputSelect = false; IsOutPutSelect = false; }
		} mutable ConnectionEstablish;
	};

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

			ImVec4 BorderColor		= ImVec4{ 0.4f, 0.4f, 0.4f, 1.f };
			ImVec4 BorderHovered	= ImVec4{ 0.0f, 0.3f, 0.7f, 1.f };
			ImVec4 BackgroundColor = ImVec4{ 0.15f, 0.15f, 0.15f, 1.f };
			ImVec4 TextColor = ImVec4{ 0.0f, 0.0f, 0.0f, 1.f };

			ImVec2 Size = ImVec2{ 230.f, 200.f };
			std::string	Title = "Node title";
		};

		GraphNodePrototype(graphs::BaseNode* pNode) : m_pNode(pNode) {}
		virtual ~GraphNodePrototype() = default;

		//////////////////////////////////////////////////////////////////////////
		SHADE_INLINE const	graphs::BaseNode* GetNode()	const { return m_pNode; }
		SHADE_INLINE		graphs::BaseNode* GetNode() { return m_pNode; }

		SHADE_INLINE		void SetScreenPosition(const ImVec2& position) { m_pNode->GetScreenPosition() = glm::vec2(position.x, position.y); }
		SHADE_INLINE const	ImVec2 GetScreenPosition()				const { return ImVec2{ m_pNode->GetScreenPosition().x, m_pNode->GetScreenPosition().y }; }
		SHADE_INLINE		ImVec2 GetScreenPosition() { return ImVec2{ m_pNode->GetScreenPosition().x, m_pNode->GetScreenPosition().y }; }

		SHADE_INLINE		bool IsSelected() const { return m_IsSelected; }
		SHADE_INLINE		void SetSelected(bool is) { m_IsSelected = is; }
		//////////////////////////////////////////////////////////////////////////

		ImRect GetScaledRectangle(const InternalContext* context, const GraphVisualStyle* graphStyle);

		virtual void ProcessSideBar() {};
		virtual void ProcessBodyContent() {};
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) { ImGui::Text("Endpoint"); };


		virtual void Draw(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes);
		virtual void DrawHeader(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes);
		virtual void DrawBorder(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes);
		virtual void DrawBody(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes);
		virtual void DrawFooter(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes);
		virtual void DrawEndpoints(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes);
		virtual void DrawConnections(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes);

		void MoveNode(float scaleFactor);
		VisualStyle													Style;
	private:
		graphs::BaseNode* m_pNode;
		bool m_IsSelected = false;
	};

	class GraphPrototype : public GraphNodePrototype
	{
	public:
		GraphPrototype(graphs::BaseNode* pGraph) : GraphNodePrototype(pGraph) {}
		virtual ~GraphPrototype() = default;
		void Intialize();
	public:
		SHADE_INLINE std::vector<GraphNodePrototype*>& GetNodes()	{ return m_Nodes; }
		SHADE_INLINE const	graphs::BaseNode* GetGraph()	const	{ return reinterpret_cast<graphs::BaseNode*>(const_cast<graphs::BaseNode*>(GetNode())); }
		SHADE_INLINE		graphs::BaseNode* GetGraph()			{ return reinterpret_cast<graphs::BaseNode*>(GetNode()); }
	private:
		std::vector<GraphNodePrototype*> m_Nodes;
	};

	struct ConnectionsPrototype
	{
		ImVec2 InputScreenPosition;
		ImVec2 OutputScreenPosition;
		ImVec4 Color;
		////////////
	};

	class GraphEditor
	{
	public:
		GraphEditor() = default;
		~GraphEditor() = default;
		void Initialize(AnimationGraph* pGraph);
	public:
		bool Edit(const char* title, const ImVec2& size);
	private:
		//AnimationGraph*		m_pGraph		= nullptr;
		//graphs::BaseNode*	m_pCurrentGraph	= nullptr;
		graphs::BaseNode*	m_pRootGraph	= nullptr;
		GraphNodePrototype*	m_pSelectedNode = nullptr;

		std::unordered_map<std::size_t, GraphNodePrototype*>  m_Nodes;

		std::vector<GraphNodePrototype*> m_Path;

		InternalContext  m_Context;
		GraphVisualStyle m_VisualStyle;
	
	private:
		void InitializeRecursively(graphs::BaseNode* pNode);
		void ProcessScale();
		ImVec2 CalculateMouseWorldPos(const ImVec2& mousePosition);
		void DrawNodes();
		void DrawConnections();

		void PopupMenu();

		void DrawPathRecursevly(GraphNodePrototype* pNode);

		
	};

	class TransitionNodeDelegate : public GraphNodePrototype
	{
	public:
		TransitionNodeDelegate(graphs::BaseNode* pNode) : GraphNodePrototype(pNode) 
		{
			Style.Title = "Transition";
		}
		virtual ~TransitionNodeDelegate() = default;

	};

	class StateNodeDelegate : public GraphNodePrototype
	{
	public:
		StateNodeDelegate(graphs::BaseNode* pNode) : GraphNodePrototype(pNode) 
		{
			Style.HeaderColor = ImVec4{ 0.1, 0.4, 0.4, 1.0 };
			Style.Size = ImVec2{ 200, 75};
			Style.Title = "State : " + reinterpret_cast<animation::state_machine::StateNode*>(GetNode())->GetName();
		}
		virtual ~StateNodeDelegate() = default;

		virtual void ProcessBodyContent() override
		{
			ImGui::Text(reinterpret_cast<animation::state_machine::StateNode*>(GetNode())->GetName().c_str());
		}
		virtual void Draw(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes) override;
		virtual void DrawBody(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes) override;

		void DrawTransition(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes);
		/*virtual void DrawHeader(const InternalContext* context, const GraphVisualStyle* graphStyle);
		virtual void DrawBorder(const InternalContext* context, const GraphVisualStyle* graphStyle);

		virtual void DrawFooter(const InternalContext* context, const GraphVisualStyle* graphStyle);*/
		ImVec2 GetClosestPointOnRectBorder(const ImRect& rect, const ImVec2& point)
		{
			ImVec2 const points[4] =
			{
				ImLineClosestPoint(rect.GetTL(), rect.GetTR(), point),
				ImLineClosestPoint(rect.GetBL(), rect.GetBR(), point),
				ImLineClosestPoint(rect.GetTL(), rect.GetBL(), point),
				ImLineClosestPoint(rect.GetTR(), rect.GetBR(), point)
			};

			float distancesSq[4] =
			{
				ImLengthSqr(points[0] - point),
				ImLengthSqr(points[1] - point),
				ImLengthSqr(points[2] - point),
				ImLengthSqr(points[3] - point)
			};

			float lowestDistance = FLT_MAX;
			int32_t closestPointIdx = -1;
			for (auto i = 0; i < 4; i++)
			{
				if (distancesSq[i] < lowestDistance)
				{
					closestPointIdx = i;
					lowestDistance = distancesSq[i];
				}
			}

			return points[closestPointIdx];
		}
	};

	class AnimationGraphDeligate : public GraphNodePrototype
	{
	public:
		AnimationGraphDeligate(graphs::BaseNode* pNode) : GraphNodePrototype(pNode)
		{
			Style.Title = "Animation graph";
		}
		virtual ~AnimationGraphDeligate() = default;
		virtual void ProcessSideBar() override;
	};

	class StateMachineNodeDeligate : public GraphNodePrototype
	{
	public:
		StateMachineNodeDeligate(graphs::BaseNode* pNode) : GraphNodePrototype(pNode) 
		{
			Style.HeaderColor = ImVec4{ 0.1, 0.9, 0.7, 1.0 };
			Style.Title = "State Machine";
		}
		virtual ~StateMachineNodeDeligate() = default;

		virtual void ProcessBodyContent() override
		{
			ImGui::Text("State Machine");
		}
		virtual void ProcessSideBar() override;

	};
	class ValueNodeDelegate : public GraphNodePrototype
	{
	public:
		ValueNodeDelegate(graphs::BaseNode* pNode) : GraphNodePrototype(pNode) {}
		virtual ~ValueNodeDelegate() = default;

		virtual void ProcessBodyContent() override
		{
			ImGui::Text("Value");
		}
	};

	class OutputPoseNodeDelegate : public GraphNodePrototype
	{
	public:
		OutputPoseNodeDelegate(graphs::BaseNode* pNode);
		virtual ~OutputPoseNodeDelegate() = default;
	};

	class PoseNodeDelegate : public GraphNodePrototype
	{
	public:
		PoseNodeDelegate(graphs::BaseNode* pNode);
		virtual ~PoseNodeDelegate() = default;

		virtual void ProcessBodyContent() override;
		virtual void ProcessSideBar() override;
	private:
		std::string m_Search;
		bool m_IsAnimationPopupActive = false;
	};

	class OutputTransitionNodeDelegate : public GraphNodePrototype
	{
	public:
		OutputTransitionNodeDelegate(graphs::BaseNode* pNode);
		virtual ~OutputTransitionNodeDelegate() = default;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
	};

	class BlendNode2DNodeDelegate : public GraphNodePrototype
	{
	public:
		BlendNode2DNodeDelegate(graphs::BaseNode* pNode);
		virtual ~BlendNode2DNodeDelegate() = default;
	};

	class IntEqualsNodeDelegate : public GraphNodePrototype
	{
	public:
		IntEqualsNodeDelegate(graphs::BaseNode* pNode);
		virtual ~IntEqualsNodeDelegate() = default;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
	};

	class IntNodeDelegate : public GraphNodePrototype
	{
	public:
		IntNodeDelegate(graphs::BaseNode* pNode);
		virtual ~IntNodeDelegate() = default;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
	};
}


//#pragma once
//#include <shade/core/animation/graphs/AnimationGraph.h>
//#include <shade/core/layer/imgui/ImGuiGraph.h>
//#include <shade/core/layer/imgui/ImGuiLayer.h>
//
//namespace editor_animation_graph
//{
//	using namespace shade;
//	using namespace animation;
//
//	struct BaseNodeDeligate : public GraphNodePrototype<GraphNode::NodeIdentifier,
//		GraphNode::EndpointIdentifier,
//		SharedPointer<GraphNode>>
//	{
//		BaseNodeDeligate(GraphNode::NodeIdentifier id, SharedPointer<GraphNode> node) :
//			GraphNodePrototype<GraphNode::NodeIdentifier,
//			GraphNode::EndpointIdentifier,
//			SharedPointer<GraphNode>>(id, node) {};
//
//		virtual ~BaseNodeDeligate() = default;
//
//		virtual ConnectionsPrototype<GraphNode::NodeIdentifier, GraphNode::EndpointIdentifier>  ReceiveConnections() const override
//		{
//			shade::ConnectionsPrototype<GraphNode::NodeIdentifier, GraphNode::EndpointIdentifier> connections;
//
//			for (const auto& conenction : GetNode()->__GET_CONNECTIONS())
//				connections.emplace_back(conenction.InputNodeIdentifier, conenction.InputEndpoint, conenction.OutputNodeIdentifier, conenction.OutputEndpoint);
//			return connections;
//		}
//
//		virtual EndpointsPrototype<GraphNode::NodeIdentifier> ReceiveEndpoints() const override
//		{
//			EndpointsPrototype<GraphNode::NodeIdentifier> endpoints;
//
//			for (GraphNode::EndpointIdentifier index = 0; index < GetNode()->GetEndpoints()[GraphNode::Connection::Input].GetSize(); ++index)
//				endpoints[EndpointPrototype::Input].emplace(index, EndpointPrototype(EndpointPrototype::Input));
//			for (GraphNode::EndpointIdentifier index = 0; index < GetNode()->GetEndpoints()[GraphNode::Connection::Output].GetSize(); ++index)
//				endpoints[EndpointPrototype::Output].emplace(index, EndpointPrototype(EndpointPrototype::Output));
//
//			return endpoints;
//		}
//
//		virtual void ProcessBodyContent()																			override { ImGui::Text("Body Content"); }
//		virtual void ProcessEndpoint(const GraphNode::EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint) override { ImGui::Text("Endpoint");		}
//	};
//
//	struct OutputPoseNodeDeligate : public BaseNodeDeligate
//	{
//		OutputPoseNodeDeligate(GraphNode::NodeIdentifier id, SharedPointer<GraphNode> node);
//		virtual ~OutputPoseNodeDeligate() = default;
//		virtual void ProcessEndpoint(const GraphNode::EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint) override;
//		virtual void ProcessBodyContent() override;
//	};
//
//	struct PoseNodeDeligate : public BaseNodeDeligate
//	{
//		PoseNodeDeligate(GraphNode::NodeIdentifier id, SharedPointer<GraphNode> node);
//		virtual ~PoseNodeDeligate() = default;
//
//		virtual void ProcessBodyContent() override;
//		virtual void ProcessEndpoint(const GraphNode::EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint) override;
//		virtual void ProcessSideBar() override;
//	private:
//		std::string m_Search;
//		bool m_IsAnimationPopupActive = false;
//	};
//
//	struct BlendNodeDeligate : public BaseNodeDeligate
//	{
//		BlendNodeDeligate(GraphNode::NodeIdentifier id, SharedPointer<GraphNode> node);
//		virtual ~BlendNodeDeligate() = default;
//
//		virtual void ProcessBodyContent() override;
//		virtual void ProcessEndpoint(const GraphNode::EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint) override;
//	};
//	
//	struct BoneMaskNodeDeligate : public BaseNodeDeligate
//	{
//		BoneMaskNodeDeligate(GraphNode::NodeIdentifier id, SharedPointer<GraphNode> node);
//		virtual ~BoneMaskNodeDeligate() = default;
//
//		virtual void ProcessBodyContent() override;
//		virtual void ProcessEndpoint(const GraphNode::EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint) override;
//		virtual void ProcessSideBar() override;
//	private:
//		std::string m_Search;
//	};
//
//	struct ValueNodeDeligate : public BaseNodeDeligate
//	{
//		ValueNodeDeligate(GraphNode::NodeIdentifier id, SharedPointer<GraphNode> node);
//		virtual ~ValueNodeDeligate() = default;
//
//		virtual void ProcessBodyContent() override;
//		virtual void ProcessEndpoint(const GraphNode::EndpointIdentifier& endpointIdentifier, EndpointPrototype& endpoint) override;
//	};
//
//	struct GraphDeligate : public
//		ImGuiGraphPrototype<Asset<AnimationGraph>,
//		GraphNode::NodeIdentifier,
//		GraphNode::EndpointIdentifier,
//		SharedPointer<GraphNode>>
//	{
//		GraphDeligate(Asset<AnimationGraph> graph) : ImGuiGraphPrototype(graph) {}
//		virtual ~GraphDeligate() = default;
//
//		virtual bool Connect(const ConnectionPrototype<GraphNode::NodeIdentifier, GraphNode::EndpointIdentifier>& connection) override
//		{
//			SHADE_CORE_TRACE("OutputNodeIdentifier = {0},OutputEndpointIdentifier = {1},InputNodeIdentifier = {2},InputEndpointIdentifier = {3}",
//				connection.OutputNodeIdentifier, connection.OutputEndpointIdentifier, connection.InputNodeIdentifier, connection.InputEndpointIdentifier)
//
//				return GetGraph()->AddConnection(connection.InputNodeIdentifier, connection.InputEndpointIdentifier, connection.OutputNodeIdentifier, connection.OutputEndpointIdentifier);
//		};
//		virtual bool Disconnect(const ConnectionPrototype<GraphNode::NodeIdentifier, GraphNode::EndpointIdentifier>& coonection) override
//		{
//			return GetGraph()->RemoveConnection(coonection.InputNodeIdentifier, coonection.InputEndpointIdentifier);
//		}
//		virtual void  PopupMenu() override
//		{
//			if (ImGui::MenuItem("Blend"))
//			{
//				auto node = GetGraph()->CreateNode<animation::BlendNode2D>();
//				this->EmplaceNode<BlendNodeDeligate>(node->GetNodeIndex(), node);
//			}
//			if (ImGui::MenuItem("Pose"))
//			{
//				auto node = GetGraph()->CreateNode<animation::PoseNode>();
//				this->EmplaceNode<PoseNodeDeligate>(node->GetNodeIndex(), node);
//			}
//			if (ImGui::MenuItem("Float"))
//			{
//				auto node = GetGraph()->CreateNode<ValueNode>();
//				this->EmplaceNode<ValueNodeDeligate>(node->GetNodeIndex(), node);
//			}
//			if (ImGui::MenuItem("Bone Mask"))
//			{
//				auto node = GetGraph()->CreateNode<animation::BoneMaskNode>();
//				this->EmplaceNode<BoneMaskNodeDeligate>(node->GetNodeIndex(), node);
//			}
//		};
//		virtual void ProcessSideBar() override;
//	private:
//		std::string  m_Search;
//		bool m_IsSkeletonPopupActive = false;
//	};
//
//}
