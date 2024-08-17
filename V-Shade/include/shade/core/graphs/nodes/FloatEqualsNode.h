#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>

namespace shade
{
	namespace graphs
	{
		class SHADE_API FloatEqualsNode : public BaseNode
		{
			NODE_STATIC_TYPE_HELPER(FloatEqualsNode)

		public:
			FloatEqualsNode(GraphContext* context, NodeIdentifier identifier, BaseNode* pParentNode) :
				BaseNode(context, identifier, pParentNode, "Float equals")
			{
				m_CanBeOpen = false;

				REGISTER_ENDPOINT<Connection::Input, NodeValueType::Float>(0);
				REGISTER_ENDPOINT<Connection::Input, NodeValueType::Float>(m_Compare);

				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Bool>(0.f);

			}; virtual ~FloatEqualsNode() = default;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) override {};
			virtual void Evaluate(const FrameTimer& deltaTime) override;
		private:
			float m_Compare = 0;
		};
	}
}
