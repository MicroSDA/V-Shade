#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>
#include <shade/core/animation/graphs/AnimationGraphContext.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API BlendTree2D : public graphs::BaseNode
		{
			NODE_STATIC_TYPE_HELPER(BlendTree2D)

		public:
			BlendTree2D(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode);
			virtual ~BlendTree2D() = default;
		public:
			void AddInputPose();
			void RemoveInputPose(const graphs::EndpointIdentifier identifier);

			virtual void Evaluate(const FrameTimer& delatTime) override;

			virtual void Deserialize(std::istream& stream) override;
		};
	}
}