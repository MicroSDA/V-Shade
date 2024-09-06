#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>

namespace shade
{
	namespace graphs
	{
		class SHADE_API FloatScaleRange : public BaseNode
		{
			NODE_STATIC_TYPE_HELPER(FloatScaleRange)

		public:
			FloatScaleRange(GraphContext* context, NodeIdentifier identifier, BaseNode* pParentNode) :
				BaseNode(context, identifier, pParentNode, "Float scale to range")
			{
				m_CanBeOpen = false;

				REGISTER_ENDPOINT<Connection::Input,  NodeValueType::Float>(0.f); // Value
				
				REGISTER_ENDPOINT<Connection::Input,  NodeValueType::Float>(0.f); // Value min
				REGISTER_ENDPOINT<Connection::Input,  NodeValueType::Float>(0.f); // Value max

				REGISTER_ENDPOINT<Connection::Input,  NodeValueType::Float>(0.f); // Range min
				REGISTER_ENDPOINT<Connection::Input,  NodeValueType::Float>(0.f); // Range max

				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Float>(0.f); // Output

			}; virtual ~FloatScaleRange() = default;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) override {};
			virtual void Evaluate(const FrameTimer& deltaTime) override;
		private:
			float m_Compare = 0;
		};
	}
}
