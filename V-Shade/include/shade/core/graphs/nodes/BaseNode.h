#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/graphs/nodes/NodeValue.h>
#include <shade/core/time/Timer.h>
#include <shade/core/graphs/GraphContext.h>
#include <glm/glm/glm.hpp>
#include <ctti/type_id.hpp>

namespace shade
{
	namespace graphs
	{
		using NodeType = std::uint32_t;
		// Risky!! Try use STTI lib
		template <typename T>
		SHADE_INLINE constexpr NodeType GetNodeTypeId()
		{
			return static_cast<NodeType>(ctti::type_id<T>().hash());
		}

#define NODE_STATIC_TYPE_HELPER(atype) \
		public: \
			static SHADE_INLINE std::uint32_t GetNodeStaticType() { return graphs::GetNodeTypeId<atype>(); } \
			virtual SHADE_INLINE std::uint32_t GetNodeType() const override { return GetNodeStaticType(); } \

		
		/// @brief Represent base graph node implementation.
		class SHADE_API BaseNode
		{
		public:
			SHADE_CAST_HELPER(BaseNode)

			/// @brief Default BaseNode constructor.
			/// @param NodeIdentifier node identifier.
			BaseNode(GraphContext* context  = nullptr, NodeIdentifier identifier =~0, BaseNode* pParentNode = nullptr, const std::string& name = "Node");

			/// @brief Virtual destructor for BaseNode.
			virtual ~BaseNode();

			static std::uint32_t GetNodeStaticType();
			
			virtual std::uint32_t GetNodeType() const;
		public:
			/// @brief Initialization of node.
			/// @param BaseGraph* The parent graph which can contain the current node.
			virtual void Initialize() {};

			/// @brief Shuts down the node, removing its parent association.
			virtual void Shutdown();

			/// @brief Function for processing the branch of nodes
			/// @param deltaTime The time elapsed since the last processing
			void ProcessBranch(const FrameTimer& deltaTime);

			/// @brief Connects nodes based on their endpoints.
			/// @param connectedToEndpoint The endpoint identifier to connect to.
			/// @param pConnectedFrom The node being connected from.
			/// @param connectedFromEndpoint The endpoint identifier of the node being connected from.
			/// @return True if successful, otherwise false.
			bool ConnectNodes(EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint);

			/// @brief Disconnects nodes based on their endpoints.
			/// @param connectedToEndpoint The endpoint identifier to disconnect from.
			/// @param pConnectedFrom The node being disconnected from.
			/// @param connectedFromEndpoint The endpoint identifier of the node being disconnected from.
			/// @return True if successful, otherwise false.
			bool DisconnectNodes(EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint);

			/// @brief Sets the root node for this node.
			/// @param node Pointer to the new root node.
			void SetRootNode(BaseNode* node);

			/// @brief Retrieves the root node.
			/// @return Pointer to the root node.
			BaseNode* GetRootNode();

			/// @brief Retrieves the root node (const version).
			/// @return Const pointer to the root node.
			const BaseNode* GetRootNode() const;

			/// @brief Sets the name of the node.
			/// @param name The new name of the node.
			SHADE_INLINE void SetName(const std::string& name)
			{
				m_Name = name;
			}

			/// @brief Retrieves the name of the node.
			/// @return Const reference to the node name.
			SHADE_INLINE const std::string& GetName() const
			{
				return m_Name;
			}

			/// @brief Checks if the node can be opened.
			/// @return True if the node can be opened, otherwise false.
			SHADE_INLINE bool CanBeOpen() const
			{
				return m_CanBeOpen;
			}

			/// @brief Checks if the node is removable.
			/// @return True if the node is removable, otherwise false.
			SHADE_INLINE bool IsRemovable() const
			{
				return m_IsRemovable;
			}

			/// @brief Checks if the node is renamable.
			/// @return True if the node is renamable, otherwise false.
			SHADE_INLINE bool IsRenamable() const
			{
				return m_IsRenamable;
			}

			/// @brief Checks if the node is the root of the graph.
			/// @return True if the node is the root, otherwise false.
			SHADE_INLINE bool IsRootGraph() const
			{
				return !HasParrent();
			}

			/// @brief Checks if the node has a parent node.
			/// @return True if the node has a parent, otherwise false.
			SHADE_INLINE bool HasParrent() const
			{
				return m_pParrentNode != nullptr;
			}

