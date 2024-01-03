#pragma once
#include <shade/core/asset/Asset.h>
#include <shade/core/animation/graph/nodes/GraphNode.h>
#include <shade/core/animation/graph/nodes/TransitionNode.h>
#include <shade/core/animation/graph/nodes/BlendNode2D.h>
#include <shade/core/animation/graph/nodes/PoseNode.h>
#include <shade/core/animation/graph/nodes/OutputPoseNode.h>
#include <shade/core/animation/AnimationController.h>

/*
	Node -> virtual class
		Has instantiate method
			Argumetns is Instatiate context
				Context has container with resources(void*) and casted to specific type (Animation, etc)

				Make root node as Out final node, connect evereting to this node as childs
				got to the end fo some brunch if it las node and there no childrens - start to execute node and go back to previews node
				if there some children got till end as it was with previws node, do it for all branches till we get back to root node 
				It will broke some common parent child pattern where you shodul execute every child continiusly from parent 
*/

namespace shade
{
	namespace animation
	{
		class SHADE_API AnimationGraph : ASSET_INHERITANCE(AnimationGraph)
		{
		public:
			// TODO: ADD HELPER 
			// Destructor
			virtual ~AnimationGraph() = default;
			// Static method to get the static type of AnimationGraph asset
			static AssetMeta::Type GetAssetStaticType();
			// Virtual method to get the type of the AnimationGraph asset
			virtual AssetMeta::Type GetAssetType() const override;
			// Default constructor
			AnimationGraph(const Asset<Skeleton>& skeleton);
			// Static method to create a shared pointer to a AnimationGraph object
			static Asset<AnimationGraph> CreateEXP(const Asset<Skeleton>& skeleton); // DANGER!!
		public:
			bool AddConnection(GraphNode::NodeIDX sourceNode, GraphNode::EndpointIDX sourceEndpoint, GraphNode::NodeIDX destinationNode, GraphNode::EndpointIDX destinationEndpoint);
			bool AddRootConnection(GraphNode::NodeIDX sourceNode, GraphNode::EndpointIDX sourceEndpoint);
			void SetSkeleton(const Asset<Skeleton>& skeleton);
			const Asset<Skeleton>& GetSkeleton() const;

			template<typename Node, typename... Args>
			inline SharedPointer<Node> CreateNode(Args&&... args)
			{
				return SharedPointer<Node>(m_Nodes.emplace_back(SharedPointer<Node>::Create(m_Nodes.size(), m_GraphContext, std::forward<Args>(args)...)));
			}

			template<typename Node>
			inline SharedPointer<Node> GetNode(GraphNode::NodeIDX nodeIdx)
			{
				return  SharedPointer<Node>(m_Nodes.at(nodeIdx));
			}

			void Evaluate(const FrameTimer& delatTime);
			const SharedPointer<OutputPoseNode>& const GetOutputPoseNode();
		private:
			SharedPointer<animation::GraphNode> FindNode(GraphNode::NodeIDX idx);
			bool ConnectValues(const std::shared_ptr<NodeValue>& sourceEndpoint, std::shared_ptr<NodeValue>& destinationEndpoint);
		private:
			// Create a skeleton object with the given asset data, lifetime, and instantiation behaviour
			AnimationGraph(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
			// Create a new skeleton object with the given asset data, lifetime, and instantiation behaviour
			static AnimationGraph* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
			// Serialize the animationGraph object and write the serialized data to the given output stream
			std::size_t Serialize(std::ostream& stream) const;
			// Deserialize the animationGraph object from the given input stream and return the number of bytes read
			std::size_t Deserialize(std::istream& stream);
		private:
			friend class Serializer;
			friend class Asset<AnimationGraph>;
		private:
			std::vector<SharedPointer<animation::GraphNode>> m_Nodes;
			GraphContext m_GraphContext;
		public:
			SharedPointer<OutputPoseNode> m_RootNode;
			SharedPointer<AnimationController> m_AnimationController;
		};
	}
}