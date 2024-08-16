#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>

namespace shade
{
	namespace graphs
	{
		class SHADE_API StringNode : public BaseNode
		{
		public:
			StringNode(GraphContext* context, NodeIdentifier identifier, BaseNode* pParentNode)
				: BaseNode(context, identifier, pParentNode, "String")
			{
				m_CanBeOpen = false;
				REGISTER_ENDPOINT<Connection::Output, NodeValueType::String>("");
			}; virtual ~StringNode() = default;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) override {};
			virtual void Evaluate(const FrameTimer& deltaTime) override {}
		};
	}
}