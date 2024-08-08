#pragma once
#include <shade/utils/Utils.h>
#include <shade/core/entity/Entity.h>

namespace shade
{
	namespace graphs
	{
		class BaseNode;

		/// @brief Type aliases for node and endpoint indices
		using NodeIdentifier = std::uint32_t;
		using EndpointIdentifier = std::uint32_t;

		/// @brief Constant for representing a null node index
		static constexpr NodeIdentifier ROOT_NODE_IDENTIFIER = 0;
		static constexpr NodeIdentifier INVALID_NODE_IDENTIFIER = ~0;

		/// @brief Structure representing a connection type within a graph node
		struct Connection
		{
			enum Type : std::uint8_t
			{
				Input,
				Output,
				MAX_ENUM
			};

			// TODO Make Constructor !!
			BaseNode*				   PConnectedTo = nullptr;
			EndpointIdentifier         ConnectedToEndpoint = INVALID_NODE_IDENTIFIER;

			BaseNode*				   PConnectedFrom = nullptr;
			EndpointIdentifier         ConnectedFromEndpoint = INVALID_NODE_IDENTIFIER;

			// Uses only for editor
			glm::vec2 ConnectedToPosition	= glm::vec2(0.f);
			glm::vec2 ConnectedFromPosition = glm::vec2(0.f);
		};

		struct NodesPack
		{
			std::vector<Connection> Connections;
			std::vector<BaseNode*>  InternalNodes;
		};

		/// @brief Structure representing the context of a graph
		struct GraphContext
		{
			SHADE_CAST_HELPER(GraphContext)
		public:
			///@brief Current entity
			ecs::Entity Entity;

			template<typename T, typename... Args>
			SHADE_INLINE T* CreateNode(BaseNode* pParrent, Args&&... args)
			{
				std::unordered_map<BaseNode*, NodesPack>::iterator parrent = Nodes.find(pParrent);

				const NodeIdentifier id = (parrent != Nodes.end()) ? parrent->second.InternalNodes.size() : 0;
			
				T* node = SNEW T(this, id, pParrent, std::forward<Args>(args)...);

				if (parrent != Nodes.end()) { parrent->second.InternalNodes.emplace_back(node); }

				InitializeNode(node, pParrent);
				
				return node;
			}
		
			/*SHADE_INLINE BaseNode* FindInternalNode(BaseNode* pParrent, NodeIdentifier identifier)
			{
				std::unordered_map<BaseNode*, NodesPack>::iterator parrent = Nodes.find(pParrent);

				assert(parrent != Nodes.end() && "");

				std::vector<BaseNode*>& internalNodes = (*parrent).second.InternalNodes;

				std::vector<BaseNode*>::iterator node = std::find_if(internalNodes.begin(), internalNodes.end(), [identifier](const BaseNode* pNode)
					{
						return identifier == pNode->GetNodeIdentifier();
					});
				
				return (node != internalNodes.end()) ? *node : nullptr;
			}*/

			SHADE_API bool RemoveNode(BaseNode* pNode);

			SHADE_INLINE const Connection* FindConnection(
				BaseNode* pConnectedTo,
				EndpointIdentifier connectedToEndpoint,
				BaseNode* pConnectedFrom,
				EndpointIdentifier connectedFromEndpoint) const
			{
				std::unordered_map<BaseNode*, NodesPack>::const_iterator to		= Nodes.find(pConnectedTo);
				std::unordered_map<BaseNode*, NodesPack>::const_iterator from	= Nodes.find(pConnectedFrom);
				// TODO Assert and do not check 

				assert(to != Nodes.end() && from != Nodes.end() && "");

				const std::vector<Connection>& connections = to->second.Connections;

				auto connection = std::find_if(connections.begin(), connections.end(), [pConnectedTo, pConnectedFrom, connectedToEndpoint, connectedFromEndpoint](const Connection& connection)
					{
						return (connection.PConnectedTo == pConnectedTo &&
							connection.PConnectedFrom == pConnectedFrom &&
							connection.ConnectedToEndpoint == connectedToEndpoint &&
							connection.ConnectedFromEndpoint == connectedFromEndpoint);
					});

				return (connection != connections.end()) ? &(*connection) : nullptr;
			} 

			SHADE_INLINE const std::vector<Connection>* GetConnections(const BaseNode* pConnectedTo) const
			{
				std::unordered_map<BaseNode*, NodesPack>::const_iterator to = Nodes.find(const_cast<BaseNode*>(pConnectedTo));
				return (to != Nodes.end()) ? &to->second.Connections : nullptr;
			}

			SHADE_INLINE std::vector<Connection>* GetConnections(BaseNode* pConnectedTo)
			{
				std::unordered_map<BaseNode*, NodesPack>::iterator to = Nodes.find(pConnectedTo);
				return (to != Nodes.end()) ? &to->second.Connections : nullptr;
			}

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

					return connection == connections.end();
				}

				return false;
			}

			SHADE_INLINE bool CreateConnection(
				BaseNode* pConnectedTo,
				EndpointIdentifier connectedToEndpoint,
				BaseNode* pConnectedFrom,
				EndpointIdentifier connectedFromEndpoint)
			{
				// That means you cannot conenect same node twice !
				// If you want then need to check if endpoint free !
				if (IsEndpointFree(pConnectedTo, connectedToEndpoint) && !FindConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint))
				{
					std::unordered_map<BaseNode*, NodesPack>::iterator to = Nodes.find(pConnectedTo);
					to->second.Connections.emplace_back(Connection{pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint });
					return true;
				}

				return false;
			}
			SHADE_INLINE bool RemoveConnection(
				BaseNode* pConnectedTo,
				EndpointIdentifier connectedToEndpoint,
				BaseNode* pConnectedFrom,
				EndpointIdentifier connectedFromEndpoint)
			{
				std::unordered_map<BaseNode*, NodesPack>::iterator to   = Nodes.find(pConnectedTo);
				// TODO Assert and do not check 

				assert(to != Nodes.end() && "");

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
					connections.erase(connection, connections.end());
					return true;
				}

				return false;
			}


			SHADE_INLINE std::vector<BaseNode*>& GetInternalNodes(BaseNode* pNode)
			{
				return Nodes.at(pNode).InternalNodes;
			}

			SHADE_INLINE const std::vector<BaseNode*>& GetInternalNodes(const BaseNode* pNode) const
			{
				return Nodes.at(const_cast<BaseNode*>(pNode)).InternalNodes;
			}

			SHADE_API bool RemoveAllInputConnection(BaseNode* pConnectedTo);
			
			SHADE_API bool RemoveAllOutputConnection(BaseNode* pConnectedFrom);
			
			SHADE_INLINE bool RemoveAllConnection(BaseNode* pNode)
			{
				return (RemoveAllOutputConnection(pNode) | RemoveAllInputConnection(pNode));
			}

			SHADE_INLINE const std::unordered_map<BaseNode*, NodesPack>& GetNodes() const
			{
				return Nodes;
			}
			SHADE_INLINE  std::unordered_map<BaseNode*, NodesPack>& GetNodes()
			{
				return Nodes;
			}
		private:
			SHADE_API void InitializeNode(BaseNode* pNode, BaseNode* pParrent);
			///@brief Connections
			std::unordered_map<BaseNode*, NodesPack>	Nodes;
		};
	}
}