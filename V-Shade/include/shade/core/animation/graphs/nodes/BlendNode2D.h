#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>
#include <shade/core/animation/graphs/AnimationGraphContext.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API BlendNode2D : public graphs::BaseNode
		{
			NODE_STATIC_TYPE_HELPER(BlendNode2D)

		public:
			BlendNode2D(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode) :
				BaseNode(context, identifier, pParentNode, "Blend2D"), DefaultWeightValue(0.0f), Mask(BoneMask{nullptr})
			{
				m_CanBeOpen = false;

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

