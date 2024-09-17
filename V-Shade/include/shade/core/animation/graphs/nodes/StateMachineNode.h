#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/graphs/nodes/BaseNode.h>
#include <shade/core/graphs/nodes/IntEqualsNode.h>
#include <shade/core/graphs/nodes/BoolNode.h>
#include <shade/core/animation/graphs/nodes/OutputPoseNode.h>
#include <shade/core/animation/graphs/nodes/BlendNode.h>
#include <shade/core/animation/graphs/nodes/PoseNode.h>
#include <shade/core/animation/graphs/nodes/AdditivePose.h>
#include <shade/core/graphs/nodes/FloatScaleRange.h>
#include <shade/core/graphs/nodes/Vec2FloatNode.h>

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
				SHADE_INLINE float GetTransitionDuration()
				{
					return GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Float>(1);
				}
				/// @brief Gets the transition duration.
				/// @return The transition duration.
				SHADE_INLINE float GetTransitionDuration() const
				{
					return GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Float>(1);
				}
				/// @brief Gets the current transition accumulator value.
				/// @return The current transition accumulator value.
				SHADE_INLINE float GetTransitionAccumulator()
				{
					return m_TimeAccumulator;
				}
				SHADE_INLINE float GetTransitionAccumulator() const
				{
					return m_TimeAccumulator;
				}

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
				SHADE_INLINE void ResetTransitionAccumulator()
				{
					m_TimeAccumulator = 0.f;
				}

				/// @brief Sets the transition accumulator to a specific time.
				/// @param time The time to set the accumulator to.
				SHADE_INLINE void SetTransitionAccumulator(float time)
				{
					m_TimeAccumulator = time;
				}

				/// @brief Processes the transition accumulator, advancing it based on deltaTime.
				/// @param deltaTime The time elapsed since the last frame.
				SHADE_INLINE void ProcessTransitionAccumulator(const FrameTimer& deltaTime)
				{
					m_TimeAccumulator += !m_IsReverse ? deltaTime.GetInSeconds<float>() : -deltaTime.GetInSeconds<float>();
				}

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

				virtual void SerializeBody(std::ostream& stream) const override;

				virtual void DeserializeBody(std::istream& stream) override;

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
				bool m_CanBeInterrupted = false;               ///< Indicates if the transition can be interrupted.

				virtual void SerializeBody(std::ostream& stream) const override;

				virtual void DeserializeBody(std::istream& stream) override;

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
				virtual SHADE_INLINE Pose* GetOutputPose()
				{
					return GetRootNode()->GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(0);
				}

				/// @brief Gets the output pose of the state (const version).
				/// @return Const pointer to the Pose object.
				virtual SHADE_INLINE const Pose* GetOutputPose() const
				{
					return GetRootNode()->As<OutputPoseNode>().GetFinalPose();
				}
			private:
				std::vector<TransitionNode*> m_Transitions;			///< List of transitions from this state.
				StateNode* m_pPreviousState = nullptr;				///< Pointer to the previous state.
				TransitionSyncData m_TransitionSyncData;			///< Synchronization data for the transition.

				virtual void Serialize(std::ostream& stream) const override;

				friend class StateMachineNode;
			};

	
			// @brief Represents a entry state node in an animation state machine.
			//1. Сделать EntryState как рут StateMachine, добавть метотд сет руут стейт
			//2. Добавить на вход State machine позу, если она не нулл птр, значит EntryState будет ее хранить
			//3. При переходе из EntryState в любую другую стейт будет бленд как и обычно, если pose не nullptr
			class SHADE_API EntryStateNode : public StateNode
			{
				NODE_STATIC_TYPE_HELPER(EntryStateNode)
			public:
				EntryStateNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode, const std::string& name = "Entry");
				virtual ~EntryStateNode() = default;
				/// @brief Evaluates the state logic.
				/// @param deltaTime The time elapsed since the last frame.
				void Evaluate(const FrameTimer& deltaTime) override;

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

				/// @brief Initializes the StateMachineNode.
				virtual void Initialize() override;

			public:
				/// @brief Evaluates the state machine logic.
				/// @param deltaTime The time elapsed since the last frame.
				void Evaluate(const FrameTimer& deltaTime) override;

				/// @brief Creates a new state within the state machine.
				/// @param name The name of the new state.
				/// @return Pointer to the created StateNode.
				StateNode* CreateState(const std::string& name);

				SHADE_INLINE void SetCurrentState(StateNode* pState)
				{
					pState->m_pPreviousState = m_pCurrentState;
					m_pCurrentState = pState;
				}
				SHADE_INLINE StateNode* GetCurrentState()
				{
					return m_pCurrentState;
				}
				SHADE_INLINE const StateNode* GetCurrentState() const
				{
					return m_pCurrentState;
				}
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
				bool	m_IsTransitionHasBeenInterrupted = false;    ///< Flag indicating if the transition was interrupted.
				void	HandlePoseBlending(StateNode* pState, const animation::Pose* statePose);
				bool	IsTransitionCompletedOrImmediate(const OutputTransitionNode& transition);
				void	ResetTransitionData(TransitionNode::Data& transitionData);
				Pose*	HandleTransitionInProcess(OutputTransitionNode& transition, TransitionNode::Data& transitionData, const FrameTimer& deltaTime);
				float	AdjustBlendFactorForEntryPoint(const OutputTransitionNode& transition, const StateNode* dstState, float blendFactor);
				Pose*	ProcessTransitionPoses(AnimationController* controller, const Asset<Skeleton>& skeleton, OutputTransitionNode& transition, TransitionNode::Data& transitionData, const animation::Pose* sPose, const animation::Pose* dPose, float blendFactor, const FrameTimer& deltaTime);
				void	AdjustMultipliersForSyncStyle(SyncStyle style, float& sMultiplier, float& dMultiplier);
				void SetTransitionSyncData(
					OutputTransitionNode& transition,
					TransitionNode::Data& transitionData, TransitionStatus status, SyncStyle style,
					float blendFactor, float sMultiplier, float dMultiplier,
					const animation::Pose* sPose, const animation::Pose* dPose);

				virtual void Serialize(std::ostream& stream) const override;
				virtual void Deserialize(std::istream& stream) override;
				StateNode* m_pCurrentState = nullptr;
			};
		}
	}
}
