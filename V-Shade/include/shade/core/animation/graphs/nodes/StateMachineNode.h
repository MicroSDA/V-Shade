//#pragma once
//#include <shade/config/ShadeAPI.h>
//#include <shade/core/memory/Memory.h>
//#include <shade/core/graphs/nodes/BaseNode.h>
//#include <shade/core/graphs/nodes/IntEqualsNode.h>
//#include <shade/core/graphs/nodes/BoolNode.h>
//#include <shade/core/animation/graphs/nodes/OutputPoseNode.h>
//#include <shade/core/animation/graphs/nodes/BlendNode2D.h>
//#include <shade/core/animation/graphs/nodes/PoseNode.h>
//
//namespace shade
//{
//	namespace animation
//	{
//		namespace state_machine
//		{
//			class StateNode;
//
//			enum class TransitionStatus
//			{
//				Start = 0,
//				InProcess,
//				End
//			};
//
//			enum class SyncStyle
//			{
//				Async = 0,
//				// Freeze source animation
//				SourceFrozen,
//				// Sync destination animation time based on source animation
//				SourceToDestinationTimeSync,
//				// Sync source animation time based on destination animation
//				DestinationToSourceTimeSync,
//				// Sync both animations based on their own time
//				DestinationAndSourceTimeSync,
//				// Sync by keyframes
//				KeyFrameSync
//			};
//
//			struct SyncPreferences
//			{
//				SyncStyle Style = SyncStyle::Async;
//				bool  ResetFromStart = false;
//				float Offset = 0.f;
//			};
//
//			struct TransitionSyncData
//			{
//				SyncPreferences Preferences;
//				float BlendFactor = 0.f;
//				float CurrentTransitionTime = 0.f;
//				float TimeMultiplier = 1.f;
//				// Describes another animation 
//				float PStateAnimationDuration = 0.f;
//				float PStateAnimationCurrentPlayTime = 0.f;
//				TransitionStatus Status = TransitionStatus::End;
//			};
//
//			class  SHADE_API OutputTransitionNode : public graphs::BaseNode
//			{
//			public:
//				OutputTransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode);
//				virtual ~OutputTransitionNode() = default;
//
//				void Evaluate(const FrameTimer& deltaTime) override {};
//		
//				bool ShouldTransit() const;
//				bool IsReverse() const { return m_IsReverse; } 
//				float GetTransitionDuration();
//				float GetTransitionAccumulator();
//				std::vector<float>& GetCurveControllPoints() { return m_CurveControlPoints; }
//				SyncStyle& GetSynStyle() { return m_SyncStyle; }
//			private:
//				void ResetTransitionAccumulator();
//				void SetTransitionAccumulator(float time);
//				void ProcessTransitionAccumulator(const FrameTimer& deltaTime);
//				void SetTransitionReverse(bool set);
//				void ReverseTransition();
//			private:
//				float m_TimeAccumulator = 0.f;
//				std::vector<float> m_CurveControlPoints = {0.5f};
//				SyncStyle m_SyncStyle = SyncStyle::Async;
//				bool m_IsReverse = false;
//			private:
//				friend class StateMachineNode;
//			};
//
//			class SHADE_API TransitionNode : public graphs::BaseNode
//			{
//			public:
//				struct Data
//				{
//					StateNode* SourceState			= nullptr;
//					StateNode* DestinationState		= nullptr;
//				};
//			public:
//				TransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode, const Data& data);
//				virtual ~TransitionNode() = default;
//
//				virtual void Initialize() override;
//
//				Data& GetTransitionData();
//				const Data& GetTransitionData() const;
//
//				void Evaluate(const FrameTimer& deltaTime) override;
//
//				SHADE_INLINE bool ShouldTransit() const
//				{
//					return GetRootNode()->As<OutputTransitionNode>().ShouldTransit();
//				}
//				SHADE_INLINE bool CanBeInterrupted() const
//				{
//					return m_CanBeInterrupted;
//				}
//				SHADE_INLINE void SetInterrupted(bool is)
//				{
//					m_CanBeInterrupted = is;
//				}
//			private:
//				Data m_TransitionData;
//				OutputTransitionNode* m_pOutputTransitionNode = nullptr;
//				bool m_CanBeInterrupted = false;
//				
//			};
//
//			class SHADE_API StateNode : public graphs::BaseNode
//			{
//			public:
//				
//				StateNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode, const std::string& name);
//				virtual ~StateNode() = default;
//
//				virtual void Initialize() override;
//
//				/// @brief 
//				/// @return 
//				virtual bool RemoveNode(BaseNode* pNode) override;
//
//				void Evaluate(const FrameTimer& deltaTime) override;
//
//				// TODO ! Need to check if this already exist transition !!
//				TransitionNode* AddTransition(StateNode* destination);
//				bool RemoveTransition(TransitionNode* transition);
//
//				SHADE_INLINE std::vector<TransitionNode*>& GetTransitions() { return m_Transitions; }
//				SHADE_INLINE const std::vector<TransitionNode*>& GetTransitions() const { return m_Transitions; }
//				SHADE_INLINE void  SetTransitionSyncData(const TransitionSyncData& data) { m_TransitionSyncData = data; }
//				SHADE_INLINE TransitionSyncData& GetTransitionSyncData() { return m_TransitionSyncData; }
//
//				SHADE_INLINE Pose* GetOutPutPose()
//				{
//					return GetRootNode()->As<OutputPoseNode>().GetEndpoint<graphs::Connection::Input>(0)->As<NodeValueType::Pose>();
//				}
//				SHADE_INLINE const Pose* GetOutPutPose() const
//				{
//					return GetRootNode()->As<OutputPoseNode>().GetFinalPose();
//				}
//			private:
//				std::vector<TransitionNode*> m_Transitions;
//				TransitionNode* m_pActiveTransition = nullptr;
//				OutputPoseNode* m_pOutPutPoseNode = nullptr;
//
//				TransitionSyncData m_TransitionSyncData;
//			};
//
//			class SHADE_API StateMachineNode : public graphs::BaseNode
//			{
//			public:
//				StateMachineNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode);
//				virtual ~StateMachineNode() = default;
//
//				/// @brief 
//				/// @return 
//				virtual bool RemoveNode(BaseNode* pNode) override;
//
//			public:
//				void Evaluate(const FrameTimer& deltaTime) override;
//				StateNode* CreateState(const std::string& name);
//			private:
//				TransitionNode* m_pActiveTransition = nullptr;
//			private:
//				Pose* Transit(TransitionNode* pTransition, const FrameTimer& deltaTime, Pose* pPTPose = nullptr);
//			private:
//			;	bool  m_IsTransitionHasBeenInterrupted = false;
//			};
//		}
//	}
//}

