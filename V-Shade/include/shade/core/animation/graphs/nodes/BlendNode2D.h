#pragma oncez
#include <shade/core/graphs/nodes/BaseNode.h>
#include <shade/core/animation/graphs/AnimationGraphContext.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API BlendNode2D : public graphs::BaseNode
		{
		public:
			BlendNode2D(graphs::GraphContext* context, graphs::NodeIdentifier identifier) :
				BaseNode(context, identifier), DefaultWeightValue(0.0f), Mask(BoneMask{ nullptr })
			{
				REGISTER_ENDPOINT<graphs::Connection::Input,  NodeValueType::Float>(DefaultWeightValue);
				REGISTER_ENDPOINT<graphs::Connection::Input,  NodeValueType::BoneMask>(GetGraphContext()->As<AnimationGraphContext>().Skeleton);

				REGISTER_ENDPOINT<graphs::Connection::Input,  NodeValueType::Pose>(nullptr);
				REGISTER_ENDPOINT<graphs::Connection::Input,  NodeValueType::Pose>(nullptr);

				REGISTER_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(nullptr);
			}
			virtual ~BlendNode2D() = default;
			virtual void Evaluate(const FrameTimer& delatTime) override;
			virtual void OnConnect(graphs::Connection::Type connectionType, NodeValueType type, graphs::NodeIdentifier endpoint) override {};
		private:
			float DefaultWeightValue;
			////////Internal/////////
			BoneMask Mask;
		};
	}
}
