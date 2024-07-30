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
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Float>(0.0);
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
	// 4. Добавить Рут парент !
	// 5. Проверить как будут работать рут вейльйус
	// 6. Создать рут велью и дать возможность создавать соответствующий нод, но само значение брать из глобал вельюс


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
	SetName("Transition");

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
	if (m_pActiveTransition)
	{
		Pose* blendPose = m_pActiveTransition->GetRootNode()->As<OutputTransitionNode>().Transit(
			m_pActiveTransition->GetTransitionData().SourceState,
			m_pActiveTransition->GetTransitionData().DestinationState,
			deltaTime
		);

		if (blendPose)
		{
			*&GetParrentGraph()->GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0) = blendPose;
		}
		else
		{
			GetParrentGraph()->SetRootNode(m_pActiveTransition->GetTransitionData().DestinationState);
			m_pActiveTransition = nullptr;
		}
	}
	else
	{
		for (auto transition : GetTransitions())
		{
			transition->ProcessBranch(deltaTime);

			if (transition->GetRootNode()->As<OutputTransitionNode>().ShouldTransit())
			{
				m_pActiveTransition = transition;
			}
		}

		// Set Pose to state machine !
		*&GetParrentGraph()->GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0) = GetRootNode()->As<OutputPoseNode>().GetEndpoint<graphs::Connection::Input>(0)->As<NodeValueType::Pose>();

		GetRootNode()->ProcessBranch(deltaTime);
	}
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
	// Should be aka default state, because state machine has only states
	if (auto pRoot = GetRootNode())
	{
		SHADE_CORE_INFO("ROOT {0}", (int)pRoot);
		pRoot->ProcessBranch(deltaTime);
	}
}

shade::animation::state_machine::StateNode* shade::animation::state_machine::StateMachineNode::CreateState(const std::string& name)
{
	auto state = CreateNode<StateNode>(name);

	SHADE_CORE_INFO("Create State {0}", (int)state);

	if (GetRootNode() == nullptr)
	{
		SHADE_CORE_INFO("Root State {0}", (int)state);
		SetRootNode(state);
	}

	return state;
}

void shade::animation::state_machine::StateMachineNode::Initialize()
{
}
