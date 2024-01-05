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
        /// @brief Structure representing the context of a graph
        struct SHADE_API GraphContext final
        {
        public:
            ///@brief Asset for skeleton
            Asset<Skeleton> Skeleton; 
            ///@brief AnimationController
            mutable SharedPointer<AnimationController> Controller;
        };

        /// @brief Class representing a graph node
        class SHADE_API GraphNode
        {
            SHADE_CAST_HELPER(GraphNode)

        public:
            /// @brief Structure representing a connection type within a graph node
            struct Connection
            {
                enum Type : std::uint8_t
                {
                    Input,
                    Output,
                    MAX_ENUM
                };
            };

            /// @brief Type aliases for node and endpoint indices
            using NodeIDX = std::uint32_t;
            using EndpointIDX = std::uint32_t;

            /// @brief Constant for representing a null node index
            static constexpr NodeIDX NODE_NULL_IDX = ~0u;

        public:
            /// @brief Constructor for GraphNode
            /// @param idx The index of the graph node
            /// @param context The context of the graph
            GraphNode(NodeIDX idx, const GraphContext& context);

            /// @brief Virtual destructor for GraphNode
            virtual ~GraphNode() noexcept = default;

        public:
            /// @brief Getter for the node index
            /// @return The index of the graph node
            SHADE_INLINE NodeIDX GetNodeIndex() const { return m_NodeIdx; };

            /// @brief Virtual function for evaluating the node
            /// @param deltaTime The time elapsed since the last evaluation
            virtual void Evaluate(const FrameTimer& deltaTime) = 0;

            /// @brief Virtual function for handling connection events when a node is connected
            /// @param connectionType The type of the connection (Input or Output)
            /// @param type The type of the node value being connected
            /// @param endpoint The index of the endpoint being connected
            virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint)       { assert(false && "In case if you are using this function you need to impliment it fist!"); };;

            /// @brief Virtual function for handling connection events when a node is disconnected
            /// @param connectionType The type of the connection (Input or Output)
            /// @param type The type of the node value being disconnected
            /// @param endpoint The index of the endpoint being disconnected
            virtual void OnDisconnect(Connection::Type connectionType, NodeValueType type, EndpointIDX endpoint)    { assert(false && "In case if you are using this function you need to impliment it fist!"); };;

            /// @brief Template function for getting the value of a specific endpoint
            /// @tparam T The type of the connection (Input or Output)
            /// @param index The index of the endpoint
            /// @return The value of the specified endpoint
            template<typename Connection::Type T>
            SHADE_INLINE NodeValues::Value GetEndpoint(EndpointIDX index)
            {
                return ((index < m_Endpoints[static_cast<std::size_t>(T)].GetSize()) ? m_Endpoints[static_cast<std::size_t>(T)].At(index) : nullptr);
            }

            /// @brief Template function for getting the value of a specific endpoint (const version)
            /// @tparam T The type of the connection (Input or Output)
            /// @param index The index of the endpoint
            /// @return The value of the specified endpoint
            template<typename Connection::Type T>
            SHADE_INLINE const NodeValues::Value GetEndpoint(EndpointIDX index) const
            {
                return ((index < m_Endpoints[static_cast<std::size_t>(T)].GetSize()) ? m_Endpoints[static_cast<std::size_t>(T)].At(index) : NodeValues::Value());  
            }

            /// @brief Getter for all endpoints
            /// @return A reference to the array of endpoints
            SHADE_INLINE std::array<NodeValues, std::size_t(Connection::Type::MAX_ENUM)>& GetEndpoints() { return m_Endpoints; };

            /// @brief Const getter for all endpoints
            /// @return A const reference to the array of endpoints
            SHADE_INLINE const std::array<NodeValues, std::size_t(Connection::Type::MAX_ENUM)>& GetEndpoints() const { return m_Endpoints; };

            /// @brief Getter for the graph context
            /// @return A const reference to the graph context
            SHADE_INLINE const GraphContext& GetGraphContext() const { return m_rGraphContext; };

        private:
            /// @brief Function for processing the branch of nodes
            /// @param deltaTime The time elapsed since the last processing
            void ProcessBranch(const FrameTimer& deltaTime);

            /// @brief Function for adding a child node
            /// @param node The child node to be added
            void AddChild(const SharedPointer<GraphNode>& node);

            /// @brief Function for removing a child node
            /// @param node The child node to be removed
            void RemoveChild(const SharedPointer<GraphNode>& node);

        protected:
            NodeIDX m_NodeIdx;
            const GraphContext& m_rGraphContext;
            std::array<NodeValues, std::size_t(Connection::Type::MAX_ENUM)> m_Endpoints;
            std::unordered_map<std::size_t, SharedPointer<GraphNode>> m_Children;

        protected:
            /// @brief Template function for registering an endpoint of a specific type
            /// @tparam ConnectionType The type of the connection (Input or Output)
            /// @tparam ValueType The type of the node value to be registered
            /// @tparam Args Additional arguments for value initialization
            /// @return The index of the registered endpoint
            template<typename Connection::Type ConnectionType, typename NodeValueType ValueType, typename... Args>
                requires IsNodeValueType<typename FromNodeValueTypeToType<ValueType>::Type>
            SHADE_INLINE EndpointIDX REGISTER_ENDPOINT(Args&&... args)
            {
                return m_Endpoints[static_cast<std::size_t>(ConnectionType)].Emplace<ValueType>(std::forward<Args>(args)...);
            }

            /// @brief Template function for removing an endpoint
            /// @tparam ConnectionType The type of the connection (Input or Output)
            template<typename Connection::Type ConnectionType>
            SHADE_INLINE void REMOVE_ENDPOINT(EndpointIDX index)
            {
                m_Endpoints[static_cast<std::size_t>(ConnectionType)].Remove(index);
            }

            /// @brief Template function for getting the value of a specific endpoint
            /// @tparam ConnectionType The type of the connection (Input or Output)
            /// @tparam ValueType The type of the node value to be retrieved
            /// @tparam Args Additional arguments for value initialization
            /// @return A reference to the retrieved endpoint value
            template<typename Connection::Type ConnectionType, typename NodeValueType ValueType, typename... Args> 
                requires IsNodeValueType<typename FromNodeValueTypeToType<ValueType>::Type>
            typename FromNodeValueTypeToType<ValueType>::Type& GET_ENDPOINT(EndpointIDX index, Args&&... args)
            {
                if constexpr (sizeof...(args) > 0)
                    m_Endpoints[static_cast<std::size_t>(ConnectionType)].At(index)->Initialize<ValueType>(std::forward<Args>(args)...);

                return m_Endpoints[static_cast<std::size_t>(ConnectionType)].At(index)->As<ValueType>();
            }

            /// @brief Template function for getting the value of a specific endpoint (const version)
            /// @tparam ConnectionType The type of the connection (Input or Output)
            /// @tparam ValueType The type of the node value to be retrieved
            /// @return A const reference to the retrieved endpoint value
            template<typename Connection::Type ConnectionType, typename NodeValueType ValueType>
                requires IsNodeValueType<typename FromNodeValueTypeToType<ValueType>::Type>
            const typename FromNodeValueTypeToType<ValueType>::Type& GET_ENDPOINT(EndpointIDX index) const
            {
                return m_Endpoints[static_cast<std::size_t>(ConnectionType)].At(index)->As<ValueType>();
            }
        private:
            friend class AnimationGraph;

            /// @brief Helper function for getting the pointer to an endpoint value
            /// @tparam T The type of the connection (Input or Output)
            /// @param index The index of the endpoint
            /// @return A pointer to the endpoint value or nullptr if index is out of bounds
            template<typename Connection::Type T>
            SHADE_INLINE NodeValues::Value* __GetEndpoint(EndpointIDX index)
            {
                return ((index < m_Endpoints[static_cast<std::size_t>(T)].GetSize()) ? &m_Endpoints[static_cast<std::size_t>(T)].At(index) : nullptr);
            }

            /// @brief Helper function for getting the const pointer to an endpoint value
            /// @tparam T The type of the connection (Input or Output)
            /// @param index The index of the endpoint
            /// @return A const pointer to the endpoint value or nullptr if index is out of bounds
            template<typename Connection::Type T>
            SHADE_INLINE const NodeValues::Value* __GetEndpoint(EndpointIDX index) const
            {
                return ((index < m_Endpoints[static_cast<std::size_t>(T)].GetSize()) ? &m_Endpoints[static_cast<std::size_t>(T)].At(index) : nullptr);
            }
        };
    }
}
