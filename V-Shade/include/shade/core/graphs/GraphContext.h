//#pragma once
//#include <shade/utils/Utils.h>
//#include <shade/core/entity/Entity.h>
//#include <shade/core/graphs/nodes/NodeValue.h>
//
//namespace shade
//{
//	namespace graphs
//	{
//		class BaseNode;
//
//		/// @brief Type aliases for node and endpoint indices
//		using NodeIdentifier = std::uint32_t;
//		using EndpointIdentifier = std::uint32_t;
//
//		/// @brief Constant for representing a null node index
//		static constexpr NodeIdentifier ROOT_NODE_IDENTIFIER = 0;
//		static constexpr NodeIdentifier INVALID_NODE_IDENTIFIER = ~0;
//
//		/// @brief Structure representing a connection type within a graph node
//		struct Connection
//		{
//			enum Type : std::uint8_t
//			{
//				Input,
//				Output,
//				MAX_ENUM
//			};
//
//			// TODO Make Constructor !!
//			BaseNode* PConnectedTo = nullptr;
//			EndpointIdentifier         ConnectedToEndpoint = INVALID_NODE_IDENTIFIER;
//
//			BaseNode* PConnectedFrom = nullptr;
//			EndpointIdentifier         ConnectedFromEndpoint = INVALID_NODE_IDENTIFIER;
//
//			// Uses only for editor
//			glm::vec2 ConnectedToPosition = glm::vec2(0.f);
//			glm::vec2 ConnectedFromPosition = glm::vec2(0.f);
//		};
//
//		struct NodesPack
//		{
//			std::vector<Connection> Connections;
//			std::vector<BaseNode*>  InternalNodes;
//		};
//
//		/// @brief Structure representing the context of a graph
//		struct GraphContext
//		{
//			SHADE_CAST_HELPER(GraphContext)
//		public:
//			///@brief Current entity
//			ecs::Entity Entity;
//
//			template<typename T, typename... Args>
//			SHADE_INLINE T* CreateNode(BaseNode* pParrent, Args&&... args)
//			{
//				std::unordered_map<BaseNode*, NodesPack>::iterator parrent = Nodes.find(pParrent);
//
//				const NodeIdentifier id = (parrent != Nodes.end()) ? parrent->second.InternalNodes.size() : 0;
//
//				T* node = SNEW T(this, id, pParrent, std::forward<Args>(args)...);
//
//				if (parrent != Nodes.end()) { parrent->second.InternalNodes.emplace_back(node); }
//
//				InitializeNode(node, pParrent);
//
//				return node;
//			}
//
//			/*SHADE_INLINE BaseNode* FindInternalNode(BaseNode* pParrent, NodeIdentifier identifier)
//			{
//				std::unordered_map<BaseNode*, NodesPack>::iterator parrent = Nodes.find(pParrent);
//
//				assert(parrent != Nodes.end() && "");
//
//				std::vector<BaseNode*>& internalNodes = (*parrent).second.InternalNodes;
//
//				std::vector<BaseNode*>::iterator node = std::find_if(internalNodes.begin(), internalNodes.end(), [identifier](const BaseNode* pNode)
//					{
//						return identifier == pNode->GetNodeIdentifier();
//					});
//
//				return (node != internalNodes.end()) ? *node : nullptr;
//			}*/
//
//			SHADE_API bool RemoveNode(BaseNode* pNode);
//
//			SHADE_INLINE const Connection* FindConnection(
//				BaseNode* pConnectedTo,
//				EndpointIdentifier connectedToEndpoint,
//				BaseNode* pConnectedFrom,
//				EndpointIdentifier connectedFromEndpoint) const
//			{
//				std::unordered_map<BaseNode*, NodesPack>::const_iterator to = Nodes.find(pConnectedTo);
//				std::unordered_map<BaseNode*, NodesPack>::const_iterator from = Nodes.find(pConnectedFrom);
//				// TODO Assert and do not check 
//
//				assert(to != Nodes.end() && from != Nodes.end() && "");
//
//				const std::vector<Connection>& connections = to->second.Connections;
//
//				auto connection = std::find_if(connections.begin(), connections.end(), [pConnectedTo, pConnectedFrom, connectedToEndpoint, connectedFromEndpoint](const Connection& connection)
//					{
//						return (connection.PConnectedTo == pConnectedTo &&
//							connection.PConnectedFrom == pConnectedFrom &&
//							connection.ConnectedToEndpoint == connectedToEndpoint &&
//							connection.ConnectedFromEndpoint == connectedFromEndpoint);
//					});
//
//				return (connection != connections.end()) ? &(*connection) : nullptr;
//			}
//
//			SHADE_INLINE const std::vector<Connection>* GetConnections(const BaseNode* pConnectedTo) const
//			{
//				std::unordered_map<BaseNode*, NodesPack>::const_iterator to = Nodes.find(const_cast<BaseNode*>(pConnectedTo));
//				return (to != Nodes.end()) ? &to->second.Connections : nullptr;
//			}
//
//			SHADE_INLINE std::vector<Connection>* GetConnections(BaseNode* pConnectedTo)
//			{
//				std::unordered_map<BaseNode*, NodesPack>::iterator to = Nodes.find(pConnectedTo);
//				return (to != Nodes.end()) ? &to->second.Connections : nullptr;
//			}
//
//			SHADE_INLINE bool IsEndpointFree(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint) const
//			{
//				std::unordered_map<BaseNode*, NodesPack>::const_iterator to = Nodes.find(pConnectedTo);
//
//				if (to != Nodes.end())
//				{
//					const std::vector<Connection>& connections = to->second.Connections;
//
//					auto connection = std::find_if(connections.begin(), connections.end(), [pConnectedTo, connectedToEndpoint](const Connection& connection)
//						{
//							return (connection.ConnectedToEndpoint == connectedToEndpoint);
//						});
//
//					return connection == connections.end();
//				}
//
//				return false;
//			}
//
//			SHADE_API bool ConnectNodes(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint);
//			SHADE_API bool DisconnectNodes(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint);
//			SHADE_INLINE bool ConnectValues(std::shared_ptr<NodeValue>* inputEndpoint, const std::shared_ptr<NodeValue>* outputEndpoint)
//			{
//				if (!inputEndpoint || !outputEndpoint)
//					return false;
//
//				*inputEndpoint = *outputEndpoint;
//
//				return true;
//			}
//
//			SHADE_INLINE bool CreateConnection(
//				BaseNode* pConnectedTo,
//				EndpointIdentifier connectedToEndpoint,
//				BaseNode* pConnectedFrom,
//				EndpointIdentifier connectedFromEndpoint)
//			{
//				// That means you cannot conenect same node twice !
//				// If you want then need to check if endpoint free !
//				if (IsEndpointFree(pConnectedTo, connectedToEndpoint) && !FindConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint))
//				{
//					std::unordered_map<BaseNode*, NodesPack>::iterator to = Nodes.find(pConnectedTo);
//					to->second.Connections.emplace_back(Connection{ pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint });
//					return true;
//				}
//
//				return false;
//			}
//			SHADE_INLINE bool RemoveConnection(
//				BaseNode* pConnectedTo,
//				EndpointIdentifier connectedToEndpoint,
//				BaseNode* pConnectedFrom,
//				EndpointIdentifier connectedFromEndpoint)
//			{
//				std::unordered_map<BaseNode*, NodesPack>::iterator to = Nodes.find(pConnectedTo);
//				// TODO Assert and do not check 
//
//				assert(to != Nodes.end() && "");
//
//				std::vector<Connection>& connections = to->second.Connections;
//
//				auto connection = std::find_if(connections.begin(), connections.end(), [pConnectedTo, pConnectedFrom, connectedToEndpoint, connectedFromEndpoint](const Connection& connection)
//					{
//						return (connection.PConnectedTo == pConnectedTo &&
//							connection.PConnectedFrom == pConnectedFrom &&
//							connection.ConnectedToEndpoint == connectedToEndpoint &&
//							connection.ConnectedFromEndpoint == connectedFromEndpoint);
//					});
//
//				if (connection != connections.end())
//				{
//					connections.erase(connection);
//					return true;
//				}
//
//				return false;
//			}
//
//
//			SHADE_INLINE std::vector<BaseNode*>& GetInternalNodes(BaseNode* pNode)
//			{
//				return Nodes.at(pNode).InternalNodes;
//			}
//
//			SHADE_INLINE const std::vector<BaseNode*>& GetInternalNodes(const BaseNode* pNode) const
//			{
//				return Nodes.at(const_cast<BaseNode*>(pNode)).InternalNodes;
//			}
//
//			SHADE_API bool RemoveAllInputConnection(BaseNode* pConnectedTo);
//
//			SHADE_API bool RemoveAllOutputConnection(BaseNode* pConnectedFrom);
//
//			SHADE_INLINE bool RemoveAllConnection(BaseNode* pNode)
//			{
//				return (RemoveAllOutputConnection(pNode) | RemoveAllInputConnection(pNode));
//			}
//
//			SHADE_INLINE const std::unordered_map<BaseNode*, NodesPack>& GetNodes() const
//			{
//				return Nodes;
//			}
//			SHADE_INLINE  std::unordered_map<BaseNode*, NodesPack>& GetNodes()
//			{
//				return Nodes;
//			}
//		private:
//			SHADE_API void InitializeNode(BaseNode* pNode, BaseNode* pParrent);
//			///@brief Connections
//			std::unordered_map<BaseNode*, NodesPack>	Nodes;
//		};
//	}
//}

