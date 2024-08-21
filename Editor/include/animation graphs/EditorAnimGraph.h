#pragma once
#include <shade/core/animation/graphs/AnimationGraph.h>
#include <shade/core/layer/imgui/ImGuiLayer.h>

namespace graph_editor
{
	using namespace shade;
	using namespace animation;
	
#define POSE_VALUE_COLOR  { 0.5f, 0.7f, 0.f, 1.f }
#define FLOAT_VALUE_COLOR { 0.28f, 0.6f, 0.83f, 1.f }
#define BOOL_VALUE_COLOR  { 0.9f, 0.2f, 0.2f, 1.f }
#define STRING_VALUE_COLOR  { 0.3f, 0.8f, 0.60f, 1.f }
#define INT_VALUE_COLOR FLOAT_VALUE_COLOR
#define BONEMASK_VALUE_COLOR { 0.6f, 0.4f, 0.f, 1.f }

#define STATE_NODE_COLOR STRING_VALUE_COLOR
#define STATE_MACHINE_NODE_COLOR {0.f, 0.6f, 0.4f, 1.f}
	
	static ImVec4 GetValueColor(shade::NodeValueType type)
	{
		switch (type)
		{
		case shade::NodeValueType::Undefined:
		case shade::NodeValueType::Bool: return BOOL_VALUE_COLOR;
		case shade::NodeValueType::Int8: return INT_VALUE_COLOR;
		case shade::NodeValueType::Int:  return INT_VALUE_COLOR;
		case shade::NodeValueType::Uint8:
		case shade::NodeValueType::Uint:
		case shade::NodeValueType::Float:  return FLOAT_VALUE_COLOR;
		case shade::NodeValueType::Double: return FLOAT_VALUE_COLOR;
		case shade::NodeValueType::String: return STRING_VALUE_COLOR;
		case shade::NodeValueType::Pose: return POSE_VALUE_COLOR;
		case shade::NodeValueType::BoneMask: return BONEMASK_VALUE_COLOR;
		case shade::NodeValueType::Vector2:
		case shade::NodeValueType::Vector3:
		case shade::NodeValueType::Vector4:
		case shade::NodeValueType::Matrix3:
		case shade::NodeValueType::Matrix4:
		case shade::NodeValueType::Quaternion:
		default: return ImVec4{ 1, 1, 1, 1 };
			break;
		}
	}
	
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
		float MaxFactor = 1.8f;
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
		mutable graphs::BaseNode* CurrentNode = nullptr;

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

	class GraphEditor;

	class GraphNodePrototype
	{
	public:
		struct VisualStyle
		{
			ImVec4 HeaderColor		= ImVec4{ 0.7f, 0.7f, 0.7f, 1.f };
			ImVec4 HeaderTextColor	= ImVec4{ 0.0f, 0.0f, 0.f, 1.f };
			ImVec4 BorderColor		= ImVec4{ 0.4f, 0.4f, 0.4f, 1.f };
			ImVec4 BorderHovered	= ImVec4{ 0.0f, 0.3f, 0.7f, 1.f };
			ImVec4 BackgroundColor	= ImVec4{ 0.15f, 0.15f, 0.15f, 1.f };
			ImVec4 TextColor		= ImVec4{ 0.0f, 0.0f, 0.0f, 1.f };

			ImVec2 Size = ImVec2{ 200.f, 150.f };
			std::string	Title = "Node title";
		};

		GraphNodePrototype(graphs::BaseNode* pNode, GraphEditor* pEditor) : m_pNode(pNode), m_pEditor(pEditor) {}
		virtual ~GraphNodePrototype() = default;

		//////////////////////////////////////////////////////////////////////////
		SHADE_INLINE const	graphs::BaseNode* GetNode()	const { return m_pNode; }
		SHADE_INLINE		graphs::BaseNode* GetNode() { return m_pNode; }

		SHADE_INLINE const	GraphEditor* GetEditor() const { return m_pEditor; }
		SHADE_INLINE		GraphEditor* GetEditor() { return m_pEditor; }

		SHADE_INLINE		void SetScreenPosition(const ImVec2& position) { m_pNode->GetScreenPosition() = glm::vec2(position.x, position.y); }
		SHADE_INLINE const	ImVec2 GetScreenPosition()				const { return ImVec2{ m_pNode->GetScreenPosition().x, m_pNode->GetScreenPosition().y }; }
		SHADE_INLINE		ImVec2 GetScreenPosition() { return ImVec2{ m_pNode->GetScreenPosition().x, m_pNode->GetScreenPosition().y }; }

		SHADE_INLINE		bool IsSelected() const { return m_IsSelected; }
		SHADE_INLINE		void SetSelected(bool is) { m_IsSelected = is; }
		//////////////////////////////////////////////////////////////////////////

		ImRect GetScaledRectangle(const InternalContext* context, const GraphVisualStyle* graphStyle);

