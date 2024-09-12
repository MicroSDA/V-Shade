#include "shade_pch.h"
#include "GraphContext.h"
#include <shade/core/graphs/nodes/BaseNode.h>

void shade::graphs::GraphContext::InitializeNode(BaseNode* pNode, BaseNode* pParrent)
{
	// Add the node to the Nodes map and initialize it.
	Nodes.emplace(pNode, NodesPack{}).first->first->Initialize();
}

shade::graphs::GraphContext::~GraphContext()
{
	Drop();
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

void shade::graphs::GraphContext::RemoveAll()
{
	for (auto [pNode, pack] : Nodes)
	{
		pNode->Shutdown();
		SDELETE pNode;
	}
	Nodes.clear(); pRoot = nullptr;
	
}

void shade::graphs::GraphContext::Drop()
{
	for (auto node = Nodes.begin(); node != Nodes.end();)
	{
		if (node->first != pRoot)
		{
			node->first->Shutdown();
			SDELETE node->first;

			node = Nodes.erase(node);
		}
		else
		{
			node->first->Shutdown();
			node = Nodes.erase(node);
		}
	}

	pRoot = nullptr;
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

bool shade::graphs::GraphContext::DisconnectEndpoint(BaseNode* pConnectedTo, EndpointIdentifier connectedToEndpoint)
{
	// Find the connection between the specified nodes and endpoints.
	auto connection = FindConnection(pConnectedTo, connectedToEndpoint);

	// Ensure the connection is valid and not to an invalid endpoint.
	if (connection != nullptr && connection->ConnectedToEndpoint != INVALID_NODE_IDENTIFIER)
	{
		NodeValues::Value* connectedToValue		= pConnectedTo->__GET_ENDPOINT<Connection::Input>(connectedToEndpoint);
		NodeValues::Value* connectedFromValue	= connection->PConnectedFrom->__GET_ENDPOINT<Connection::Output>(connection->ConnectedFromEndpoint);

		// Ensure both input and output values are valid.
		if (!connectedToValue || !connectedFromValue) return false;

		// Remove the connection and notify the nodes of the disconnection.
		if (RemoveConnection(pConnectedTo, connectedToEndpoint, connection->PConnectedFrom, connection->ConnectedFromEndpoint))
		{
			pConnectedTo->OnDisconnect(Connection::Type::Input, connectedToValue->get()->GetType(), connectedToEndpoint);
			connection->PConnectedFrom->OnDisconnect(Connection::Type::Output, connectedFromValue->get()->GetType(), connection->ConnectedFromEndpoint);

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

void shade::graphs::GraphContext::Serialize(std::ostream& stream) const
{
	// Serialize count fo nodes
	serialize::Serializer::Serialize(stream, std::uint32_t(Nodes.size()));

	for (const auto& [node, pack] : Nodes)
	{
		// Serialize connected to node identifier
		serialize::Serializer::Serialize(stream, node->GetNodeIdentifier());

		// Serialize connections count
		serialize::Serializer::Serialize(stream, std::uint32_t(pack.Connections.size()));

		// Serialize node connection
		for (const Connection& connection : pack.Connections)
		{
			serialize::Serializer::Serialize(stream, connection.ConnectedToEndpoint);
			serialize::Serializer::Serialize(stream, connection.PConnectedFrom->GetNodeIdentifier());
			serialize::Serializer::Serialize(stream, connection.ConnectedFromEndpoint);
		}
	}
}

void shade::graphs::GraphContext::Deserialize(std::istream& stream)
{
	// Deserialize count fo nodes
	std::uint32_t nodeCount; serialize::Serializer::Deserialize(stream, nodeCount);
	
	for (std::size_t i = 0; i < nodeCount; ++i)
	{
		// Deserialize connected to node identifier
		NodeIdentifier connectedToIdentifier; serialize::Serializer::Deserialize(stream, connectedToIdentifier);

		std::uint32_t connectionCount; serialize::Serializer::Deserialize(stream, connectionCount);
		
		for (std::uint32_t i = 0; i < connectionCount; ++i)
		{
			// Deserialize node connection
			EndpointIdentifier connectedToEndpoint;		serialize::Serializer::Deserialize(stream, connectedToEndpoint);
			NodeIdentifier connectedFromIdentifier;		serialize::Serializer::Deserialize(stream, connectedFromIdentifier);
			EndpointIdentifier connectedFromEndpoint;	serialize::Serializer::Deserialize(stream, connectedFromEndpoint);

			// Connect nodes
			ConnectNodes(FindNode(connectedToIdentifier), connectedToEndpoint, FindNode(connectedFromIdentifier), connectedFromEndpoint);
		}
	}
}