#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>

namespace shade
{
	namespace graphs
	{
		class SHADE_API IntNode : public BaseNode
		{
			NODE_STATIC_TYPE_HELPER(IntNode)

		public:
			IntNode(GraphContext* context, NodeIdentifier identifier, BaseNode* pParentNode) 
				: BaseNode(context, identifier, pParentNode, "Int")
			{
				m_CanBeOpen = false;
				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Int>(0);
			}; virtual ~IntNode() = default;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) override {};
			virtual void Evaluate(const FrameTimer& deltaTime) override {}
		};
	}
}