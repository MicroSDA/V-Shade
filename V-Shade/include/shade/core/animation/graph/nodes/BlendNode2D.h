#pragma oncez
#include <shade/core/animation/graph/nodes/GraphNode.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API BlendNode2D : public GraphNode
		{
		public:
			BlendNode2D(NodeIDX idx, const GraphContext& context) :
				GraphNode(idx, context), DefaultWeightValue(0.0f), Mask(BoneMask{ context.Skeleton })
			{

				REGISTER_ENDPOINT<Connection::Input,  NodeValueType::Float>(DefaultWeightValue);

				REGISTER_ENDPOINT<Connection::Input,  NodeValueType::Pose>(nullptr);
				REGISTER_ENDPOINT<Connection::Input,  NodeValueType::Pose>(nullptr);

				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Pose>(nullptr);
			}
			virtual void Evaluate(const FrameTimer& delatTime) override;
			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};
		private:
			float DefaultWeightValue;
			////////Internal/////////
			BoneMask Mask;
		};
	}
}

