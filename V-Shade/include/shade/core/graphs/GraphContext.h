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
			BaseNode*				   InputNode = nullptr;
			EndpointIdentifier         InputEndpoint = INVALID_NODE_IDENTIFIER;

			BaseNode*				   OutputNode = nullptr;
			EndpointIdentifier         OutputEndpoint = INVALID_NODE_IDENTIFIER;

			Type					   ConnectionType = MAX_ENUM;

			// Uses only for editor
			glm::vec2 InputScreenPosition	= glm::vec2(0.f);
			glm::vec2 OutputScreenPosition	= glm::vec2(0.f);
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
			///@brief Connections
			std::unordered_map<BaseNode*, std::vector<Connection>> Connections;

			SHADE_INLINE bool IsEndpointFree(BaseNode* firstNode, EndpointIdentifier firstEndpoint,
				BaseNode* secondNode, EndpointIdentifier secondEndpoint, Connection::Type type) const
			{
				const auto node = Connections.find(firstNode);

				if (node != Connections.end())
				{
					auto connection = std::find_if(
						node->second.begin(),
						node->second.end(),
						[secondNode, firstEndpoint](const Connection& connection)
						{
							return (connection.InputEndpoint != firstEndpoint);
						});

					return connection != node->second.end();
				}
				else
				{
					return true;
				}
			}

			SHADE_INLINE bool IsConnectionExist(BaseNode* firstNode, EndpointIdentifier firstEndpoint, 
				BaseNode* secondNode, EndpointIdentifier secondEndpoint, Connection::Type type) const 
			{
				const auto node = Connections.find(firstNode);

				if (node != Connections.end())
				{
					auto connection = std::find_if(node->second.begin(), node->second.end(), [secondNode, secondEndpoint](const Connection& connection)
						{
							return (connection.OutputNode == secondNode && connection.OutputEndpoint == secondEndpoint);
						});

					return connection != node->second.end();
				}
				else
				{
					return false;
				}	
			}

			SHADE_INLINE bool AddConnection(BaseNode* firstNode, EndpointIdentifier firstEndpoint,
				BaseNode* secondNode, EndpointIdentifier secondEndpoint, Connection::Type type)
			{
				if (IsEndpointFree(firstNode, firstEndpoint, secondNode, secondEndpoint, type) && !IsConnectionExist(firstNode, firstEndpoint, secondNode, secondEndpoint, type))
				{
					auto connection = Connections.find(firstNode);
					if (connection != Connections.end())
						connection->second.emplace_back(firstNode, firstEndpoint, secondNode, secondEndpoint, type);
					else
						Connections[firstNode].emplace_back(firstNode, firstEndpoint, secondNode, secondEndpoint, type);

					return true;
				}

				return false;

			}

			SHADE_INLINE bool RemoveConnection(BaseNode* firstNode, EndpointIdentifier firstEndpoint,
				BaseNode* secondNode, EndpointIdentifier secondEndpoint) // Private ??
			{
				const auto node = Connections.find(firstNode);

				if (node != Connections.end())
				{
					auto connection = std::find_if(node->second.begin(), node->second.end(), [secondNode, secondEndpoint](const Connection& connection)
						{
							return (connection.OutputNode == secondNode && connection.OutputEndpoint == secondEndpoint);
						});

					if (connection != node->second.end())
					{
						node->second.erase(connection); return true;
					}
				}

				return false;
			}

			SHADE_API bool RemoveAllInputConnection(BaseNode* firstNode);
			
			SHADE_API bool RemoveAllOutputConnection(BaseNode* firstNode);
			
			SHADE_INLINE bool RemoveAllConnection(BaseNode* firstNode)
			{
				return (RemoveAllOutputConnection(firstNode) | RemoveAllInputConnection(firstNode));
			}

			SHADE_INLINE Connection FindConnection(BaseNode* firstNode, EndpointIdentifier firstEndpoint,
				BaseNode* secondNode, EndpointIdentifier secondEndpoint) const
			{
				const auto node = Connections.find(firstNode);

				if (node != Connections.end())
				{
					auto connection = std::find_if(node->second.begin(), node->second.end(), [secondNode, secondEndpoint](const Connection& connection)
						{
							return (connection.OutputNode == secondNode && connection.OutputEndpoint == secondEndpoint);
						});

					return *connection;
				}
				
				return Connection();
			}
		};
	}
}