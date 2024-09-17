#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>
#include <shade/core/animation/Animation.h>
#include <shade/core/animation/graphs/AnimationGraphContext.h>
#include <shade/core/animation/graphs/nodes/PoseNode.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API AdditivePose : public PoseNode
		{
			NODE_STATIC_TYPE_HELPER(AdditivePose)

		public:
			AdditivePose(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode);
			virtual ~AdditivePose() = default;
			virtual void Evaluate(const FrameTimer& delatTime) override;

			virtual void OnConnect(graphs::Connection::Type connectionType, NodeValueType type, graphs::EndpointIdentifier endpoint) override {};
			bool m_IsAddative = false;
		private:


			AnimationController::AnimationControlData m_AnimationData;
		};
	}
}