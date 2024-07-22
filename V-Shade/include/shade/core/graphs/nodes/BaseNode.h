#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/graphs/nodes/NodeValue.h>
#include <shade/core/time/Timer.h>
#include <shade/core/graphs/GraphContext.h>
#include <glm/glm/glm.hpp>

namespace shade
{
	namespace graphs
	{
		/// @brief Represent base graph node implementation.
		class SHADE_API BaseNode
		{
		public:
			SHADE_CAST_HELPER(BaseNode)

				/// @brief Default BaseNode constructor.
				/// @param NodeIdentifier node identifier.
				BaseNode(GraphContext* context, NodeIdentifier identifier);


			/// @brief Default BaseNode constructor.
			/// @param NodeIdentifier node identifier.
			BaseNode(GraphContext* context, NodeIdentifier identifier, const std::string& name);

			virtual ~BaseNode();

		public:
			/// @brief Initialization of node.
			/// @param BaseGraph* The parent graph which can contains current node.
			virtual void Initialize(BaseNode* pParentGraph = nullptr, BaseNode* pRootParrentNode = nullptr);

			/// @brief Unparrent.
			virtual void Shutdown();

			/// @brief
			/// @param
			/// @param
			/// @param
			/// @param
			/// @return
			bool ConnectNodes(BaseNode* inputNode, EndpointIdentifier inputEndpoint, BaseNode* outputNode, EndpointIdentifier outputEndpoint);
			/// @brief
			/// @param
			/// @param
			/// @param
			/// @param
			/// @return
			bool DisconnectNodes(NodeIdentifier inputNode, EndpointIdentifier inputEndpoint, NodeIdentifier outputNode, EndpointIdentifier outputEndpoint);

			/// @brief
			/// @param
			/// @return
			void SetRootNode(BaseNode* node);

			/// @brief
			/// @return
			BaseNode* GetRootNode();

			/// @brief
			/// @return
			const BaseNode* GetRootNode() const;

			/// @brief
			/// @param
			/// @return
			BaseNode* FindNode(NodeIdentifier identifier);

			/// @brief
			/// @return
			SHADE_INLINE void SetName(const std::string& name)
			{
				m_Name = name;
			}

			/// @brief
			/// @return
			SHADE_INLINE const std::string& GetName() const
			{
				return m_Name;
			}

			/// @brief
			/// @return
			SHADE_INLINE bool CanBeOpen() const
			{
				return m_CanBeOpen;
			}

			/// @brief
			/// @return
			SHADE_INLINE bool IsRemovable() const
			{
				return m_IsRemovable;
			}

			/// @brief
			/// @return
			SHADE_INLINE bool IsRenamable() const
			{
				return m_IsRenamable;
			}

			/// @brief
			/// @return
			SHADE_INLINE bool IsRootGraph() const
			{
				return !HasParrent();
			}

			/// @brief
			/// @return
			SHADE_INLINE bool HasParrent() const
			{
				return m_pParrentNode != nullptr;
			}

			/// @brief
			void AddReferNode(BaseNode* node)
			{
				if (std::find(m_ReferNodes.begin(), m_ReferNodes.end(), node) == m_ReferNodes.end());
				m_ReferNodes.emplace_back(node);
			}

			template<typename T, typename... Args>
			T* CreateNode(Args&&... args)
			{
				T* node = SNEW T(GetGraphContext(), m_Nodes.size(), std::forward<Args>(args)...);
				m_Nodes.emplace_back(node)->Initialize(this, (!m_pRootParrentNode) ? this : m_pRootParrentNode);
				return node;
			}

			/// @brief 
			/// @return 
			bool RemoveNode(BaseNode* pNode);


			/// @brief Getter for the node identifier
			/// @return The index of the graph node
			SHADE_INLINE NodeIdentifier GetNodeIdentifier() const
			{
				return m_NodeIdentifier;
			};

			/// @brief Checks if the current node has a parent graph.
			/// @return True if has, otherwise false.
			SHADE_INLINE bool HasParentNode() const
			{
				return m_pParrentNode != nullptr;
			}

			/// @brief Get parrent graph pointer.
			/// @return BaseGraph* pointer to parrent graph. Can be nullptr if here's no parrent graph.
			SHADE_INLINE BaseNode* GetParrentGraph()
			{
				return m_pParrentNode;
			}