#pragma once
#include <shade/utils/Utils.h>
#include <shade/core/entity/Entity.h>
#include <shade/core/graphs/nodes/NodeValue.h>
#include <shade/core/serializing/Serializer.h>

namespace shade
{
	namespace graphs
	{
		class BaseNode;

		/// @brief Type aliases for node and endpoint indices.
		using NodeIdentifier = std::uint32_t;
		using EndpointIdentifier = std::uint32_t;

		/// @brief Constant for representing a null node index.
		static constexpr NodeIdentifier ROOT_NODE_IDENTIFIER = 0;
		static constexpr NodeIdentifier INVALID_NODE_IDENTIFIER = ~0;

		/// @brief Structure representing a connection type within a graph node.
		struct Connection
		{
			enum Type : std::uint8_t
			{
				Input,
				Output,
				MAX_ENUM
			};

			BaseNode* PConnectedTo = nullptr;
			EndpointIdentifier ConnectedToEndpoint = INVALID_NODE_IDENTIFIER;

			BaseNode* PConnectedFrom = nullptr;
			EndpointIdentifier ConnectedFromEndpoint = INVALID_NODE_IDENTIFIER;

			// Only used for editor.
			glm::vec2 ConnectedToPosition = glm::vec2(0.f);
			glm::vec2 ConnectedFromPosition = glm::vec2(0.f);
		};