		virtual void ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes) {};
		virtual void ProcessBodyContent(const InternalContext* context) {};
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) { ImGui::Text("Endpoint"); };
		virtual void ProcessPopup(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, const std::string& search);


		virtual bool Draw(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes);
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
		GraphEditor* m_pEditor;
		bool m_IsSelected = false;
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
		void InitializeRecursively(graphs::BaseNode* pNode, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes); // Strange and not clear
		GraphNodePrototype* GetPrototypedNode(graphs::BaseNode* pNode); // Strange and not clear
		GraphNodePrototype* GetPrototypedReferNode(graphs::BaseNode* pNode); // Strange and not clear

		std::unordered_map<std::size_t, GraphNodePrototype*>& GetNodes() { return m_Nodes; }
		std::unordered_map<std::size_t, GraphNodePrototype*>& GetReferNodes() { return m_ReferNodes; }

		graphs::BaseNode* GetRootGraph() { return m_pRootGraph; }

		void SetToRemove(graphs::BaseNode* pNode);
	public:
		bool Edit(const char* title, const ImVec2& size);
	private:

		template<typename T>
		void CreateNode(shade::graphs::BaseNode* pNode, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
		{
			nodes.insert({ std::size_t(pNode) ^ pNode->GetNodeIdentifier(), new T(pNode, this) });
		}

		graphs::BaseNode*	m_pRootGraph	= nullptr;
		GraphNodePrototype*	m_pSelectedNode = nullptr;

		std::unordered_map<std::size_t, GraphNodePrototype*>  m_Nodes;
		std::unordered_map<std::size_t, GraphNodePrototype*>  m_ReferNodes;

		std::vector<GraphNodePrototype*> m_Path;

		InternalContext  m_Context;
		GraphVisualStyle m_VisualStyle;
	
	private:
		bool RemoveNode(graphs::BaseNode*& pNode);
		void ProcessScale();
		ImVec2 CalculateMouseWorldPos(const ImVec2& mousePosition);
		void DrawNodes();
		void DrawConnections();
		void PopupMenu();

		void DrawPathRecursevly(graphs::BaseNode* pNode);

		graphs::BaseNode* m_pNodeToRemove = nullptr;
	};

	class AnimationGraphDeligate : public GraphNodePrototype
	{
	public:
		AnimationGraphDeligate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
		{
			Style.Title = "Animation graph";
		}
		virtual ~AnimationGraphDeligate() = default;
		virtual void ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes) override;
	private:
		template<typename T>
		graph_editor::GraphNodePrototype* CreateReferNode(const std::string& name)
		{
			auto node = GetNode()->As<AnimationGraph>().CreateInputNode<T>(
				name + std::to_string(GetCurrentTimeStamp<std::chrono::milliseconds>()));
			if (node)
			{
				GetEditor()->InitializeRecursively(node, GetEditor()->GetReferNodes());
				return GetEditor()->GetPrototypedReferNode(node);
			}
			return nullptr;
		}
		bool DeleteReferNode(const std::string& name);
	};

	class TransitionNodeDelegate : public GraphNodePrototype
	{
	public:
		TransitionNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~TransitionNodeDelegate() = default;
	};

	class StateNodeDelegate : public GraphNodePrototype
	{
	public:
		StateNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~StateNodeDelegate() = default;

		virtual void ProcessBodyContent(const InternalContext* context) override
		{
			ImGui::Text(reinterpret_cast<animation::state_machine::StateNode*>(GetNode())->GetName().c_str());
		}
		virtual bool Draw(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes) override;
		virtual void DrawBody(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes) override;

		void DrawTransition(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes);
		
	private:
		struct TransitionEstablish
		{
			graphs::BaseNode* From = nullptr;
			graphs::BaseNode* To = nullptr;
			void Reset() { From = nullptr; To = nullptr; }
		};

		static TransitionEstablish m_TransitionEstablish;
	};

	
	class StateMachineNodeDeligate : public GraphNodePrototype
	{
	public:
		StateMachineNodeDeligate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~StateMachineNodeDeligate() = default;

		virtual void ProcessBodyContent(const InternalContext* context) override;

		virtual void ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes) override;
		virtual void ProcessPopup(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, const std::string& search) override;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
	private:
		
	};
	class OutputPoseNodeDelegate : public GraphNodePrototype
	{
	public:
		OutputPoseNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~OutputPoseNodeDelegate() = default;
		virtual void ProcessBodyContent(const InternalContext* context) override;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
	};

	class PoseNodeDelegate : public GraphNodePrototype
	{
	public:
		PoseNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~PoseNodeDelegate() = default;

		virtual void ProcessBodyContent(const InternalContext* context) override;
		virtual void ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes) override;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
	private:
		std::string m_Search;
		bool m_IsAnimationPopupActive = false;
	};

	class OutputTransitionNodeDelegate : public GraphNodePrototype
	{
	public:
		OutputTransitionNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~OutputTransitionNodeDelegate() = default;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
		virtual void ProcessBodyContent(const InternalContext* context) override;
		virtual void ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes) override;
	};

	class BlendNode2DNodeDelegate : public GraphNodePrototype
	{
	public:
		BlendNode2DNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~BlendNode2DNodeDelegate() = default;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
	};

	class BoneMaskNodeDelegate : public GraphNodePrototype
	{
	public:
		BoneMaskNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~BoneMaskNodeDelegate() = default;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
		virtual void ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes) override;
		virtual void ProcessBodyContent(const InternalContext* context) override;
	private:
		std::string m_Search;
	};

	class IntEqualsNodeDelegate : public GraphNodePrototype
	{
	public:
		IntEqualsNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~IntEqualsNodeDelegate() = default;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
	};

	class IntNodeDelegate : public GraphNodePrototype
	{
	public:
		IntNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~IntNodeDelegate() = default;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
		virtual void ProcessBodyContent(const InternalContext* context) override;
	};

	class FloatEqualsNodeDelegate : public GraphNodePrototype
	{
	public:
		FloatEqualsNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~FloatEqualsNodeDelegate() = default;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
	};

	class FloatNodeDelegate : public GraphNodePrototype
	{
	public:
		FloatNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~FloatNodeDelegate() = default;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
		virtual void ProcessBodyContent(const InternalContext* context) override;
	};

	class StringNodeDelegate : public GraphNodePrototype
	{
	public:
		StringNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~StringNodeDelegate() = default;
		virtual void ProcessBodyContent(const InternalContext* context) override;
	};

	class BoolNodeDelegate : public GraphNodePrototype
	{
	public:
		BoolNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~BoolNodeDelegate() = default;
		virtual void ProcessBodyContent(const InternalContext* context) override;
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
