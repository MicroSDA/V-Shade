#pragma once
#include <shade/core/animation/graph/nodes/GraphNode.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API BoneMaskNode : public GraphNode
		{

		public:
			BoneMaskNode(NodeIDX idx, const GraphContext& context) :
				GraphNode(idx, context)
			{
				REGISTER_ENDPOINT<Connection::Output, NodeValueType::BoneMask>(context.Skeleton);
			}
			virtual ~BoneMaskNode() noexcept = default;
			virtual void Evaluate(const FrameTimer& delatTime) override;
			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};

			BoneMask& GetBoneMask();
		private:

		};
	}
}