//#include "shade_pch.h"
//#include "GraphContext.h"
//#include <shade/core/graphs/nodes/BaseNode.h>
//
//SHADE_API void shade::graphs::GraphContext::InitializeNode(BaseNode* pNode, BaseNode* pParrent)
//{
//	Nodes.emplace(pNode, NodesPack{}).first->first->Initialize();
//}
//
//SHADE_API bool shade::graphs::GraphContext::RemoveNode(BaseNode* pNode)
//{
//	std::unordered_map<BaseNode*, NodesPack>::iterator node = Nodes.find(pNode);
//
//	assert(node != Nodes.end() && "");
//
//	RemoveAllConnection(pNode);
//
//	for (BaseNode* internalNode : GetInternalNodes(pNode))
//	{
//		RemoveNode(internalNode);
//	}
//
//	BaseNode* pParrent = pNode->GetParrentGraph();
//
//	if (pParrent != nullptr)
//	{
//		std::unordered_map<BaseNode*, NodesPack>::iterator parrent = Nodes.find(pParrent);
//		assert(parrent != Nodes.end() && "");
//
//		std::vector<BaseNode*>& internalNodes = parrent->second.InternalNodes;
//
//		auto asInternalNode = std::find_if(internalNodes.begin(), internalNodes.end(), [pNode](const BaseNode* node)
//			{
//				return node == pNode;
//			});
//
//		internalNodes.erase(asInternalNode); // Need to remove them recursevly ?
//	}
//
//	pNode->Shutdown(); SDELETE pNode; Nodes.erase(node);
//
//	return true;
//}
//
//bool shade::graphs::GraphContext::ConnectNodes(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint)
//{
//	if (pConnectedTo == pConnectedFrom) return false;
//
//	auto inputValue = pConnectedTo->__GET_ENDPOINT<Connection::Input>(connectedToEndpoint);
//	auto outputValue = pConnectedFrom->__GET_ENDPOINT<Connection::Output>(connectedFromEndpoint);
//
//	if (!inputValue || !outputValue) return false;
//
//	if (inputValue->get()->GetType() != outputValue->get()->GetType()) return false;
//
//
//	if (CreateConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint))
//	{
//		if (ConnectValues(inputValue, outputValue))
//		{
//			pConnectedTo->OnConnect(Connection::Type::Input, inputValue->get()->GetType(), connectedToEndpoint);
//			pConnectedFrom->OnConnect(Connection::Type::Output, outputValue->get()->GetType(), connectedFromEndpoint);
//
//			return true;
//		}
//	}
//	return false;
//}
//
//bool shade::graphs::GraphContext::DisconnectNodes(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint)
//{
//	auto connection = FindConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint);
//
//	if (connection != nullptr && connection->ConnectedToEndpoint != INVALID_NODE_IDENTIFIER)
//	{
//		NodeValues::Value* connectedToValue = pConnectedTo->__GET_ENDPOINT<Connection::Input>(connectedToEndpoint);
//		NodeValues::Value* connectedFromValue = pConnectedFrom->__GET_ENDPOINT<Connection::Output>(connectedFromEndpoint);
//
//		if (!connectedToValue || !connectedFromValue) return false;
//
//		if (RemoveConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint))
//		{
//			pConnectedTo->OnDisconnect(Connection::Type::Input, connectedToValue->get()->GetType(), connectedToEndpoint);
//			pConnectedFrom->OnDisconnect(Connection::Type::Output, connectedFromValue->get()->GetType(), connectedFromEndpoint);
//
//			return true;
//		}
//	}
//
//	return false;
//}
//
//bool shade::graphs::GraphContext::RemoveAllInputConnection(BaseNode* pConnectedTo)
//{
//	bool wasRemoved = false;
//
//	if (std::vector<Connection>* pConnections = GetConnections(pConnectedTo))
//	{
//		std::vector<Connection> connections = *pConnections;
//
//		for (Connection& connection : connections)
//		{
//			DisconnectNodes(
//				connection.PConnectedTo,
//				connection.ConnectedToEndpoint,
//				connection.PConnectedFrom,
//				connection.ConnectedFromEndpoint
//			);
//
//			wasRemoved = true;
//		}
//	}
//	return wasRemoved;
//}
//
//
//bool shade::graphs::GraphContext::RemoveAllOutputConnection(BaseNode* pConnectedFrom)
//{
//	bool wasRemoved = false;
//
//	for (auto& [node, pack] : Nodes)
//	{
//		// Keep as copy
//		std::vector<Connection> connections = pack.Connections;
//
//		for (Connection& connection : connections)
//		{
//			if (connection.PConnectedFrom == pConnectedFrom)
//			{
//				DisconnectNodes(
//					connection.PConnectedTo,
//					connection.ConnectedToEndpoint,
//					connection.PConnectedFrom,
//					connection.ConnectedFromEndpoint
//				);
//				wasRemoved = true;
//			}
//		}
//	}
//
//	return wasRemoved;
//}

