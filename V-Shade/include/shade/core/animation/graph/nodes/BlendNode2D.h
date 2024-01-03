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
				REGISTER_INPUT_ENDPOINT(	FLOAT,	DefaultWeightValue);				

				REGISTER_INPUT_ENDPOINT(	POSE,   nullptr);	
				// Sorce pose
				REGISTER_INPUT_ENDPOINT(	POSE,	nullptr);					// Target pose

				REGISTER_OUTPUT_ENDPOINT(	POSE,	nullptr);					// Out pose
			}
			virtual void Evaluate(const FrameTimer& delatTime) override;
			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};
			virtual void OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override;
		private:
			float DefaultWeightValue;
			////////Internal/////////
			BoneMask Mask;
		};
	}
}