			/// @brief Creates a new node and sets it as a child of this node.
			/// @tparam T The type of the node to create.
			/// @tparam Args The types of arguments for the node constructor.
			/// @param args Arguments to forward to the node constructor.
			/// @return Pointer to the created node.
			template<typename T, typename... Args>
			T* CreateNode(Args&&... args)
			{
				// Set created node as root in case there's no root already
				return GetGraphContext()->CreateNode<T>(this, std::forward<Args>(args)...);
			}

			/// @brief Removes a node.
			/// @param pNode Pointer to the node to remove.
			/// @return True if the node was removed, otherwise false.
			virtual bool RemoveNode(BaseNode* pNode);

			/// @brief Retrieves all connections for this node.
			/// @return Reference to a vector of connections.
			SHADE_INLINE std::vector<Connection>& GetConnections()
			{
				return *m_pGraphContext->GetConnections(this);
			}

			/// @brief Retrieves all connections for this node (const version).
			/// @return Const reference to a vector of connections.
			SHADE_INLINE const std::vector<Connection>& GetConnections() const
			{
				return *m_pGraphContext->GetConnections(this);
			}

			/// @brief Setter for the node identifier.
			/// @param The identifier of the graph node.
			SHADE_INLINE void SetNodeIdentifier(NodeIdentifier id)
			{
				m_NodeIdentifier = id;
			};

			/// @brief Getter for the node identifier.
			/// @return The identifier of the graph node.
			SHADE_INLINE NodeIdentifier GetNodeIdentifier() const
			{
				return m_NodeIdentifier;
			};

			//@brief Get all child nodes
			/// @return A std::vector<BaseNode*>&
			SHADE_INLINE std::vector<BaseNode*>& GetInternalNodes()
			{
				return m_pGraphContext->GetInternalNodes(this);
			}

			/// @brief Get all child nodes
			/// @return A const std::vector<BaseNode*>&
			SHADE_INLINE const std::vector<BaseNode*>& GetInternalNodes() const
			{
				return m_pGraphContext->GetInternalNodes(this);
			}

			/// @brief Checks if the current node has a parent graph.
			/// @return True if it has, otherwise false.
			SHADE_INLINE bool HasParentNode() const
			{
				return m_pParrentNode != nullptr;
			}

			/// @brief Retrieves the parent graph.
			/// @return Pointer to the parent graph. Can be nullptr if there is no parent.
			SHADE_INLINE BaseNode* GetParrentGraph()
			{
				return m_pParrentNode;
			}

			/// @brief Retrieves the parent graph (const version).
			/// @return Const pointer to the parent graph. Can be nullptr if there is no parent.
			SHADE_INLINE const BaseNode* GetParrentGraph() const
			{
				return m_pParrentNode;
			}
			/// @brief Getter for the graph context
			/// @return A pointer to the graph context
			SHADE_INLINE GraphContext* GetGraphContext()
			{
				return m_pGraphContext;
			}
			/// @brief Getter for the graph context
			/// @return A const pointer to the graph context
			SHADE_INLINE const GraphContext* GetGraphContext() const
			{
				return m_pGraphContext;
			}
			/// @brief Retrieves the root parent graph.
			/// @return Const pointer to the root parent graph.
			SHADE_INLINE const BaseNode* GetParrentRootGraph() const { return m_pRootParrentNode; }

			/// @brief Retrieves the root parent graph.
			/// @return Pointer to the root parent graph.
			SHADE_INLINE BaseNode* GetParrentRootGraph() { return m_pRootParrentNode; }

			//@brief Get screen position(For editor only)
			/// @return A const glm::vec2
			SHADE_INLINE const glm::vec2& GetScreenPosition() const
			{
				return m_ScreenPosition;
			}
			/// @brief Get screen position(For editor only)
			/// @return A glm::vec2
			SHADE_INLINE glm::vec2& GetScreenPosition()
			{
				return m_ScreenPosition;
			}

			/// @brief Virtual function for evaluating the node.
			/// @param deltaTime The time elapsed since the last evaluation.
			virtual void Evaluate(const FrameTimer& deltaTime) = 0;

			/// @brief Virtual function for handling connection events when a node is connected.
			/// @param connectionType The type of the connection (Input or Output).
			/// @param type The type of the node value being connected.
			/// @param endpoint The identifier of the endpoint being connected.
			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) { /*assert(false && "In case if you are using this function you need to impliment it fist!"); */ };

			/// @brief Virtual function for handling connection events when a node is disconnected.
			/// @param connectionType The type of the connection (Input or Output).
			/// @param type The type of the node value being disconnected.
			/// @param endpoint The identifier of the endpoint being disconnected.
			virtual void OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint);