#include "shade_pch.h"
#include "GraphContext.h"
#include <shade/core/graphs/nodes/BaseNode.h>

void shade::graphs::GraphContext::InitializeNode(BaseNode* pNode, BaseNode* pParrent)
{
	// Add the node to the Nodes map and initialize it.
	Nodes.emplace(pNode, NodesPack{}).first->first->Initialize();
}

bool shade::graphs::GraphContext::RemoveNode(BaseNode* pNode)
{
	// Find the node in the Nodes map.
	std::unordered_map<BaseNode*, NodesPack>::iterator node = Nodes.find(pNode);

	// Ensure the node exists.
	assert(node != Nodes.end() && "Node not found in the Nodes map.");

	// Remove all connections associated with the node.
	RemoveAllConnection(pNode);

	// Recursively remove all internal nodes associated with the node.
	for (BaseNode* internalNode : GetInternalNodes(pNode))
	{
		RemoveNode(internalNode);
	}

	// Get the parent of the node.
	BaseNode* pParrent = pNode->GetParrentGraph();

	if (pParrent != nullptr)
	{
		// Find the parent node in the Nodes map.
		std::unordered_map<BaseNode*, NodesPack>::iterator parrent = Nodes.find(pParrent);
		assert(parrent != Nodes.end() && "Parent node not found in the Nodes map.");

		// Get the internal nodes of the parent.
		std::vector<BaseNode*>& internalNodes = parrent->second.InternalNodes;

		// Remove the node from the parent's internal nodes.
		auto asInternalNode = std::find_if(internalNodes.begin(), internalNodes.end(), [pNode](const BaseNode* node)
			{
				return node == pNode;
			});

		internalNodes.erase(asInternalNode); // Remove the node from the parent's list of internal nodes.
	}

	// Shutdown the node and deallocate its memory.
	pNode->Shutdown();
	SDELETE pNode;
	Nodes.erase(node);

	return true;
}

shade::graphs::BaseNode* shade::graphs::GraphContext::FindInternalNode(BaseNode* pParrent, NodeIdentifier identifier)
{
	std::unordered_map<BaseNode*, NodesPack>::iterator parrent = Nodes.find(pParrent);

	assert(parrent != Nodes.end() && "");

	std::vector<BaseNode*>& internalNodes = (*parrent).second.InternalNodes;

	std::vector<BaseNode*>::iterator node = std::find_if(internalNodes.begin(), internalNodes.end(), [identifier](const BaseNode* pNode)
		{
			return identifier == pNode->GetNodeIdentifier();
		});

	return (node != internalNodes.end()) ? *node : nullptr;

}

 shade::graphs::BaseNode* shade::graphs::GraphContext::FindNode(NodeIdentifier identifier)
{
	auto node = std::find_if(Nodes.begin(), Nodes.end(), [identifier](const std::pair<BaseNode*, NodesPack>& pack)
		{
			return identifier == pack.first->GetNodeIdentifier();
		});

	return (node != Nodes.end()) ? node->first : nullptr;
}

bool shade::graphs::GraphContext::ConnectNodes(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint)
{
	// Ensure the source and destination nodes are not the same.
	if (pConnectedTo == pConnectedFrom) return false;

	// Get the input and output values for the specified endpoints.
	auto inputValue = pConnectedTo->__GET_ENDPOINT<Connection::Input>(connectedToEndpoint);
	auto outputValue = pConnectedFrom->__GET_ENDPOINT<Connection::Output>(connectedFromEndpoint);

	// Ensure both input and output values are valid.
	if (!inputValue || !outputValue) return false;

	// Ensure the types of the input and output values match.
	if (inputValue->get()->GetType() != outputValue->get()->GetType()) return false;

	// Create the connection between the nodes.
	if (CreateConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint))
	{
		// Connect the values and notify the nodes of the connection.
		if (ConnectValues(inputValue, outputValue))
		{
			pConnectedTo->OnConnect(Connection::Type::Input, inputValue->get()->GetType(), connectedToEndpoint);
			pConnectedFrom->OnConnect(Connection::Type::Output, outputValue->get()->GetType(), connectedFromEndpoint);

			return true;
		}
	}
	return false;
}

