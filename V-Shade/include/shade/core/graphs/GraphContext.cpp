#include "shade_pch.h"
#include "GraphContext.h"
#include <shade/core/graphs/nodes/BaseNode.h>

SHADE_API void shade::graphs::GraphContext::InitializeNode(BaseNode* pNode, BaseNode* pParrent)
{
	Nodes.emplace(pNode, NodesPack{}).first->first->Initialize();
}

SHADE_API bool shade::graphs::GraphContext::RemoveNode(BaseNode* pNode)
{
	std::unordered_map<BaseNode*, NodesPack>::iterator node = Nodes.find(pNode);

	assert(node != Nodes.end() && "");

	RemoveAllConnection(pNode);

	for (BaseNode* internalNode : GetInternalNodes(pNode))
	{
		RemoveNode(internalNode);
	}

	BaseNode* pParrent = pNode->GetParrentGraph();

	if (pParrent != nullptr)
	{
		std::unordered_map<BaseNode*, NodesPack>::iterator parrent = Nodes.find(pParrent);
		assert(parrent != Nodes.end() && "");

		std::vector<BaseNode*>& internalNodes = parrent->second.InternalNodes;

		auto asInternalNode = std::find_if(internalNodes.begin(), internalNodes.end(), [pNode](const BaseNode* node)
			{
				return node == pNode;
			});

		internalNodes.erase(asInternalNode, internalNodes.end()); // Need to remove them recursevly!
	}

	pNode->Shutdown(); SDELETE pNode; Nodes.erase(node);

	return true;
}

bool shade::graphs::GraphContext::RemoveAllInputConnection(BaseNode* pConnectedTo)
{
	std::vector<Connection>* connections = GetConnections(pConnectedTo);

	if (connections != nullptr)
	{
		connections->clear();
		return true;
	}

	return false;
}


bool shade::graphs::GraphContext::RemoveAllOutputConnection(BaseNode* pConnectedFrom)
{
	bool wasRemoved = false;

	for (auto& [node, pack] : Nodes)
	{
		std::vector<Connection>& connections = pack.Connections;

		auto remove = std::remove_if(connections.begin(), connections.end(), [pConnectedFrom](const Connection& connection)
			{
				return connection.PConnectedFrom == pConnectedFrom;
			});

		if (remove != connections.end())
		{
			connections.erase(remove, connections.end());
			wasRemoved = true;
		}
	}

	return wasRemoved;
}

