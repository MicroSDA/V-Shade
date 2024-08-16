#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>

namespace shade
{
	namespace graphs
	{
		class SHADE_API BoolNode : public BaseNode
		{
		public:
			BoolNode(GraphContext* context, NodeIdentifier identifier, BaseNode* pParentNode) :
				BaseNode(context, identifier, pParentNode, "Bool")
			{
				m_CanBeOpen = false;
				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Bool>(false);
			}; virtual ~BoolNode() = default;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) override {};
			virtual void Evaluate(const FrameTimer& deltaTime) override {}
		};
	}
}