#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/graphs/nodes/BaseNode.h>
#include <shade/core/graphs/nodes/IntEqualsNode.h>
#include <shade/core/graphs/nodes/BoolNode.h>
#include <shade/core/animation/graphs/nodes/OutputPoseNode.h>
#include <shade/core/animation/graphs/nodes/BlendNode2D.h>
#include <shade/core/animation/graphs/nodes/PoseNode.h>

namespace shade
{
	namespace animation
	{
		namespace state_machine
		{
			class StateNode;

			enum class TransitionStatus
			{
				Start = 0,
				InProcess,
				End
			};

			enum class SyncStyle : std::uint8_t
			{
				Async = 0,
				// Freeze source animation
				SourceFrozen,
				// Sync destination animation time based on source animation
				SourceToDestinationTimeSync,
				// Sync source animation time based on destination animation
				DestinationToSourceTimeSync,
				// Sync both animations based on their own time
				DestinationAndSourceTimeSync,
				// Sync by keyframes
				KeyFrameSync
			};

			struct SyncPreferences
			{
				SyncStyle Style = SyncStyle::Async;
				bool  ResetFromStart = false;
				float Offset = 0.f;
			};

			struct TransitionSyncData
			{
				SyncPreferences Preferences;
				float BlendFactor = 0.f;
				float CurrentTransitionTime = 0.f;
				float TimeMultiplier = 1.f;
				// Describes another animation 
				float PStateAnimationDuration = 0.f;
				float PStateAnimationCurrentPlayTime = 0.f;
				TransitionStatus Status = TransitionStatus::End;
			};

