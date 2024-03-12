#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>

namespace shade 
{
	namespace graphs
	{
		class SHADE_API ValueNode : public BaseNode
		{
		public:
			ValueNode(GraphContext* context, NodeIdentifier identifier) : BaseNode(context, identifier)
			{
				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Float>(0.f);
				REGISTER_ENDPOINT<Connection::Input, NodeValueType::Float>(0.f);

			}; virtual ~ValueNode() = default;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) override {};
			virtual void Evaluate(const FrameTimer& deltaTime) override { std::cout << std::hex << this << "\n"; }
		};
	}
}

