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
			NODE_STATIC_TYPE_HELPER(PoseNode)

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
			void ResetAnimationData(const AnimationController::AnimationControlData& data);

			SHADE_INLINE Pose* GetOutputPose()
			{
				return GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0);
			}
			SHADE_INLINE const Pose* GetOutputPose() const
			{
				return GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0);
			}

			const AnimationController::AnimationControlData& GetAnimationData() const;
			AnimationController::AnimationControlData& GetAnimationData();
			bool m_IsAddative = false; // Need to set private !! or remove
		private:
			
			virtual void SerializeBody(std::ostream& stream) const override;
			virtual void DeserializeBody(std::istream& stream) override;

			AnimationController::AnimationControlData m_AnimationData;
		};
	}
}