		/// @brief Structure containing connections and internal nodes of a graph node.
		struct NodesPack
		{
			std::vector<Connection> Connections;
			std::vector<BaseNode*>  InternalNodes;
		};

		/// @brief Structure representing the context of a graph.
		struct GraphContext
		{
			SHADE_CAST_HELPER(GraphContext)
		public:
			/// @brief The current entity associated with the graph context.
			ecs::Entity Entity;

			/// @brief Creates a new node within the graph context.
			/// @tparam T The type of node to create.
			/// @tparam Args Argument types for the node constructor.
			/// @param pParrent The parent node to which the new node belongs.
			/// @param args Arguments to pass to the node constructor.
			/// @return A pointer to the newly created node.
			template<typename T, typename... Args>
			SHADE_INLINE T* CreateNode(BaseNode* pParrent, Args&&... args)
			{
				std::unordered_map<BaseNode*, NodesPack>::iterator parrent = Nodes.find(pParrent);

				const NodeIdentifier id = GenerateRandomValue<std::uint32_t>(0, std::numeric_limits<std::uint32_t>::max());

				T* node = SNEW T(this, id, pParrent, std::forward<Args>(args)...);

				if (parrent != Nodes.end())
				{
					parrent->second.InternalNodes.emplace_back(node); // Add node to parent's internal nodes
				}

				InitializeNode(node, pParrent); // Initialize the newly created node

				return node; // Return the created node
			}

