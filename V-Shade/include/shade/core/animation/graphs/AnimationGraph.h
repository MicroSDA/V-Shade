#pragma once
#include <shade/core/memory/Memory.h>
#include <shade/core/asset/Asset.h>
#include <shade/core/animation/AnimationController.h>
#include <shade/core/animation/graphs/nodes/PoseNode.h>
#include <shade/core/animation/graphs/nodes/OutputPoseNode.h>
#include <shade/core/animation/graphs/nodes/BoneMaskNode.h>
#include <shade/core/animation/graphs/AnimationGraphContext.h>
#include <shade/core/animation/graphs/nodes/StateMachineNode.h>
#include <shade/core/graphs/nodes/IntNode.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API AnimationGraph : public graphs::BaseNode//, ASSET_INHERITANCE(AnimationGraph)
		{
			//ASSET_DEFINITION_HELPER(AnimationGraph)

		public:
			AnimationGraph(graphs::GraphContext* context, graphs::NodeIdentifier identifier = 0u);
			virtual ~AnimationGraph() = default;

			void ProcessGraph(const shade::FrameTimer& deltaTime);

			const Pose* GetOutPutPose() const;

			template<typename T, typename... Args>
			SHADE_INLINE T* CreateInputNode(const std::string& name, Args&&... args)
			{
				T* node = SNEW T(GetGraphContext(), m_InputNodes.size(), std::forward<Args>(args)...);
				m_InputNodes.emplace(name, node).first->second->Initialize(this, this);
				return node;
			}

			template<typename T>
			SHADE_INLINE void SetInputValue(const std::string& name, const T& value)
			{
				auto node = GetInputNode(name);
				assert(FromTypeToNodeValueType<T>::Type == node->GetEndpoint<graphs::Connection::Output>(0)->GetType() && "Wrong value type");
				node->GET_ENDPOINT<graphs::Connection::Output, FromTypeToNodeValueType<T>::Type>(0, value);
			}

			BaseNode* GetInputNode(const std::string& name);
			const BaseNode* GetInputNode(const std::string& name) const;

			std::unordered_map<std::string, BaseNode*> GetInputNodes() { return m_InputNodes; }
			const std::unordered_map<std::string, BaseNode*> GetInputNodes() const { return m_InputNodes; }
		private:
			virtual void Evaluate(const FrameTimer& deltaTime) override;
			OutputPoseNode* m_OutPutPoseNode = nullptr;
			std::unordered_map<std::string, BaseNode*> m_InputNodes;
		private:
			// Create a animation graph object with the given asset data, lifetime, and instantiation behaviour
			//AnimationGraph(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
			// Serialize the animation graph object and write the serialized data to the given output stream
			std::size_t Serialize(std::ostream& stream) const;
			// Deserialize the animation graph object from the given input stream and return the number of bytes read
			std::size_t Deserialize(std::istream& stream);
		private:
			friend class Serializer;
		};
	}

	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const animation::AnimationGraph& graph, std::size_t)
	{
		return graph.Serialize(stream);
	}
	/* Deserialize Skeleton.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, animation::AnimationGraph& graph, std::size_t)
	{
		return graph.Deserialize(stream);
	}
	/* Serialize Asset<Skeleton>.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Asset<animation::AnimationGraph>& graph, std::size_t)
	{
		return graph->Serialize(stream);
	}
	/* Deserialize Asset<Skeleton>.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Asset<animation::AnimationGraph>& graph, std::size_t)
	{
		return graph->Deserialize(stream);
	}
}


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