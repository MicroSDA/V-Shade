#include "shade_pch.h"
#include "StateMachineNode.h"

#include <shade/core/event/Input.h>
#include <glfw/glfw3.h>

// TIP : Do not create child nodes in constructor !!!

shade::animation::state_machine::OutputTransitionNode::OutputTransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode)
	: BaseNode(context, identifier, pParentNode, "OutputTransition")
{
	m_IsRenamable = false;
	m_IsRemovable = false;
	m_CanBeOpen = false;

	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Bool>(false);
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Float>(1.0);
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Bool>(0.0);
}

void shade::animation::state_machine::OutputTransitionNode::Evaluate(const FrameTimer& deltaTime)
{
	//assert(false);
}

shade::animation::Pose* shade::animation::state_machine::OutputTransitionNode::Transit(StateNode* sourceState, StateNode* destinationState, const FrameTimer& deltaTime)
{
	// Но сначала нужно сделать эдитр что бы создавать ноды, удалять, делать и удалять конекшены
	// 1. Сделать синхронизацию, сних с сурс аниацией, синх с дестинейшен анимацией, синх в обе стороны, не сихн вообще
	// 2. Дать возможность ресетить анимацию, или проигрывать с того момента на котором оно остановилось
	// 3. Добавить оффсет по времени, если стоит инхронихация значит оффсет тоже нужно перемножить
	// 5. Проверить как будут работать рут вейльйус
	// 6. Создать рут велью и дать возможность создавать соответствующий нод, но само значение брать из глобал вельюс


	// If accumulator more than duration that means transition has been done, or duration is 0 so transition should be immediately
	if (GetTransitionAccumulator() > GetTransitionDuration() || !GetTransitionDuration())
	{
		// Reset transition data
		sourceState->SetTransitionSyncData(StateNode::TransitionSyncData{}); destinationState->SetTransitionSyncData(StateNode::TransitionSyncData{});
		// Reset accamulator
		ResetTransitionAccumulator();
	}
	else // We need make a transition
	{
		auto& controller	= GetGraphContext()->As<AnimationGraphContext>().Controller;
		auto& skeleton		= GetGraphContext()->As<AnimationGraphContext>().Skeleton;

		const animation::Pose* sPose	= sourceState->GetRootNode()->As<OutputPoseNode>().GetFinalPose();
		const animation::Pose* dPose	= destinationState->GetRootNode()->As<OutputPoseNode>().GetFinalPose();

		if (sPose && dPose)
		{
			float blendFactor = glm::clamp(GetTransitionAccumulator() / GetTransitionDuration(), 0.0f, 1.0f);

			if (IsSync())
			{
				float duration1 = sPose->GetDuration();
				float duration2 = dPose->GetDuration();

				float currentTime1 = sPose->GetCurrentPlayTime();
				float currentTime2 = dPose->GetCurrentPlayTime();


				float normalizedTime1 = currentTime1 / duration1;
				float normalizedTime2 = currentTime2 / duration2;

				float targetNormalizedTime = (normalizedTime1 * (1.0f - blendFactor)) + (normalizedTime2 * blendFactor);

				float targetTime1 = targetNormalizedTime * duration1;
				float targetTime2 = targetNormalizedTime * duration2;


				sourceState->SetTransitionSyncData({
					.PStateAnimationDuration = dPose->GetDuration(),
					.PStateAnimationCurrentPlayTime = dPose->GetCurrentPlayTime(),
					.CurrentTransitionTime = GetTransitionAccumulator(),
					.BlendFactor = blendFactor
					});

				destinationState->SetTransitionSyncData({
						.PStateAnimationDuration = sPose->GetDuration(),
						.PStateAnimationCurrentPlayTime = sPose->GetCurrentPlayTime(),
						.CurrentTransitionTime = GetTransitionAccumulator(),
						.BlendFactor = blendFactor
					});
			}

			destinationState->Evaluate(deltaTime); sourceState->Evaluate(deltaTime);

			ProcessTransitionAccumulator(deltaTime);

			return controller->Blend(skeleton, sPose, dPose, blendFactor, animation::BoneMask{ nullptr });
		}
	}

	return nullptr;

	if (GetTransitionAccumulator() > GetTransitionDuration() || GetTransitionDuration() == 0.f)
	{
		sourceState->SetTransitionSyncData(StateNode::TransitionSyncData{}); sourceState->SetTransitionSyncData(StateNode::TransitionSyncData{});
		m_TimeAccumulator = 0.f;
		return nullptr;
	}
	else
	{
		auto& controller = GetGraphContext()->As<AnimationGraphContext>().Controller;
		auto& skeleton = GetGraphContext()->As<AnimationGraphContext>().Skeleton;

		const animation::Pose* sourcePose = sourceState->GetRootNode()->As<OutputPoseNode>().GetFinalPose();
		const animation::Pose* destinationPose = nullptr;

		float blendFactor = glm::clamp(GetTransitionAccumulator() / GetTransitionDuration(), 0.0f, 1.0f);

		if (IsSync())
		{
			destinationState->SetTransitionSyncData({
				.PStateAnimationDuration = sourcePose->GetDuration(),
				.PStateAnimationCurrentPlayTime = sourcePose->GetCurrentPlayTime(),
				.CurrentTransitionTime = GetTransitionAccumulator(),
				.BlendFactor = blendFactor
				});

			destinationState->GetRootNode()->ProcessBranch(deltaTime);

			destinationPose = destinationState->GetRootNode()->As<OutputPoseNode>().GetFinalPose();

			destinationState->SetTransitionSyncData({
				.PStateAnimationDuration = destinationPose->GetDuration(),
				.PStateAnimationCurrentPlayTime = destinationPose->GetCurrentPlayTime(),
				.CurrentTransitionTime = GetTransitionAccumulator(),
				.BlendFactor = blendFactor
				});

			sourceState->GetRootNode()->ProcessBranch(deltaTime);
		}
		else
		{
			sourceState->GetRootNode()->ProcessBranch(deltaTime);
			destinationState->GetRootNode()->ProcessBranch(deltaTime);

			destinationPose = destinationState->GetRootNode()->As<OutputPoseNode>().GetFinalPose();
		}

		m_TimeAccumulator += deltaTime.GetInSeconds<float>();

		return controller->Blend(skeleton, sourcePose, destinationPose, blendFactor, animation::BoneMask{ nullptr });
	}
}