			/// @brief Removes a node from the graph context.
			/// @param pNode Pointer to the node to be removed.
			/// @return True if the node was successfully removed, otherwise false.
			SHADE_API bool RemoveNode(BaseNode* pNode);

			/// @brief Finds a connection between two nodes.
			/// @param pConnectedTo Pointer to the node that is the destination of the connection.
			/// @param connectedToEndpoint The endpoint identifier for the destination node.
			/// @param pConnectedFrom Pointer to the node that is the source of the connection.
			/// @param connectedFromEndpoint The endpoint identifier for the source node.
			/// @return Pointer to the found connection, or nullptr if not found.
			SHADE_INLINE const Connection* FindConnection(
				BaseNode* pConnectedTo,
				EndpointIdentifier connectedToEndpoint,
				BaseNode* pConnectedFrom,
				EndpointIdentifier connectedFromEndpoint) const
			{
				std::unordered_map<BaseNode*, NodesPack>::const_iterator to = Nodes.find(pConnectedTo);
				std::unordered_map<BaseNode*, NodesPack>::const_iterator from = Nodes.find(pConnectedFrom);

				// Ensure both nodes exist in the context
				assert(to != Nodes.end() && "pConnectedTo node is not found in Nodes."); assert(from != Nodes.end() && "pConnectedFrom node is not found in Nodes.");

				const std::vector<Connection>& connections = to->second.Connections;

				auto connection = std::find_if(connections.begin(), connections.end(), [pConnectedTo, pConnectedFrom, connectedToEndpoint, connectedFromEndpoint](const Connection& connection)
					{
						return (connection.PConnectedTo == pConnectedTo &&
							connection.PConnectedFrom == pConnectedFrom &&
							connection.ConnectedToEndpoint == connectedToEndpoint &&
							connection.ConnectedFromEndpoint == connectedFromEndpoint);
					});

				return (connection != connections.end()) ? &(*connection) : nullptr; // Return the found connection or nullptr
			}

			/// @brief Gets the internal node for a given parrent node.
			/// @param BaseNode* pParrent pointer to the parrent node.
			/// @param NodeIdentifier identifier of internal node.
			/// @return A pointer to the internal node, or nullptr if none exist.
			SHADE_API BaseNode* FindInternalNode(BaseNode* pParrent, NodeIdentifier identifier);

			/// @brief Gets the node.
			/// @param NodeIdentifier identifier of node.
			/// @return A pointer to the node, or nullptr if none exist.
			SHADE_API BaseNode* FindNode(NodeIdentifier identifier);

			/// @brief Gets the connections for a given node.
			/// @param pConnectedTo Pointer to the node whose connections are to be retrieved.
			/// @return A pointer to the vector of connections for the node, or nullptr if none exist.
			SHADE_INLINE const std::vector<Connection>* GetConnections(const BaseNode* pConnectedTo) const
			{
				std::unordered_map<BaseNode*, NodesPack>::const_iterator to = Nodes.find(const_cast<BaseNode*>(pConnectedTo));
				return (to != Nodes.end()) ? &to->second.Connections : nullptr; // Return the connections or nullptr
			}

			/// @brief Gets the connections for a given node.
			/// @param pConnectedTo Pointer to the node whose connections are to be retrieved.
			/// @return A pointer to the vector of connections for the node, or nullptr if none exist.
			SHADE_INLINE std::vector<Connection>* GetConnections(BaseNode* pConnectedTo)
			{
				std::unordered_map<BaseNode*, NodesPack>::iterator to = Nodes.find(pConnectedTo);
				return (to != Nodes.end()) ? &to->second.Connections : nullptr; // Return the connections or nullptr
			}

