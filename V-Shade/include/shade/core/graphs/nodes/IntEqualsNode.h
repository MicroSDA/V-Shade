#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>

namespace shade
{
	namespace graphs
	{
		class SHADE_API IntEqualsNode : public BaseNode
		{
			NODE_STATIC_TYPE_HELPER(IntEqualsNode)

		public:
			IntEqualsNode(GraphContext* context, NodeIdentifier identifier, BaseNode* pParentNode) :
				BaseNode(context, identifier, pParentNode, "Int equals")
			{
				m_CanBeOpen = false;
				REGISTER_ENDPOINT<Connection::Input, NodeValueType::Int>(0);
				REGISTER_ENDPOINT<Connection::Input, NodeValueType::Int>(m_Compare);

				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Bool>(0.f);

			}; virtual ~IntEqualsNode() = default;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) override {};
			virtual void Evaluate(const FrameTimer& deltaTime) override;
		private:
			std::int32_t m_Compare = 0;
		};
	}
}
