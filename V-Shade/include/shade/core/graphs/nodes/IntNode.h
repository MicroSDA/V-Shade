#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>

namespace shade
{
	namespace graphs
	{
		class SHADE_API IntNode : public BaseNode
		{
		public:
			IntNode(GraphContext* context, NodeIdentifier identifier) : BaseNode(context, identifier)
			{
				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Int>(0);
			}; virtual ~IntNode() = default;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) override {};
			virtual void Evaluate(const FrameTimer& deltaTime) override { std::cout << std::hex << this << "\n"; }
		};
	}
}