			/// @brief Represents the output node for a transition in an animation state machine.
			class SHADE_API OutputTransitionNode : public graphs::BaseNode
			{
				NODE_STATIC_TYPE_HELPER(OutputTransitionNode)

			public:
				/// @brief Constructor for OutputTransitionNode.
				/// @param context Pointer to the graph context.
				/// @param identifier Unique node identifier.
				/// @param pParentNode Pointer to the parent node.
				OutputTransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode);

				/// @brief Destructor for OutputTransitionNode.
				virtual ~OutputTransitionNode() = default;

				/// @brief Evaluates the node logic.
				/// @param deltaTime The time elapsed since the last frame.
				void Evaluate(const FrameTimer& deltaTime) override {};

				/// @brief Checks if the transition should occur.
				/// @return True if the transition should occur, false otherwise.
				bool ShouldTransit() const;

				/// @brief Checks if the transition is in reverse.
				/// @return True if the transition is in reverse, false otherwise.
				SHADE_INLINE bool IsReverse() const 
				{
					return m_IsReverse; 
				}

				/// @brief Gets the transition duration.
				/// @return The transition duration.
				float GetTransitionDuration();

				/// @brief Gets the current transition accumulator value.
				/// @return The current transition accumulator value.
				float GetTransitionAccumulator();

				/// @brief Gets the control points of the curve used in the transition.
				/// @return A reference to the vector of curve control points.
				SHADE_INLINE std::vector<float>& GetCurveControllPoints() 
				{
					return m_CurveControlPoints;
				}

				/// @brief Gets the synchronization style of the transition.
				/// @return A reference to the SyncStyle enum.
				SHADE_INLINE SyncStyle& GetSynStyle() 
				{ 
					return m_SyncStyle; 
				}
			private:
				/// @brief Resets the transition accumulator to its initial value.
				void ResetTransitionAccumulator();

				/// @brief Sets the transition accumulator to a specific time.
				/// @param time The time to set the accumulator to.
				void SetTransitionAccumulator(float time);

				/// @brief Processes the transition accumulator, advancing it based on deltaTime.
				/// @param deltaTime The time elapsed since the last frame.
				void ProcessTransitionAccumulator(const FrameTimer& deltaTime);

				/// @brief Sets whether the transition should be in reverse.
				/// @param set Boolean indicating if the transition should be reversed.
				void SetTransitionReverse(bool set);

				/// @brief Reverses the transition process.
				void ReverseTransition();
			private:
				float m_TimeAccumulator = 0.f;                      ///< Accumulator for the transition time.
				std::vector<float> m_CurveControlPoints = { 0.5f };  ///< Control points for the transition curve.
				SyncStyle m_SyncStyle = SyncStyle::Async;          ///< Synchronization style for the transition.
				bool m_IsReverse = false;                          ///< Indicates if the transition is reversed.
			private:
				friend class StateMachineNode;                      ///< Friend class for state machine integration.

				virtual std::size_t SerializeBody(std::ostream& stream) const override;

				virtual std::size_t DeserializeBody(std::istream& stream) override;

			};

			/// @brief Represents a transition between states in an animation state machine.
			class SHADE_API TransitionNode : public graphs::BaseNode
			{
				NODE_STATIC_TYPE_HELPER(TransitionNode)