			/// @brief Get parrent graph const pointer.
			/// @return const BaseGraph* pointer to parrent graph. Can be nullptr if here's no parrent graph.
			SHADE_INLINE const BaseNode* GetParrentGraph() const
			{
				return m_pParrentNode;
			}
			SHADE_INLINE const BaseNode* GetParrentRootGraph() const { return m_pRootParrentNode; }
			SHADE_INLINE BaseNode* GetParrentRootGraph() { return m_pRootParrentNode; }

			/// @brief Virtual function for evaluating the node
			/// @param deltaTime The time elapsed since the last evaluation
			virtual void Evaluate(const FrameTimer& deltaTime) = 0;

			/// @brief Virtual function for handling connection events when a node is connected
			/// @param connectionType The type of the connection (Input or Output)
			/// @param type The type of the node value being connected
			/// @param endpoint The index of the endpoint being connected
			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) { /*assert(false && "In case if you are using this function you need to impliment it fist!"); */ };

			/// @brief Virtual function for handling connection events when a node is disconnected
			/// @param connectionType The type of the connection (Input or Output)
			/// @param type The type of the node value being disconnected
			/// @param endpoint The index of the endpoint being disconnected
			virtual void OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint);

			/// @brief Template function for getting the value of a specific endpoint
			/// @tparam T The type of the connection (Input or Output)
			/// @param index The index of the endpoint
			/// @return The value of the specified endpoint
			template<typename Connection::Type T>
			SHADE_INLINE NodeValues::Value GetEndpoint(EndpointIdentifier index)
			{
				return ((index < m_Endpoints[static_cast<std::size_t>(T)].GetSize()) ? m_Endpoints[static_cast<std::size_t>(T)].At(index) : nullptr);
			}

			/// @brief Template function for getting the value of a specific endpoint (const version)
			/// @tparam T The type of the connection (Input or Output)
			/// @param index The index of the endpoint
			/// @return The value of the specified endpoint
			template<typename Connection::Type T>
			SHADE_INLINE const NodeValues::Value GetEndpoint(EndpointIdentifier index) const
			{
				return ((index < m_Endpoints[static_cast<std::size_t>(T)].GetSize()) ? m_Endpoints[static_cast<std::size_t>(T)].At(index) : NodeValues::Value());
			}

			/// @brief Getter for all endpoints
			/// @return A reference to the array of endpoints
			SHADE_INLINE std::array<NodeValues, std::size_t(Connection::Type::MAX_ENUM)>& GetEndpoints()
			{
				return m_Endpoints;
			};

			/// @brief Const getter for all endpoints
			/// @return A const reference to the array of endpoints
			SHADE_INLINE const std::array<NodeValues, std::size_t(Connection::Type::MAX_ENUM)>& GetEndpoints() const
			{
				return m_Endpoints;
			};

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
			/// @brief Get screen position(For editor only)
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
			/// @brief Get all child nodes
			/// @return A std::vector<BaseNode*>&
			SHADE_INLINE std::vector<BaseNode*>& GetNodes()
			{
				return m_Nodes;
			}
			/// @brief Get all child nodes
			/// @return A const std::vector<BaseNode*>&
			SHADE_INLINE const std::vector<BaseNode*>& GetNodes() const
			{
				return m_Nodes;
			}

			SHADE_INLINE std::vector<BaseNode*>& GetReferNodes()
			{
				return m_ReferNodes;
			}

			SHADE_INLINE const std::vector<BaseNode*>& GetReferNodes() const
			{
				return m_ReferNodes;
			}

			/// @brief Function for processing the branch of nodes
			/// @param deltaTime The time elapsed since the last processing
			void ProcessBranch(const FrameTimer& deltaTime);
		public:

			bool ConnectValues(std::shared_ptr<NodeValue>* inputEndpoint, const std::shared_ptr<NodeValue>* outputEndpoint);

			/// @brief Template function for registering an endpoint of a specific type
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
			BaseNode* m_pParrentNode = nullptr;
			BaseNode* m_pRootParrentNode = nullptr;
			BaseNode* m_pRootNode = nullptr;
			glm::vec2							m_ScreenPosition = glm::vec2(0.f);
			std::vector<BaseNode*>				m_Nodes;
			std::vector<BaseNode*>				m_ReferNodes;
			std::string							m_Name = "Node";
			bool								m_IsRenamable = true;
			bool								m_IsRemovable = true;
			bool								m_CanBeOpen = true;
		private:
			NodeIdentifier																	m_NodeIdentifier;
			GraphContext* const 															m_pGraphContext = nullptr;
			std::array<NodeValues, static_cast<std::size_t>(Connection::Type::MAX_ENUM)>	m_Endpoints;
			void RemoveReferNodeRecursively(shade::graphs::BaseNode* pNode);
		};
	}
}