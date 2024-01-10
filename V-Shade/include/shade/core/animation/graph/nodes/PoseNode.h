#pragma once
#include <shade/core/animation/graph/nodes/GraphNode.h>
#include <shade/core/animation/Animation.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API PoseNode : public GraphNode
		{
		
		public:
			PoseNode(NodeIDX idx, const GraphContext& context) :
				GraphNode(idx, context)
			{
				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Pose>(nullptr);
			}
			virtual ~PoseNode() noexcept = default;
			virtual void Evaluate(const FrameTimer& delatTime) override;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};

			void ResetAnimationData(const Asset<Animation>& animation);
			AnimationController::AnimationControllData& GetAnimationData();
		private:
			AnimationController::AnimationControllData AnimationData;
		};
	}
}