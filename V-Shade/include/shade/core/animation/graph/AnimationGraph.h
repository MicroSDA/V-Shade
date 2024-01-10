#pragma once
#include <shade/core/asset/Asset.h>
#include <shade/core/animation/graph/nodes/GraphNode.h>
#include <shade/core/animation/graph/nodes/TransitionNode.h>
#include <shade/core/animation/graph/nodes/BlendNode2D.h>
#include <shade/core/animation/graph/nodes/PoseNode.h>
#include <shade/core/animation/graph/nodes/OutputPoseNode.h>
#include <shade/core/animation/AnimationController.h>
#include <shade/core/animation/graph/nodes/ValueNode.h>

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
			//bool AddConnection(GraphNode::NodeIDX sourceNode, GraphNode::EndpointIDX sourceEndpoint, GraphNode::NodeIDX destinationNode, GraphNode::EndpointIDX destinationEndpoint);
			bool AddConnection(GraphNode::NodeIDX inputNode, GraphNode::EndpointIDX inputEndpoint, GraphNode::NodeIDX outputNode, GraphNode::EndpointIDX outputEndpoint);
			bool RemoveConnection(GraphNode::NodeIDX inputNode, GraphNode::EndpointIDX inputEndpoint);


			bool AddRootConnection(GraphNode::NodeIDX inputNodeIdx, GraphNode::EndpointIDX outputEndpointIdx);
			bool RemoveRootConnection();

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

			std::vector<SharedPointer<animation::GraphNode>>& GetNodes() { return m_Nodes; }

			void Evaluate(const FrameTimer& delatTime);
			const SharedPointer<OutputPoseNode>& const GetOutputPoseNode();
		private:
			SharedPointer<animation::GraphNode> FindNode(GraphNode::NodeIDX idx);

			//bool ConnectValues(const std::shared_ptr<NodeValue>* sourceEndpoint, std::shared_ptr<NodeValue>* destinationEndpoint);
			bool ConnectValues(std::shared_ptr<NodeValue>* inputEndpoint, const std::shared_ptr<NodeValue>* outputEndpoint);
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