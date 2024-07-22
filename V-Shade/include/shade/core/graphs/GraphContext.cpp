#include "shade_pch.h"
#include "GraphContext.h"
#include <shade/core/graphs/nodes/BaseNode.h>

bool shade::graphs::GraphContext::RemoveAllInputConnection(BaseNode* firstNode)
{
	const auto node = Connections.find(firstNode);

	if (node != Connections.end())
	{
		for (auto& connection : node->second)
		{
			connection.OutputNode->OnDisconnect(Connection::Type::Output, connection.OutputNode->GetEndpoint<Connection::Output>(connection.OutputEndpoint)->GetType(), connection.OutputEndpoint);
			connection.InputNode->OnDisconnect(Connection::Type::Input, connection.InputNode->GetEndpoint<Connection::Input>(connection.InputEndpoint)->GetType(), connection.InputEndpoint);
		}

		Connections.erase(node);
		return true;
	}
	else
	{
		return false;
	}
}


bool shade::graphs::GraphContext::RemoveAllOutputConnection(BaseNode* firstNode)
{
	bool isRemoved = false;

	for (auto it = Connections.begin(); it != Connections.end(); )
	{
		auto& [input, connections] = *it;

		// Remove connections where OutputNode == firstNode
		auto remove = std::remove_if(connections.begin(), connections.end(), [firstNode](const Connection& connection)
			{
				return connection.OutputNode == firstNode;
			});

		if (remove != connections.end())
		{
			
			remove->OutputNode->OnDisconnect(Connection::Type::Output, remove->OutputNode->GetEndpoint<Connection::Output>(remove->OutputEndpoint)->GetType(), remove->OutputEndpoint);
			remove->InputNode->OnDisconnect(Connection::Type::Input, remove->InputNode->GetEndpoint<Connection::Input>(remove->InputEndpoint)->GetType(), remove->InputEndpoint);

			connections.erase(remove, connections.end());
			isRemoved = true;
		}

		// Check if the connections vector is now empty
		if (connections.empty())
		{
			// Erase the entry from the Connections map
			it = Connections.erase(it);
		}
		else
		{
			++it;
		}
	}

	return isRemoved;
}