bool shade::animation::state_machine::OutputTransitionNode::ShouldTransit() const
{
	return GetEndpoint<graphs::Connection::Input>(0)->As<NodeValueType::Bool>();
}

float shade::animation::state_machine::OutputTransitionNode::GetTransitionDuration()
{
	return  GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Float>(1);
}

float shade::animation::state_machine::OutputTransitionNode::GetTransitionAccumulator()
{
	return m_TimeAccumulator;
}

void shade::animation::state_machine::OutputTransitionNode::ResetTransitionAccumulator()
{
	m_TimeAccumulator = 0.f;
}

void shade::animation::state_machine::OutputTransitionNode::ProcessTransitionAccumulator(const FrameTimer& deltaTime)
{
	m_TimeAccumulator += deltaTime.GetInSeconds<float>();
}

bool& shade::animation::state_machine::OutputTransitionNode::IsSync()
{
	return GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Bool>(2);
}


shade::animation::state_machine::TransitionNode::TransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode, const Data& data)
	: BaseNode(context, identifier, pParentNode), m_TransitionData(data)
{
}

void shade::animation::state_machine::TransitionNode::Initialize()
{
	SetName(GetTransitionData().SourceState->GetName() + " -> " +GetTransitionData().DestinationState->GetName());

	m_pOutputTransitionNode = CreateNode<OutputTransitionNode>();
	SetRootNode(m_pOutputTransitionNode);
}

shade::animation::state_machine::TransitionNode::Data& shade::animation::state_machine::TransitionNode::GetTransitionData()
{
	return m_TransitionData;
}

const shade::animation::state_machine::TransitionNode::Data& shade::animation::state_machine::TransitionNode::GetTransitionData() const
{
	return m_TransitionData;
}

void shade::animation::state_machine::TransitionNode::Evaluate(const FrameTimer& deltaTime)
{
	GetRootNode()->ProcessBranch(deltaTime);
}

