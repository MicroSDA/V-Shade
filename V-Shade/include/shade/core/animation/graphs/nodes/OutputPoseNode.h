#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>
#include <shade/core/animation/Animation.h>
#include <shade/core/animation/graphs/AnimationGraphContext.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API OutputPoseNode : public graphs::BaseNode
		{
			NODE_STATIC_TYPE_HELPER(OutputPoseNode)

		public:
			OutputPoseNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode) :
				BaseNode(context, identifier, pParentNode, "Output pose")
			{
				m_IsRenamable = false;
				m_IsRemovable = false;
				m_CanBeOpen = false;
				// We have only input !
				REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(nullptr);
			}

			virtual ~OutputPoseNode() = default;
			virtual void Evaluate(const FrameTimer& delatTime) override {};
			virtual void OnConnect(graphs::Connection::Type connectionType, NodeValueType type, graphs::EndpointIdentifier endpoint) override {};

			const Pose* GetFinalPose() const;
		};
	}
}
