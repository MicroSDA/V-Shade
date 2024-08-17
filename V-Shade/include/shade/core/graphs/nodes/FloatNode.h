#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>

namespace shade
{
	namespace graphs
	{
		class SHADE_API FloatNode : public BaseNode
		{
			NODE_STATIC_TYPE_HELPER(FloatNode)

		public:
			FloatNode(GraphContext* context, NodeIdentifier identifier, BaseNode* pParentNode) :
				BaseNode(context, identifier, pParentNode, "Float")
			{
				m_CanBeOpen = false;
				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Float>(0);
			}; virtual ~FloatNode() = default;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) override {};
			virtual void Evaluate(const FrameTimer& deltaTime) override {}
		};
	}
}

