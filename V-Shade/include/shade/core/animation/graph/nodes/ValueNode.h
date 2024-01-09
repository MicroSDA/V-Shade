#pragma once
#include <shade/core/animation/graph/nodes/GraphNode.h>
#include <shade/core/animation/Animation.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API ValueNode : public GraphNode
		{

		public:
			ValueNode(NodeIDX idx, const GraphContext& context) :
				GraphNode(idx, context)
			{
				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Float>(0.f);
			}
			virtual ~ValueNode() noexcept = default;
			virtual void Evaluate(const FrameTimer& delatTime) override;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};
			virtual void OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};

		private:

		};
	}
}