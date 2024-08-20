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
#include <shade/core/graphs/nodes/IntEqualsNode.h>
#include <shade/core/graphs/nodes/FloatNode.h>
#include <shade/core/graphs/nodes/FloatEqualsNode.h>
#include <shade/core/graphs/nodes/StringNode.h>

namespace shade
{
	namespace animation
	{
		class SHADE_API AnimationGraph : public graphs::BaseNode, ASSET_INHERITANCE(AnimationGraph)
		{
			ASSET_DEFINITION_HELPER_1(AnimationGraph, graphs::GraphContext*, context)

			NODE_STATIC_TYPE_HELPER(AnimationGraph)

		public:
			/// @brief Constructs an AnimationGraph with the given context and optional name.
			/// @param context The context to be used for the graph.
			/// @param name The optional name of the graph. Default is "Animation graph".
			AnimationGraph(graphs::GraphContext* context, const std::string& name = "Animation graph");

			/// @brief Destructor for the AnimationGraph.
			virtual ~AnimationGraph() = default;

			/// @brief Processes the graph for the current frame.
			/// @param deltaTime The time elapsed since the last frame.
			void ProcessGraph(const shade::FrameTimer& deltaTime);

			/// @brief Gets the current output pose of the graph.
			/// @return A pointer to the current Pose object, or nullptr if there is no output pose.
			const Pose* GetOutPutPose() const;

			/// @brief Creates an input node of the specified type and adds it to the graph.
			/// @tparam T The type of the node to be created.
			/// @param name The name of the new input node.
			/// @param args The arguments to be passed to the node constructor.
			/// @return A pointer to the newly created node, or nullptr if a node with the same name already exists.
			template<typename T, typename... Args>
			SHADE_INLINE T* CreateInputNode(const std::string& name, Args&&... args)
			{
				if (m_InputNodes.find(name) != m_InputNodes.end())
					return nullptr;

				auto node = GetGraphContext()->CreateNode<T>(this, std::forward<Args>(args)...);

				node->SetName(name);

				m_InputNodes.emplace(name, node);

				return node;
			}

			/// @brief Renames an existing input node.
			/// @param pNode The node to be renamed.
			/// @param newName The new name for the node.
			/// @return True if the node was successfully renamed, false otherwise.
			SHADE_INLINE bool RenameInputNode(BaseNode* pNode, const std::string& newName)
			{
				auto node = m_InputNodes.find(pNode->GetName());
				if (node != m_InputNodes.end())
				{
					if (m_InputNodes.find(newName) == m_InputNodes.end())
					{
						auto pN = node->second;
						pN->SetName(newName);
						m_InputNodes.erase(node);
						m_InputNodes.emplace(newName, pN);
						return true;
					}
				}
				return false;
			}

			/// @brief Removes an input node by name.
			/// @param name The name of the node to be removed.
			/// @return True if the node was successfully removed, false if the node was not found.
			SHADE_INLINE bool RemoveInputNode(const std::string& name)
			{
				auto node = m_InputNodes.find(name);
				if (node != m_InputNodes.end())
				{
					m_InputNodes.erase(node);
					return true;
				}

				return false;
			}

			/// @brief Removes an input node by pointer.
			/// @param pNode The pointer to the node to be removed.
			/// @return True if the node was successfully removed, false if the node was not found.
			SHADE_INLINE bool RemoveInputNode(BaseNode* pNode)
			{
				auto node = std::find_if(m_InputNodes.begin(), m_InputNodes.end(), [pNode](const std::pair<std::string, BaseNode*>& node)
					{
						return node.second == pNode;
					});

				if (node != m_InputNodes.end())
				{
					m_InputNodes.erase(node);
					return true;
				}

				return false;
			}