bool shade::graphs::GraphContext::DisconnectNodes(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint, BaseNode* pConnectedFrom, EndpointIdentifier connectedFromEndpoint)
{
	// Find the connection between the specified nodes and endpoints.
	auto connection = FindConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint);

	// Ensure the connection is valid and not to an invalid endpoint.
	if (connection != nullptr && connection->ConnectedToEndpoint != INVALID_NODE_IDENTIFIER)
	{
		NodeValues::Value* connectedToValue = pConnectedTo->__GET_ENDPOINT<Connection::Input>(connectedToEndpoint);
		NodeValues::Value* connectedFromValue = pConnectedFrom->__GET_ENDPOINT<Connection::Output>(connectedFromEndpoint);

		// Ensure both input and output values are valid.
		if (!connectedToValue || !connectedFromValue) return false;

		// Remove the connection and notify the nodes of the disconnection.
		if (RemoveConnection(pConnectedTo, connectedToEndpoint, pConnectedFrom, connectedFromEndpoint))
		{
			pConnectedTo->OnDisconnect(Connection::Type::Input, connectedToValue->get()->GetType(), connectedToEndpoint);
			pConnectedFrom->OnDisconnect(Connection::Type::Output, connectedFromValue->get()->GetType(), connectedFromEndpoint);

			return true;
		}
	}

	return false;
}

bool shade::graphs::GraphContext::RemoveAllInputConnection(BaseNode* pConnectedTo)
{
	bool wasRemoved = false;

	// Get all connections for the node.
	if (std::vector<Connection>* pConnections = GetConnections(pConnectedTo))
	{
		std::vector<Connection> connections = *pConnections;

		// Disconnect each connection.
		for (Connection& connection : connections)
		{
			DisconnectNodes(
				connection.PConnectedTo,
				connection.ConnectedToEndpoint,
				connection.PConnectedFrom,
				connection.ConnectedFromEndpoint
			);

			wasRemoved = true;
		}
	}
	return wasRemoved;
}

bool shade::graphs::GraphContext::RemoveAllOutputConnection(BaseNode* pConnectedFrom)
{
	bool wasRemoved = false;

	// Iterate over all nodes in the graph context.
	for (auto& [node, pack] : Nodes)
	{
		// Keep a copy of the connections.
		std::vector<Connection> connections = pack.Connections;

		// Disconnect connections where the node is the source.
		for (Connection& connection : connections)
		{
			if (connection.PConnectedFrom == pConnectedFrom)
			{
				DisconnectNodes(
					connection.PConnectedTo,
					connection.ConnectedToEndpoint,
					connection.PConnectedFrom,
					connection.ConnectedFromEndpoint
				);
				wasRemoved = true;
			}
		}
	}

	return wasRemoved;
}

std::size_t shade::graphs::GraphContext::Serialize(std::ostream& stream) const
{
	// Serialize count fo nodes
	std::size_t size = Serializer::Serialize(stream, std::uint32_t(Nodes.size()));

	for (const auto& [node, pack] : Nodes)
	{
		// Serialize connected to node identifier
		size += Serializer::Serialize(stream, node->GetNodeIdentifier());

		// Serialize connections count
		size += Serializer::Serialize(stream, std::uint32_t(pack.Connections.size()));

		// Serialize node connection
		for (const Connection& connection : pack.Connections)
		{
			size += Serializer::Serialize(stream, connection.ConnectedToEndpoint);
			size += Serializer::Serialize(stream, connection.PConnectedFrom->GetNodeIdentifier());
			size += Serializer::Serialize(stream, connection.ConnectedFromEndpoint);
		}
	}
	
	return size;
}

std::size_t shade::graphs::GraphContext::Deserialize(std::istream& stream)
{
	// Deserialize count fo nodes
	std::uint32_t nodeCount; std::size_t size = Serializer::Deserialize(stream, nodeCount);
	
	for (std::size_t i = 0; i < nodeCount; ++i)
	{
		// Deserialize connected to node identifier
		NodeIdentifier connectedToIdentifier;		size += Serializer::Deserialize(stream, connectedToIdentifier);

		std::uint32_t connectionCount; size += Serializer::Deserialize(stream, connectionCount);
		
		for (std::uint32_t i = 0; i < connectionCount; ++i)
		{
			// Deserialize node connection
			EndpointIdentifier connectedToEndpoint;		size += Serializer::Deserialize(stream, connectedToEndpoint);
			NodeIdentifier connectedFromIdentifier;		size += Serializer::Deserialize(stream, connectedFromIdentifier);
			EndpointIdentifier connectedFromEndpoint;	size += Serializer::Deserialize(stream, connectedFromEndpoint);

			// Connect nodes
			ConnectNodes(FindNode(connectedToIdentifier), connectedToEndpoint, FindNode(connectedFromIdentifier), connectedFromEndpoint);
		}
	}

	return size;
}