shade::animation::state_machine::StateNode::StateNode(
	graphs::GraphContext* context,
	graphs::NodeIdentifier identifier,
	graphs::BaseNode* pParentNode,
	const std::string& name)
	:BaseNode(context, identifier, pParentNode, name)
{
}

void shade::animation::state_machine::StateNode::Initialize()
{
	m_pOutPutPoseNode = CreateNode<OutputPoseNode>();
	// Not sure where there shoudl be blend node
	auto pose = CreateNode<PoseNode>();
	SetRootNode(m_pOutPutPoseNode);

	ConnectNodes(m_pOutPutPoseNode, 0, pose, 0); // has to be removed ! for test only
	ProcessBranch(FrameTimer{});
}

bool shade::animation::state_machine::StateNode::RemoveNode(BaseNode* pNode)
{
	auto it = std::find(m_Transitions.begin(), m_Transitions.end(), pNode);

	// In case we are removing transition !
	if (it != m_Transitions.end())
	{
		m_Transitions.erase(it);
		return GetGraphContext()->RemoveNode(pNode);
	}
	else
	{
		return GetGraphContext()->RemoveNode(pNode);
	}
}

void shade::animation::state_machine::StateNode::Evaluate(const FrameTimer& deltaTime)
{
	GetRootNode()->ProcessBranch(deltaTime);
}

shade::animation::state_machine::TransitionNode* shade::animation::state_machine::StateNode::AddTransition(StateNode* destination)
{
	auto transition = CreateNode<TransitionNode>(TransitionNode::Data{ this, destination });
	return m_Transitions.emplace_back(transition);
}

shade::animation::state_machine::StateMachineNode::StateMachineNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode)
	: BaseNode(context, identifier, pParentNode, "State machine")
{
	REGISTER_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(nullptr);
}

bool shade::animation::state_machine::StateMachineNode::RemoveNode(BaseNode* pNode)
{
	for (BaseNode* node : GetInternalNodes())
	{
		StateNode& pState = node->As<StateNode>();
		auto it = pState.GetTransitions().begin();

		while (it != pState.GetTransitions().end())
		{
			it = std::find_if(pState.GetTransitions().begin(), pState.GetTransitions().end(), [pNode](const TransitionNode* pTransition)
				{
					return (pTransition->GetTransitionData().DestinationState == pNode);
				});

			if (it != pState.GetTransitions().end())
			{
				it = pState.GetTransitions().erase(it);
			}
			else
			{
				it++;
			}
		}
	}
	return GetGraphContext()->RemoveNode(pNode);
}

void shade::animation::state_machine::StateMachineNode::Evaluate(const FrameTimer& deltaTime)
{
	// TODO: Make transition interruptible
	// Should be aka default state, because state machine has only states
	if (StateNode* pState = &GetRootNode()->As<StateNode>())
	{
		if (m_pActiveTransition)
		{
			Pose* blendPose = m_pActiveTransition->GetRootNode()->As<OutputTransitionNode>().Transit(
				m_pActiveTransition->GetTransitionData().SourceState,
				m_pActiveTransition->GetTransitionData().DestinationState,
				deltaTime
			);

			// When transition active, set blended pose to state machine 
			if (blendPose)
			{
				GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, blendPose);
			}
			else
			{
				// When transition has been done, set current state and pose to state machine 
				SetRootNode(m_pActiveTransition->GetTransitionData().DestinationState);
				m_pActiveTransition = nullptr;
			}
		}
		else // If there's no active transition 
		{
			// Process current state branch
			pState->ProcessBranch(deltaTime);

			// Go through all state transitions
			for (TransitionNode* pTransition : pState->GetTransitions())
			{
				// Process transition 
				pTransition->ProcessBranch(deltaTime);

				// If we has to transit
				if (pTransition->ShouldTransit())
				{
					m_pActiveTransition = pTransition;
					return;
				}
			}

			// Set current pose to state machine output
			GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, pState->GetOutPutPose());
		}
	}
}

shade::animation::state_machine::StateNode* shade::animation::state_machine::StateMachineNode::CreateState(const std::string& name)
{
	auto state = CreateNode<StateNode>(name);
	if (GetRootNode() == nullptr) SetRootNode(state);
	return state;
}