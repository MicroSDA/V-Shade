#pragma once
#include <shade/core/animation/graph/nodes/GraphNode.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API TransitionNode : public GraphNode
		{
		public:
			TransitionNode(NodeIDX idx, const GraphContext& context) :
				GraphNode(idx, context)
			{
				REGISTER_INPUT_ENDPOINT(FLOAT, 0.0f); // Weight
				REGISTER_OUTPUT_ENDPOINT(FLOAT, 0.0f); // Weight
			}
			virtual void Evaluate(const FrameTimer& delatTime) override;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};
			virtual void OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};
		private:
			
		};
	}
}

