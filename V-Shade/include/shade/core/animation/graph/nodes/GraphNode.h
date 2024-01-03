#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/time/Timer.h>
#include <shade/core/event/Event.h>
#include <shade/core/animation/graph/nodes/NodeValue.h>
#include <shade/core/animation/AnimationController.h>

#include <shade/utils/Logger.h>

namespace shade
{
	namespace animation
	{

#define REGISTER_INPUT_ENDPOINT(type, ...) INPUT.emplace_back(std::make_shared<NodeValue>())->Set<NodeValueType::##type>(__VA_ARGS__);
#define REGISTER_OUTPUT_ENDPOINT(type, ...) OUTPUT.emplace_back(std::make_shared<NodeValue>())->Set<NodeValueType::##type>(__VA_ARGS__);

#ifndef GET_INPUT_ENDPOINT
	#define GET_INPUT_ENDPOINT(type, idx) INPUT[idx]->Get<NodeValueType::##type>()
#endif // !GET_INPUT_ENDPOINT
#ifndef GET_OUTPUT_ENDPOINT
	#define GET_OUTPUT_ENDPOINT(type, idx) OUTPUT[idx]->Get<NodeValueType::##type>()
#endif // !GET_OUTPUT_ENDPOINT

		struct SHADE_API GraphContext final
		{
		public:
			Asset<Skeleton> Skeleton;
			mutable SharedPointer<AnimationController> Controller;
		};

		struct Connection
		{
			enum class Type : std::uint8_t
			{
				Input,
				Output
			};
		};
	
		class SHADE_API GraphNode 
		{
			SHADE_CAST_HELPER(GraphNode)
			
		public:

			using NodeIDX		= std::uint32_t;
			using EndpointIDX	= std::uint32_t;

			static constexpr NodeIDX NODE_NULL_IDX = ~0u;
		public:
			GraphNode(NodeIDX idx, const GraphContext& context);
			virtual ~GraphNode() noexcept = default;
		public:
			NodeIDX GetNodeIndex() const;

			virtual void Evaluate(const FrameTimer& delatTime) = 0;
			void ProcessBranch(const FrameTimer& delatTime);

			template<NodeValueType T>
			inline FromNodeValueTypeToType<T>::Type* GetInputEndpointRaw(EndpointIDX idx)  { return (idx < INPUT.size()) ? &INPUT.at(idx).get()->Get<T>() : nullptr; }
			template<NodeValueType T>
			inline FromNodeValueTypeToType<T>::Type* GetOutputEndpointRaw(EndpointIDX idx) { return (idx < OUTPUT.size()) ? &OUTPUT.at(idx).get()->Get<T>() : nullptr; }

			template<NodeValueType T>
			inline const FromNodeValueTypeToType<T>::Type* GetInputEndpointRaw(EndpointIDX idx) const { return (idx < INPUT.size()) ? &INPUT.at(idx).get()->Get<T>() : nullptr; }
			template<NodeValueType T>
			inline const FromNodeValueTypeToType<T>::Type* GetOutputEndpointRaw(EndpointIDX idx) const { return (idx < OUTPUT.size()) ? &OUTPUT.at(idx).get()->Get<T>() : nullptr; }

			std::vector<std::shared_ptr<NodeValue>>& GetInputEndpoints();
			const std::vector<std::shared_ptr<NodeValue>>& GetInputEndpoints() const;

			std::vector<std::shared_ptr<NodeValue>>& GetOutputEndpoints();
			const std::vector<std::shared_ptr<NodeValue>>& GetOutputEndpoints() const;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint)	 { assert(false && "In case if you are using this function you need to impliment it fist!"); };
			virtual void OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint) { assert(false && "In case if you are using this function you need to impliment it fist!"); };
		protected:
			inline std::shared_ptr<NodeValue>* GetInputEndpointWrapper(EndpointIDX idx) { return (idx < INPUT.size()) ? &INPUT.at(idx) : nullptr; }
			inline std::shared_ptr<NodeValue>* GetOutputEndpointWrapper(EndpointIDX idx) { return (idx < OUTPUT.size()) ? &OUTPUT.at(idx) : nullptr; }
			const GraphContext& GetGraphContext() const;
		private:
			NodeIDX m_NodeIdx;
			const GraphContext& m_rGraphContext;
			friend class AnimationGraph;
			// Using only inside AnimationGraph
			void AddChild(const SharedPointer<GraphNode>& node);
			void RemoveChild(const SharedPointer<GraphNode>& node);

		protected:
			std::vector<std::shared_ptr<NodeValue>> INPUT, OUTPUT;
			std::unordered_map<std::size_t, SharedPointer<GraphNode>> m_Children;
			
		};
	}	
}