			/// @brief Checks if an endpoint on a node is free (not connected).
			/// @param pConnectedTo Pointer to the node to check.
			/// @param connectedToEndpoint The endpoint identifier to check.
			/// @return True if the endpoint is free, otherwise false.
			SHADE_INLINE bool IsEndpointFree(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint) const
			{
				std::unordered_map<BaseNode*, NodesPack>::const_iterator to = Nodes.find(pConnectedTo);

				if (to != Nodes.end())
				{
					const std::vector<Connection>& connections = to->second.Connections;

					auto connection = std::find_if(connections.begin(), connections.end(), [pConnectedTo, connectedToEndpoint](const Connection& connection)
						{
							return (connection.ConnectedToEndpoint == connectedToEndpoint);
						});

					return connection == connections.end(); // Return true if no connection exists for the endpoint
				}

				return false; // Return false if no connections were found
			}

			/// @brief Connects two nodes within the graph context.
			/// @param pConnectedTo Pointer to the destination node.
			/// @param connectedToEndpoint The endpoint identifier for the destination node.
			/// @param pConnectedFrom Pointer to the source node.
			/// @param connectedFromEndpoint The endpoint identifier for the source node.
			/// @return True if the nodes were successfully connected, otherwise false.
			SHADE_API bool ConnectNodes(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint);

			/// @brief Disconnects two nodes within the graph context.
			/// @param pConnectedTo Pointer to the destination node.
			/// @param connectedToEndpoint The endpoint identifier for the destination node.
			/// @param pConnectedFrom Pointer to the source node.
			/// @param connectedFromEndpoint The endpoint identifier for the source node.
			/// @return True if the nodes were successfully disconnected, otherwise false.
			SHADE_API bool DisconnectNodes(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint);

			/// @brief Connects the values between two node endpoints.
			/// @param inputEndpoint Pointer to the input endpoint.
			/// @param outputEndpoint Pointer to the output endpoint.
			/// @return True if the values were successfully connected, otherwise false.
			SHADE_INLINE bool ConnectValues(std::shared_ptr<NodeValue>* pConnectedToEndpoint, std::shared_ptr<NodeValue>* pConnectedFromEndpoint)
			{
				if (!pConnectedToEndpoint || !pConnectedFromEndpoint)
					return false;

				*pConnectedToEndpoint = *pConnectedFromEndpoint;

				return true; // Return true if the connection was successful
			}

			/// @brief Creates a connection between two nodes.
			/// @param pConnectedTo Pointer to the destination node.
			/// @param connectedToEndpoint The endpoint identifier for the destination node.
			/// @param pConnectedFrom Pointer to the source node.
			/// @param connectedFromEndpoint The endpoint identifier for the source node.
			/// @return True if the connection was successfully created, otherwise false.
			SHADE_INLINE bool CreateConnection(
				BaseNode* pConnectedTo,
				EndpointIdentifier connectedToEndpoint,
				BaseNode* pConnectedFrom,
				EndpointIdentifier connectedFromEndpoint)
			{
				if (IsEndpointFree(pConnectedTo, connectedToEndpoint) && !FindConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint))
				{
					Nodes.at(pConnectedTo).Connections.emplace_back(Connection{ pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint });
					return true; // Return true if the connection was created
				}

				return false; // Return false if the connection could not be created
			}

			/// @brief Removes a connection between two nodes.
			/// @param pConnectedTo Pointer to the destination node.
			/// @param connectedToEndpoint The endpoint identifier for the destination node.
			/// @param pConnectedFrom Pointer to the source node.
			/// @param connectedFromEndpoint The endpoint identifier for the source node.
			/// @return True if the connection was successfully removed, otherwise false.
			SHADE_INLINE bool RemoveConnection(
				BaseNode* pConnectedTo,
				EndpointIdentifier connectedToEndpoint,
				BaseNode* pConnectedFrom,
				EndpointIdentifier connectedFromEndpoint)
			{
				std::unordered_map<BaseNode*, NodesPack>::iterator to = Nodes.find(pConnectedTo);
				// Ensure the node exists in the Nodes map
				assert(to != Nodes.end() && "pConnectedTo node is not found in Nodes.");

				std::vector<Connection>& connections = to->second.Connections;

				auto connection = std::find_if(connections.begin(), connections.end(), [pConnectedTo, pConnectedFrom, connectedToEndpoint, connectedFromEndpoint](const Connection& connection)
					{
						return (connection.PConnectedTo == pConnectedTo &&
							connection.PConnectedFrom == pConnectedFrom &&
							connection.ConnectedToEndpoint == connectedToEndpoint &&
							connection.ConnectedFromEndpoint == connectedFromEndpoint);
					});

				if (connection != connections.end())
				{
					connections.erase(connection); // Remove the connection if found
					return true; // Return true if the connection was removed
				}

				return false; // Return false if the connection was not found
			}

