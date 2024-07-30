#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>
#include <shade/core/animation/Animation.h>
#include <shade/core/animation/graphs/AnimationGraphContext.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API PoseNode : public graphs::BaseNode
		{

		public:
			PoseNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode) :
				BaseNode(context, identifier, pParentNode, "Pose")
			{
				m_CanBeOpen = false;
				REGISTER_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(nullptr);
			}
			virtual ~PoseNode() = default;
			virtual void Evaluate(const FrameTimer& delatTime) override;

			virtual void OnConnect(graphs::Connection::Type connectionType, NodeValueType type, graphs::EndpointIdentifier endpoint) override {};

			void ResetAnimationData(const Asset<Animation>& animation);
			AnimationController::AnimationControlData& GetAnimationData();
		private:
			AnimationController::AnimationControlData AnimationData;
		};
	}
}