			public:
				struct Data
				{
					StateNode* SourceState = nullptr;
					StateNode* DestinationState = nullptr;
				};
			public:
				/// @brief Constructor for TransitionNode.
				/// @param context Pointer to the graph context.
				/// @param identifier Unique node identifier.
				/// @param pParentNode Pointer to the parent node.
				/// @param data Transition data containing source and destination states.
				TransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode, const Data& data);

				/// @brief Constructor for TransitionNode.
				/// @param context Pointer to the graph context.
				/// @param identifier Unique node identifier.
				/// @param pParentNode Pointer to the parent node.
				TransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode);

				/// @brief Destructor for TransitionNode.
				virtual ~TransitionNode() = default;

				/// @brief Initializes the TransitionNode.
				virtual void Initialize() override;

				/// @brief Gets the transition data.
				/// @return A reference to the transition data.
				Data& GetTransitionData();

				/// @brief Gets the transition data (const version).
				/// @return A const reference to the transition data.
				const Data& GetTransitionData() const;

				/// @brief Sets the transition data.
				/// @param data the transition data.
				void SetTransitionData(const Data& data);

				/// @brief Evaluates the transition logic.
				/// @param deltaTime The time elapsed since the last frame.
				void Evaluate(const FrameTimer& deltaTime) override;

				/// @brief Checks if the transition should occur.
				/// @return True if the transition should occur, false otherwise.
				SHADE_INLINE bool ShouldTransit() const
				{
					return GetRootNode()->As<OutputTransitionNode>().ShouldTransit();
				}

				/// @brief Checks if the transition can be interrupted.
				/// @return True if the transition can be interrupted, false otherwise.
				SHADE_INLINE bool CanBeInterrupted() const
				{
					return m_CanBeInterrupted;
				}

				/// @brief Sets whether the transition can be interrupted.
				/// @param is Boolean indicating if the transition can be interrupted.
				SHADE_INLINE void SetInterrupted(bool is)
				{
					m_CanBeInterrupted = is;
				}
			private:
				Data m_TransitionData;                         ///< Data containing source and destination states.
				//OutputTransitionNode* m_pOutputTransitionNode = nullptr; ///< Pointer to the output transition node.
				bool m_CanBeInterrupted = false;               ///< Indicates if the transition can be interrupted.

				virtual std::size_t SerializeBody(std::ostream& stream) const override;