			/// @brief Template function for getting the value of a specific endpoint.
			/// @tparam T The type of the connection (Input or Output).
			/// @param index The identifier of the endpoint.
			/// @return The value of the specified endpoint.
			template<typename Connection::Type T>
			SHADE_INLINE NodeValues::Value GetEndpoint(EndpointIdentifier index)
			{
				return ((index < m_Endpoints[static_cast<std::size_t>(T)].GetSize()) ? m_Endpoints[static_cast<std::size_t>(T)].At(index) : nullptr);
			}

			/// @brief Template function for getting the value of a specific endpoint (const version).
			/// @tparam T The type of the connection (Input or Output).
			/// @param index The identifier of the endpoint.
			/// @return The value of the specified endpoint.
			template<typename Connection::Type T>
			SHADE_INLINE const NodeValues::Value GetEndpoint(EndpointIdentifier index) const
			{
				return ((index < m_Endpoints[static_cast<std::size_t>(T)].GetSize()) ? m_Endpoints[static_cast<std::size_t>(T)].At(index) : NodeValues::Value());
			}

			/// @brief Getter for all endpoints.
			/// @return Reference to the array of endpoints.
			SHADE_INLINE std::array<NodeValues, static_cast<std::size_t>(Connection::Type::MAX_ENUM)>& GetEndpoints()
			{
				return m_Endpoints;
			}

			/// @brief Getter for all endpoints (const version).
			/// @return Const reference to the array of endpoints.
			SHADE_INLINE const std::array<NodeValues, static_cast<std::size_t>(Connection::Type::MAX_ENUM)>& GetEndpoints() const
			{
				return m_Endpoints;
			}

			/*/// @brief Getter for all endpoints.
			/// @return Reference to the array of endpoints.
			template<typename Connection::Type ConnectionType>
			SHADE_INLINE NodeValues& GetEndpoints()
			{
				return m_Endpoints[static_cast<std::size_t>(ConnectionType)];
			}

			/// @brief Getter for all endpoints.
			/// @return Reference to the array of endpoints.
			template<typename Connection::Type ConnectionType>
			SHADE_INLINE const NodeValues& GetEndpoints() const
			{
				return m_Endpoints[static_cast<std::size_t>(ConnectionType)];
			}*/

			/// @brief Gets the count of all endpoints.
			/// @return The number of endpoints.
			SHADE_INLINE std::size_t GetEndpointCount() const
			{
				return m_Endpoints[Connection::Type::Input].GetSize() + m_Endpoints[Connection::Type::Output].GetSize();
			}

			//@brief Template function for registering an endpoint of a specific type
			/// @tparam ConnectionType The type of the connection (Input or Output)
			/// @tparam ValueType The type of the node value to be registered
			/// @tparam Args Additional arguments for value initialization
			/// @return The index of the registered endpoint
			template<typename Connection::Type ConnectionType, typename NodeValueType ValueType, typename... Args>
				requires IsNodeValueType<typename FromNodeValueTypeToType<ValueType>::Type>
			SHADE_INLINE EndpointIdentifier REGISTER_ENDPOINT(Args&&... args)
			{
				return m_Endpoints[static_cast<std::size_t>(ConnectionType)].Emplace<ValueType>(std::forward<Args>(args)...);
			}

			/// @brief Template function for removing an endpoint
			/// @tparam ConnectionType The type of the connection (Input or Output)
			template<typename Connection::Type ConnectionType>
			SHADE_INLINE void REMOVE_ENDPOINT(EndpointIdentifier index)
			{
				m_Endpoints[static_cast<std::size_t>(ConnectionType)].Remove(index);
			}

			/// @brief Template function for getting the value of a specific endpoint
			/// @tparam ConnectionType The type of the connection (Input or Output)
			/// @tparam ValueType The type of the node value to be retrieved
			/// @tparam Args Additional arguments for value initialization
			/// @return A reference to the retrieved endpoint value
			template<typename Connection::Type ConnectionType, typename NodeValueType ValueType, typename... Args>
				requires IsNodeValueType<typename FromNodeValueTypeToType<ValueType>::Type>
			SHADE_INLINE typename FromNodeValueTypeToType<ValueType>::Type& GET_ENDPOINT(EndpointIdentifier index, Args&&... args)
			{
				if constexpr (sizeof...(args) > 0)
					m_Endpoints[static_cast<std::size_t>(ConnectionType)].At(index)->Initialize<ValueType>(std::forward<Args>(args)...);

				return m_Endpoints[static_cast<std::size_t>(ConnectionType)].At(index)->As<ValueType>();
			}

