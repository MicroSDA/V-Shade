#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>
#include <shade/core/animation/graphs/AnimationGraphContext.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API BoneMaskNode : public  graphs::BaseNode
		{

		public:
			BoneMaskNode(graphs::NodeIdentifier identifier, graphs::GraphContext* context) :
				BaseNode(context, identifier)
			{
				//REGISTER_ENDPOINT<graphs::Connection::Output, NodeValueType::BoneMask>(context->As<AnimationGraphContext>().Skeleton);
				REGISTER_ENDPOINT<graphs::Connection::Output, NodeValueType::BoneMask>(nullptr);
			}
			virtual ~BoneMaskNode() = default;
			virtual void Evaluate(const FrameTimer& delatTime) override;
			virtual void OnConnect(graphs::Connection::Type connectionType, NodeValueType type, graphs::EndpointIdentifier endpoint) override {};

			BoneMask& GetBoneMask();
		private:

		};
	}
}