			/// @brief Sets the value of an input node.
			/// @tparam T The type of the value to set.
			/// @param name The name of the input node.
			/// @param value The value to set.
			/// @note This function asserts if the value type does not match the node's expected type.
			template<typename T>
			SHADE_INLINE void SetInputValue(const std::string& name, const T& value)
			{
				if (BaseNode* node = GetInputNode(name))
				{
					assert(FromTypeToNodeValueType<T>::Type == node->GetEndpoint<graphs::Connection::Output>(0)->GetType() && "Wrong value type");
					node->GET_ENDPOINT<graphs::Connection::Output, FromTypeToNodeValueType<T>::Type>(0, value);
				}
			}

			/// @brief Removes a node from the graph.
			/// @param pNode The node to be removed.
			/// @return True if the node was successfully removed, false otherwise.
			virtual bool RemoveNode(BaseNode* pNode) override;

			/// @brief Gets an input node by name.
			/// @param name The name of the input node.
			/// @return A pointer to the input node, or nullptr if the node was not found.
			BaseNode* GetInputNode(const std::string& name);

			/// @brief Gets an input node by name (const version).
			/// @param name The name of the input node.
			/// @return A pointer to the input node, or nullptr if the node was not found.
			const BaseNode* GetInputNode(const std::string& name) const;

			/// @brief Gets a reference to the map of input nodes.
			/// @return A reference to the unordered_map of input nodes.
			SHADE_INLINE std::unordered_map<std::string, BaseNode*>& GetInputNodes() 
			{
				return m_InputNodes; 
			}

			/// @brief Gets a const reference to the map of input nodes.
			/// @return A const reference to the unordered_map of input nodes.
			SHADE_INLINE const std::unordered_map<std::string, BaseNode*>& GetInputNodes() const 
			{
				return m_InputNodes; 
			}

		private:
			/// @brief Evaluates the graph for the current frame.
			/// @param deltaTime The time elapsed since the last frame.
			virtual void Evaluate(const FrameTimer& deltaTime) override;

			/// @brief Map of input nodes by name.
			std::unordered_map<std::string, BaseNode*> m_InputNodes; 

		private:
			// Constructor for asset manager only
			/// @brief Constructor for creating an AnimationGraph with asset data.
			/// @param assetData A shared pointer to the asset data.
			/// @param lifeTime The lifetime management policy for the asset.
			/// @param behaviour The instantiation behavior for the asset.
			/// @param context A pointer to the GraphContext associated with this animation graph.
			AnimationGraph(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour, graphs::GraphContext* context);

			/// @brief Serializes the animation graph to the given output stream.
			/// @param stream The output stream to serialize to.
			/// @return The number of bytes written.
			std::size_t Serialize(std::ostream& stream) const;

			/// @brief Deserializes the animation graph from the given input stream.
			/// @param stream The input stream to deserialize from.
			/// @return The number of bytes read.
			std::size_t Deserialize(std::istream& stream);

		private:
			friend class Serializer;
		};
	}

	// Serialize AnimationGraph
	template<>
	SHADE_INLINE std::size_t shade::Serializer::Serialize(std::ostream& stream, const animation::AnimationGraph& graph, std::size_t)
	{
		return graph.Serialize(stream);
	}

	// Deserialize AnimationGraph
	template<>
	SHADE_INLINE std::size_t shade::Serializer::Deserialize(std::istream& stream, animation::AnimationGraph& graph, std::size_t)
	{
		return graph.Deserialize(stream);
	}

	// Serialize Asset<AnimationGraph>
	template<>
	SHADE_INLINE std::size_t shade::Serializer::Serialize(std::ostream& stream, const Asset<animation::AnimationGraph>& graph, std::size_t)
	{
		return graph->Serialize(stream);
	}

	// Deserialize Asset<AnimationGraph>
	template<>
	SHADE_INLINE std::size_t shade::Serializer::Deserialize(std::istream& stream, Asset<animation::AnimationGraph>& graph, std::size_t)
	{
		return graph->Deserialize(stream);
	}
}