			/// @brief Template function for getting the value of a specific endpoint (const version)
			/// @tparam ConnectionType The type of the connection (Input or Output)
			/// @tparam ValueType The type of the node value to be retrieved
			/// @return A const reference to the retrieved endpoint value
			template<typename Connection::Type ConnectionType, typename NodeValueType ValueType>
				requires IsNodeValueType<typename FromNodeValueTypeToType<ValueType>::Type>
			SHADE_INLINE const typename FromNodeValueTypeToType<ValueType>::Type& GET_ENDPOINT(EndpointIdentifier index) const
			{
				return m_Endpoints[static_cast<std::size_t>(ConnectionType)].At(index)->As<ValueType>();
			}

			/// @brief Helper function for getting the pointer to an endpoint value
			/// @tparam T The type of the connection (Input or Output)
			/// @param index The index of the endpoint
			/// @return A pointer to the endpoint value or nullptr if index is out of bounds
			template<typename Connection::Type T>
			SHADE_INLINE NodeValues::Value* __GET_ENDPOINT(EndpointIdentifier index)
			{
				return ((index < m_Endpoints[static_cast<std::size_t>(T)].GetSize()) ? &m_Endpoints[static_cast<std::size_t>(T)].At(index) : nullptr);
			}

			/// @brief Helper function for getting the const pointer to an endpoint value
			/// @tparam T The type of the connection (Input or Output)
			/// @param index The index of the endpoint
			/// @return A const pointer to the endpoint value or nullptr if index is out of bounds
			template<typename Connection::Type T>
			SHADE_INLINE const NodeValues::Value* __GET_ENDPOINT(EndpointIdentifier index) const
			{
				return ((index < m_Endpoints[static_cast<std::size_t>(T)].GetSize()) ? &m_Endpoints[static_cast<std::size_t>(T)].At(index) : nullptr);
			}
		protected:
			/// @brief Protected function for evaluating node behavior based on input/output values.
			virtual void OnEvaluate(const FrameTimer& deltaTime) {};

			/// @brief Reference to the graph context.
			GraphContext* m_pGraphContext;

			/// @brief Identifier for the node.
			NodeIdentifier m_NodeIdentifier;
		
			/// @brief The root node.
			BaseNode* m_pRootNode = nullptr;

			/// @brief The parent node.
			BaseNode* m_pParrentNode = nullptr;

			/// @brief The root parent node.
			BaseNode* m_pRootParrentNode = nullptr;

			/// @brief The name of the node.
			std::string m_Name;

			/// @brief Flag to indicate if the node can be opened.
			bool m_CanBeOpen;

			/// @brief Flag to indicate if the node is removable.
			bool m_IsRemovable;

			/// @brief Flag to indicate if the node can be renamed.
			bool m_IsRenamable;

			/// @brief Screen position of node, uses only in editor.
			glm::vec2 m_ScreenPosition = glm::vec2(0.f);

			/// @brief Array of node values for input and output connections.
			std::array<NodeValues, static_cast<std::size_t>(Connection::Type::MAX_ENUM)> m_Endpoints;

			/// @brief Serializes the base node to the given output stream.
			/// @param stream The output stream to serialize to.
			/// @return The number of bytes written.
			virtual void Serialize(std::ostream& stream) const;

			/// @brief Deserializes the base node from the given input stream.
			/// @param stream The input stream to deserialize from.
			/// @return The number of bytes read.
			virtual void Deserialize(std::istream& stream);

			/// @brief Serializes the body of base node to the given output stream.
			/// @param stream The output stream to serialize to.
			/// @return The number of bytes written.
			virtual void SerializeBody(std::ostream& stream) const;

			/// @brief Deserializes the body of base node to the given output stream.
			/// @param stream The output stream to deserialize from.
			/// @return The number of bytes written.
			virtual void DeserializeBody(std::istream& stream);

		
			BaseNode* CreateNodeByType(NodeType type);

			friend class serialize::Serializer;
		};
	}

	// Serialize AnimationGraph
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const shade::graphs::BaseNode& node)
	{
		return node.Serialize(stream);
	}

	// Deserialize AnimationGraph
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, shade::graphs::BaseNode& node)
	{
		return node.Deserialize(stream);
	}
}