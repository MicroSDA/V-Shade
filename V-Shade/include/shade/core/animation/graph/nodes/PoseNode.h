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
			struct AnimationControllData
			{
				Asset<Animation> Animation;
				Animation::State State = Animation::State::Stop;
				float CurrentPlayTime = 0.0;
				float Duration = 0.0;
				float TiksPerSecond = 0.0;
			};
		public:
			PoseNode(NodeIDX idx, const GraphContext& context) :
				GraphNode(idx, context)
			{
				REGISTER_INPUT_ENDPOINT(POSE,  nullptr)
				REGISTER_OUTPUT_ENDPOINT(POSE, nullptr)
			}
			virtual ~PoseNode() noexcept = default;
			virtual void Evaluate(const FrameTimer& delatTime) override;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};
			virtual void OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};
		public:
			AnimationControllData	AnimationData;
		};
	}
}