#pragma once
#include <shade/core/asset/Asset.h>
#include <shade/core/animation/AnimationController.h>
#include <shade/core/animation/graphs/nodes/PoseNode.h>
#include <shade/core/animation/graphs/nodes/OutputPoseNode.h>
#include <shade/core/animation/graphs/nodes/BoneMaskNode.h>
#include <shade/core/animation/graphs/AnimationGraphContext.h>
#include <shade/core/animation/graphs/nodes/StateMachineNode.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API AnimationGraph : public graphs::BaseNode
		{
		public:
			AnimationGraph(graphs::GraphContext* context, graphs::NodeIdentifier identifier = 0u);
			virtual ~AnimationGraph() = default;

			void ProcessGraph(const shade::FrameTimer& deltaTime);

			const Pose* GetOutPutPose() const;

			const NodeValues& GetGlobalValues() const;
			NodeValues& GetGlobalValues();

		private:
			virtual void Evaluate(const FrameTimer& deltaTime) override;
			OutputPoseNode* m_OutPutPoseNode = nullptr;
			NodeValues m_GlobalValues;
		};
	}
}
// TODO: Fiuere out context polynorphism !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//namespace shade
//{
//	namespace animation
//	{
//		class SHADE_API AnimationGraph : public Graph, ASSET_INHERITANCE(AnimationGraph)
//		{
//		public:
//			// TODO: ADD HELPER 
//			// Destructor
//			virtual ~AnimationGraph() = default;
//			// Static method to get the static type of AnimationGraph asset
//			static AssetMeta::Type GetAssetStaticType();
//			// Virtual method to get the type of the AnimationGraph asset
//			virtual AssetMeta::Type GetAssetType() const override;
//			// Default constructor
//			AnimationGraph(const Asset<Skeleton>&skeleton, ecs::Entity entity);
//			// Static method to create a shared pointer to a AnimationGraph object
//			static Asset<AnimationGraph> CreateEXP(const Asset<Skeleton>& skeleton, ecs::Entity entity); // DANGER!!
//		protected:
//			AnimationGraph() = default;
//		public:
//			void SetSkeleton(const Asset<Skeleton>&skeleton);
//			const Asset<Skeleton>& GetSkeleton() const;
//			SharedPointer<OutputPoseNode> GetOutputPoseNode(); // Tip: cannot be const ref !
//		private:
//			// Create a skeleton object with the given asset data, lifetime, and instantiation behaviour
//			AnimationGraph(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
//			// Create a new skeleton object with the given asset data, lifetime, and instantiation behaviour
//			static AnimationGraph* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
//			// Serialize the animationGraph object and write the serialized data to the given output stream
//			std::size_t Serialize(std::ostream& stream) const;
//			// Deserialize the animationGraph object from the given input stream and return the number of bytes read
//			std::size_t Deserialize(std::istream& stream);
//		private:
//			friend class Serializer;
//			friend class Asset<AnimationGraph>;
//		private:
//			animation::AnimationGraphContext m_GraphContext;
//			SharedPointer<AnimationController> m_AnimationController;
//		};
//	}
//}