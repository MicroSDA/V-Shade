#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/graphs/nodes/BaseNode.h>
#include <shade/core/graphs/nodes/IntEqualsNode.h>
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

			enum class SyncStylePreferences
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
				SyncStylePreferences Style = SyncStylePreferences::Async;
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

			class  SHADE_API OutputTransitionNode : public graphs::BaseNode
			{
			public:
				OutputTransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode);
				virtual ~OutputTransitionNode() = default;

				void Evaluate(const FrameTimer& deltaTime) override {};
		
				bool ShouldTransit() const;
				float GetTransitionDuration();
				float GetTransitionAccumulator();
				std::vector<float>& GetCurveControllPoints() { return m_CurveControlPoints; }
				SyncPreferences& GetSyncPreferences() { return m_SyncPreferences; }
			private:
				void ResetTransitionAccumulator();
				void ProcessTransitionAccumulator(const FrameTimer& deltaTime);
			private:
				float m_TimeAccumulator = 0.f;
				std::vector<float> m_CurveControlPoints = {0.5f};
				SyncPreferences m_SyncPreferences;
			private:
				friend class StateMachineNode;
			};

			class SHADE_API TransitionNode : public graphs::BaseNode
			{
			public:
				struct Data
				{
					StateNode* SourceState			= nullptr;
					StateNode* DestinationState		= nullptr;
				};
			public:
				TransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode, const Data& data);
				virtual ~TransitionNode() = default;

				virtual void Initialize() override;

				Data& GetTransitionData();
				const Data& GetTransitionData() const;

				void Evaluate(const FrameTimer& deltaTime) override;

				SHADE_INLINE bool ShouldTransit() const
				{
					return GetRootNode()->As<OutputTransitionNode>().ShouldTransit();
				}
			private:
				Data m_TransitionData;
				OutputTransitionNode* m_pOutputTransitionNode = nullptr;
			};

			class SHADE_API StateNode : public graphs::BaseNode
			{
			public:
				
				StateNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode, const std::string& name);
				virtual ~StateNode() = default;

				virtual void Initialize() override;

				/// @brief 
				/// @return 
				virtual bool RemoveNode(BaseNode* pNode) override;

				void Evaluate(const FrameTimer& deltaTime) override;

				// TODO ! Need to check if this already exist transition !!
				TransitionNode* AddTransition(StateNode* destination);

				SHADE_INLINE std::vector<TransitionNode*>& GetTransitions() { return m_Transitions; }
				SHADE_INLINE const std::vector<TransitionNode*>& GetTransitions() const { return m_Transitions; }
				SHADE_INLINE void  SetTransitionSyncData(const TransitionSyncData& data) { m_TransitionSyncData = data; }
				SHADE_INLINE TransitionSyncData& GetTransitionSyncData() { return m_TransitionSyncData; }

				/*SHADE_INLINE Pose* GetOutPutPose()
				{
					return GetRootNode()->As<OutputPoseNode>().GetEndpoint<graphs::Connection::Input>(0)->As<NodeValueType::Pose>();
				}*/
				SHADE_INLINE const Pose* GetOutPutPose() const
				{
					return GetRootNode()->As<OutputPoseNode>().GetFinalPose();
				}
			private:
				std::vector<TransitionNode*> m_Transitions;
				TransitionNode* m_pActiveTransition = nullptr;
				OutputPoseNode* m_pOutPutPoseNode = nullptr;

				TransitionSyncData m_TransitionSyncData;
			};

			class SHADE_API StateMachineNode : public graphs::BaseNode
			{
			public:
				StateMachineNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode);
				virtual ~StateMachineNode() = default;

				/// @brief 
				/// @return 
				virtual bool RemoveNode(BaseNode* pNode) override;

			public:
				void Evaluate(const FrameTimer& deltaTime) override;
				StateNode* CreateState(const std::string& name);
			private:
				TransitionNode* m_pActiveTransition = nullptr;
			private:
				Pose* Transit(TransitionNode* pTransition, const FrameTimer& deltaTime);
				//1. Current(Active State)
				//2. Every State has to have output pose node
				//3. At end of state machine evaluate need to set output pose from active state to state machine
				// So Machine output pose should contain output value from active state 
			};
		}
	}
}