			/// @brief Retrieves the internal nodes of a given node.
			/// @param pNode Pointer to the node.
			/// @return A reference to a vector of internal nodes.
			SHADE_INLINE std::vector<BaseNode*>& GetInternalNodes(BaseNode* pNode)
			{
				return Nodes.at(pNode).InternalNodes; // Return the internal nodes of the node
			}

			/// @brief Retrieves the internal nodes of a given node.
			/// @param pNode Pointer to the node.
			/// @return A reference to a constant vector of internal nodes.
			SHADE_INLINE const std::vector<BaseNode*>& GetInternalNodes(const BaseNode* pNode) const
			{
				return Nodes.at(const_cast<BaseNode*>(pNode)).InternalNodes; // Return the internal nodes of the node
			}

			/// @brief Removes all input connections for a given node.
			/// @param pConnectedTo Pointer to the node whose input connections are to be removed.
			/// @return True if the input connections were successfully removed, otherwise false.
			SHADE_API bool RemoveAllInputConnection(BaseNode* pConnectedTo);

			/// @brief Removes all output connections for a given node.
			/// @param pConnectedFrom Pointer to the node whose output connections are to be removed.
			/// @return True if the output connections were successfully removed, otherwise false.
			SHADE_API bool RemoveAllOutputConnection(BaseNode* pConnectedFrom);

			/// @brief Removes all connections for a given node.
			/// @param pNode Pointer to the node whose connections are to be removed.
			/// @return True if all connections were successfully removed, otherwise false.
			SHADE_INLINE bool RemoveAllConnection(BaseNode* pNode)
			{
				return (RemoveAllOutputConnection(pNode) | RemoveAllInputConnection(pNode)); // Remove all connections for the node
			}

			/// @brief Retrieves all nodes and their connections in the graph context.
			/// @return A reference to an unordered map of nodes and their connections.
			SHADE_INLINE const std::unordered_map<BaseNode*, NodesPack>& GetNodes() const
			{
				return Nodes; // Return the map of nodes and their connections
			}

			/// @brief Retrieves all nodes and their connections in the graph context.
			/// @return A reference to an unordered map of nodes and their connections.
			SHADE_INLINE std::unordered_map<BaseNode*, NodesPack>& GetNodes()
			{
				return Nodes; // Return the map of nodes and their connections
			}

		private:
			/// @brief Initializes a node within the graph context.
			/// @param pNode Pointer to the node to initialize.
			/// @param pParrent Pointer to the parent node.
			/// @return None.
			SHADE_API void InitializeNode(BaseNode* pNode, BaseNode* pParrent);

			/// @brief Contains all nodes and their connections within the graph context.
			std::unordered_map<BaseNode*, NodesPack> Nodes;

			/// @brief Serializes node's connections to the given output stream.
			/// @param stream The output stream to serialize to.
			/// @return The number of bytes written.
			std::size_t Serialize(std::ostream& stream) const;

			/// @brief Deserializes node's connections from the given input stream.
			/// @param stream The input stream to deserialize from.
			/// @return The number of bytes read.
			std::size_t Deserialize(std::istream& stream);

			friend class Serializer;
		};
	}

	// Serialize GraphContext
	template<>
	SHADE_INLINE std::size_t shade::Serializer::Serialize(std::ostream& stream, const graphs::GraphContext& context, std::size_t)
	{
		return context.Serialize(stream);
	}

	// Deserialize GraphContext
	template<>
	SHADE_INLINE std::size_t shade::Serializer::Deserialize(std::istream& stream, graphs::GraphContext& context, std::size_t)
	{
		return context.Deserialize(stream);
	}
}