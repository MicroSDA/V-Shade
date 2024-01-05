#pragma once
#include <shade/core/animation/graph/nodes/GraphNode.h>
#include <shade/core/animation/Animation.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API OutputPoseNode : public GraphNode
		{
		public:
			OutputPoseNode(NodeIDX idx, const GraphContext& context) :
				GraphNode(idx, context)
			{
				// We have only input !
				REGISTER_ENDPOINT<Connection::Input, NodeValueType::Pose>(nullptr);
			}

			virtual ~OutputPoseNode() = default;

			virtual void Evaluate(const FrameTimer& delatTime) override {};
			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};
			virtual void OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) override {};

			const Pose* GetFinalPose() const;
		};
	}
}
