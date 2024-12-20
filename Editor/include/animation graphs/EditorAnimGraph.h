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
#define VEC2_VALUE_COLOR { 0.8f, 0.8f, 0.5f, 1.f }

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
		case shade::NodeValueType::Vector2: return VEC2_VALUE_COLOR;
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

			ImVec2 Size = ImVec2{ 150.f, 150.f };
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
		virtual void DrawNodes();

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
		~GraphEditor();

		/**
		 * @brief Initializes the editor with the given animation graph asset.
		 * @param graph The animation graph asset to initialize with.
		 */
		 void Initialize(Asset<AnimationGraph>& graph);

		/**
		 * @brief Recursively initializes nodes and populates the provided map.
		 * @param pNode The current node to initialize.
		 * @param nodes The map to populate with nodes and their prototypes.
		 */
		 void InitializeRecursively(graphs::BaseNode* pNode, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, bool cursor = false);

		/**
		 * @brief Retrieves the prototyped node corresponding to the given base node.
		 * @param pNode The base node to find the prototype for.
		 * @return Pointer to the prototyped node, or nullptr if not found.
		 */
		 GraphNodePrototype* GetPrototypedNode(graphs::BaseNode* pNode);

		/**
		 * @brief Retrieves the prototyped refer node corresponding to the given base node.
		 * @param pNode The base node to find the refer prototype for.
		 * @return Pointer to the prototyped refer node, or nullptr if not found.
		 */
		 GraphNodePrototype* GetPrototypedReferNode(graphs::BaseNode* pNode);

		/**
		 * @brief Gets the map of all prototyped nodes.
		 * @return Reference to the map of prototyped nodes.
		 */
		SHADE_INLINE  std::unordered_map<std::size_t, GraphNodePrototype*>& GetNodes() { return m_Nodes; }

		/**
		 * @brief Gets the map of all prototyped refer nodes.
		 * @return Reference to the map of prototyped refer nodes.
		 */
		SHADE_INLINE  std::unordered_map<std::size_t, GraphNodePrototype*>& GetReferNodes() { return m_ReferNodes; }

		/**
		 * @brief Gets the root graph node.
		 * @return Pointer to the root graph node.
		 */
		SHADE_INLINE  graphs::BaseNode* GetRootGraph() { return m_pRootGraph.Raw(); }

		/**
		 * @brief Marks a node for removal.
		 * @param pNode The node to mark for removal.
		 */
		 void SetToRemove(graphs::BaseNode* pNode);

		 SHADE_INLINE InternalContext& GetContext() { return m_Context; }
		 SHADE_INLINE GraphVisualStyle& GetVisualStyle() { return m_VisualStyle; }
		 SHADE_INLINE void SetSelectedNode(GraphNodePrototype* node) { m_pSelectedNode = node; }
		 SHADE_INLINE GraphNodePrototype* GetSelectedNode() { return m_pSelectedNode; }

	public:
		/**
		 * @brief Opens the editor UI to edit the graph.
		 * @param title The title of the editor window.
		 * @param size The size of the editor window.
		 * @param menuCallBack Optional callback function for menu actions.
		 * @return True if the graph was edited, false otherwise.
		 */
		 bool Edit(const char* title, const ImVec2& size, const std::function<void()>& menuCallBack = std::function<void()>());

	private:
		/**
		 * @brief Creates a node of type T and adds it to the provided map.
		 * @tparam T The type of the node to create.
		 * @param pNode The base node to create from.
		 * @param nodes The map to add the created node prototype to.
		 */
		template<typename T>
		SHADE_INLINE  void CreateNode(shade::graphs::BaseNode* pNode, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes)
		{
			nodes.emplace(std::size_t(pNode) ^ pNode->GetNodeIdentifier(), new T(pNode, this));	
		}

		// Root animation graph asset
		 Asset<AnimationGraph> m_pRootGraph;

		// Currently selected graph node prototype
		 GraphNodePrototype* m_pSelectedNode = nullptr;

		// Maps of nodes and refer nodes prototypes
		 std::unordered_map<std::size_t, GraphNodePrototype*>  m_Nodes;
		 std::unordered_map<std::size_t, GraphNodePrototype*>  m_ReferNodes;

		// Path of graph nodes
		 std::vector<GraphNodePrototype*> m_Path;

		// Internal context and visual style for the graph editor
		 InternalContext  m_Context;
		 GraphVisualStyle m_VisualStyle;

		/**
		 * @brief Removes the specified node.
		 * @param pNode The node to remove.
		 * @return True if the node was successfully removed, false otherwise.
		 */
		 bool RemoveNode(graphs::BaseNode*& pNode);

		/**
		 * @brief Processes scaling of the graph editor view.
		 */
		 void ProcessScale();

		/**
		 * @brief Calculates the world position of the mouse based on its screen position.
		 * @param mousePosition The position of the mouse on the screen.
		 * @return The calculated world position of the mouse.
		 */
		 ImVec2 CalculateMouseWorldPos(const ImVec2& mousePosition);

		/**
		 * @brief Draws all nodes in the graph.
		 */
		 void DrawNodes();

		/**
		 * @brief Draws all connections between nodes.
		 */
		 void DrawConnections();

		/**
		 * @brief Displays a popup menu for graph nodes.
		 */
		 void PopupMenu();

		/**
		 * @brief Recursively draws the path for the specified node.
		 * @param pNode The node to start drawing the path from.
		 */
		 void DrawPathRecursevly(graphs::BaseNode* pNode);

		 // Node marked for removal
		 graphs::BaseNode* m_pNodeToRemove = nullptr;

		 bool m_Initialized = false;
	};


	class AnimationGraphDelegate : public GraphNodePrototype
	{
	public:
		AnimationGraphDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor) : GraphNodePrototype(pNode, pEditor)
		{
			Style.Title = "Animation graph";
		}
		virtual ~AnimationGraphDelegate() = default;
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
	private:
	};

	class StateMachineNodeDelegate : public GraphNodePrototype
	{
	public:
		StateMachineNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~StateMachineNodeDelegate() = default;

		virtual void ProcessBodyContent(const InternalContext* context) override;

		virtual void ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes) override;
		virtual void ProcessPopup(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, const std::string& search) override;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;

		virtual bool Draw(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes) override;
		virtual void DrawBody(const InternalContext* context, const GraphVisualStyle* graphStyle, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes) override;
		virtual void DrawNodes() override;
		void DrawTransitions();

		void TransitionEsteblish();
	private:
		struct TransitionEstablish
		{
			graphs::BaseNode* From = nullptr;
			graphs::BaseNode* To = nullptr;
			void Reset() { From = nullptr; To = nullptr; }
		};

		TransitionEstablish m_TransitionEstablish;
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

	class BlendNodeDelegate : public GraphNodePrototype
	{
	public:
		BlendNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~BlendNodeDelegate() = default;
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
	class FloatScaleRangeDelegate : public GraphNodePrototype
	{
	public:
		FloatScaleRangeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~FloatScaleRangeDelegate() = default;
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

	class BlendTree2DNodeDelegate : public GraphNodePrototype
	{
	public:
		BlendTree2DNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~BlendTree2DNodeDelegate() = default;
		virtual void ProcessBodyContent(const InternalContext* context) override;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
		virtual void ProcessSideBar(const InternalContext* context, std::unordered_map<std::size_t, GraphNodePrototype*>& nodes, std::unordered_map<std::size_t, GraphNodePrototype*>& referNodes) override;
	};

	class Vec2FloatDNodeDelegate : public GraphNodePrototype
	{
	public:
		Vec2FloatDNodeDelegate(graphs::BaseNode* pNode, GraphEditor* pEditor);
		virtual ~Vec2FloatDNodeDelegate() = default;
		virtual void ProcessEndpoint(graphs::EndpointIdentifier identifier, graphs::Connection::Type type, NodeValue& endpoint) override;
		virtual void ProcessBodyContent(const InternalContext* context) override;
	};
}