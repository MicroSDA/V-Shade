#pragma once
#include <shade/core/graphs/GraphContext.h>
#include <shade/core/animation/Skeleton.h>
#include <shade/core/animation/AnimationController.h>


namespace shade
{
	namespace animation
	{
		/// @brief Structure representing the context of a graph
		struct AnimationGraphContext : public graphs::GraphContext
		{
		public:
			///@brief Current entity
			Asset<Skeleton> Skeleton;
			///@brief AnimationController
			mutable SharedPointer<AnimationController> Controller;
		};
	}
}