				virtual std::size_t DeserializeBody(std::istream& stream) override;

			};

			/// @brief Represents a state node in an animation state machine.
			class SHADE_API StateNode : public graphs::BaseNode
			{
				NODE_STATIC_TYPE_HELPER(StateNode)

			public:
				/// @brief Constructor for StateNode.
				/// @param context Pointer to the graph context.
				/// @param identifier Unique node identifier.
				/// @param pParentNode Pointer to the parent node.
				/// @param name Name of the state.
				StateNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode, const std::string& name = "State");

				/// @brief Destructor for StateNode.
				virtual ~StateNode() = default;

				/// @brief Initializes the StateNode.
				virtual void Initialize() override;

				/// @brief Removes a node from the state.
				/// @param pNode Pointer to the node to be removed.
				/// @return True if the node was removed successfully, false otherwise.
				virtual bool RemoveNode(BaseNode* pNode) override;

				/// @brief Evaluates the state logic.
				/// @param deltaTime The time elapsed since the last frame.
				void Evaluate(const FrameTimer& deltaTime) override;

				/// @brief Adds an existing transition.
				/// @param transitions Pointer to the transition node.
				/// @return Pointer to the added TransitionNode.
				TransitionNode* AddTransition(TransitionNode* transition);

				/// @brief Create a transition from this state to a destination state.
				/// @param destination Pointer to the destination state node.
				/// @return Pointer to the created TransitionNode.
				TransitionNode* CreateTransition(StateNode* destination);

				/// @brief Create a transition from this state to a destination state.
				/// @param destination Pointer to the destination state node.
				/// @return Pointer to the created TransitionNode.
				TransitionNode* EmplaceTransition(StateNode* destination);

				/// @brief Removes a transition from this state.
				/// @param transition Pointer to the transition node to be removed.
				/// @return True if the transition was removed successfully, false otherwise.
				bool RemoveTransition(TransitionNode* transition);

				/// @brief Gets the list of transitions from this state.
				/// @return Reference to the vector of TransitionNode pointers.
				SHADE_INLINE std::vector<TransitionNode*>& GetTransitions()
				{
					return m_Transitions; 
				}

				/// @brief Gets the list of transitions from this state (const version).
				/// @return Const reference to the vector of TransitionNode pointers.
				SHADE_INLINE const std::vector<TransitionNode*>& GetTransitions() const 
				{
					return m_Transitions;
				}

				/// @brief Sets the synchronization data for the transition.
				/// @param data Reference to the TransitionSyncData to set.
				SHADE_INLINE void  SetTransitionSyncData(const TransitionSyncData& data) 
				{ 
					m_TransitionSyncData = data;
				}

				/// @brief Gets the synchronization data for the transition.
				/// @return Reference to the TransitionSyncData.
				SHADE_INLINE TransitionSyncData& GetTransitionSyncData() 
				{ 
					return m_TransitionSyncData; 
				}

				/// @brief Gets the output pose of the state.
				/// @return Pointer to the Pose object.
				SHADE_INLINE Pose* GetOutputPose()
				{
					return GetRootNode()->GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(0);
				}

				/// @brief Gets the output pose of the state (const version).
				/// @return Const pointer to the Pose object.
				SHADE_INLINE const Pose* GetOutputPose() const
				{
					return GetRootNode()->As<OutputPoseNode>().GetFinalPose();
				}
			private:
				std::vector<TransitionNode*> m_Transitions;       ///< List of transitions from this state.
				TransitionNode* m_pActiveTransition = nullptr;    ///< Pointer to the currently active transition.
				TransitionSyncData m_TransitionSyncData;          ///< Synchronization data for the transition.

				virtual std::size_t Serialize(std::ostream& stream) const override;
			};

			/// @brief Represents a state machine node in an animation graph.
			class SHADE_API StateMachineNode : public graphs::BaseNode
			{
				NODE_STATIC_TYPE_HELPER(StateMachineNode)

			public:
				/// @brief Constructor for StateMachineNode.
				/// @param context Pointer to the graph context.
				/// @param identifier Unique node identifier.
				/// @param pParentNode Pointer to the parent node.
				StateMachineNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode);

				/// @brief Destructor for StateMachineNode.
				virtual ~StateMachineNode() = default;

				/// @brief Removes a node from the state machine.
				/// @param pNode Pointer to the node to be removed.
				/// @return True if the node was removed successfully, false otherwise.
				virtual bool RemoveNode(BaseNode* pNode) override;

			public:
				/// @brief Evaluates the state machine logic.
				/// @param deltaTime The time elapsed since the last frame.
				void Evaluate(const FrameTimer& deltaTime) override;

				/// @brief Creates a new state within the state machine.
				/// @param name The name of the new state.
				/// @return Pointer to the created StateNode.
				StateNode* CreateState(const std::string& name);
			private:
				TransitionNode* m_pActiveTransition = nullptr;    ///< Pointer to the currently active transition.
			private:
				/// @brief Handles the transition between states within the state machine.
				/// @param pTransition Pointer to the TransitionNode to process.
				/// @param deltaTime The time elapsed since the last frame.
				/// @param pPTPose Optional pointer to a Pose object.
				/// @return Pointer to the Pose after transition.
				Pose* Transit(TransitionNode* pTransition, const FrameTimer& deltaTime, Pose* pPTPose = nullptr);
			private:
				bool m_IsTransitionHasBeenInterrupted = false;    ///< Flag indicating if the transition was interrupted.

				virtual std::size_t Serialize(std::ostream& stream) const override;
				virtual std::size_t Deserialize(std::istream& stream) override;

			};
		}
	}
}
