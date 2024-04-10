#include "shade_pch.h"
#include "StateMachineNode.h"

#include <shade/core/event/Input.h>
#include <glfw/glfw3.h>

#include <shade/core/animation/graphs/AnimationGraph.h>

shade::animation::state_machine::TransitionNode::TransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, const Data& data) : BaseNode(context, identifier) , m_TransitionData(data)
{
	m_pOutputTransitionNode = CreateNode<OutputTransitionNode>();
	SetRootNode(m_pOutputTransitionNode);
}

shade::animation::state_machine::TransitionNode::~TransitionNode()
{
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

shade::animation::state_machine::StateNode::StateNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, const std::string& name) : BaseNode(context, identifier) ,m_Name(name)
{
	m_pOutPutPoseNode = CreateNode<OutputPoseNode>();
	// Not sure where there shoudl be blend node
	auto pose = CreateNode<PoseNode>();
	SetRootNode(m_pOutPutPoseNode);

	ConnectNodes(m_pOutPutPoseNode, 0, pose, 0); // has to be removed ! for test only
}

shade::animation::state_machine::StateNode::~StateNode()
{

}
void shade::animation::state_machine::StateNode::Evaluate(const FrameTimer& deltaTime)
{
	if (GetParrentRootGraph())
	{
		for (auto v : GetParrentRootGraph()->As<animation::AnimationGraph>().GetInputNodes())
		{
			//std::cout << (int)v.first->GetType() << std::endl;
		}
	}

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

		*&GetParrentGraph()->GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0) = GetRootNode()->As<OutputPoseNode>().GetEndpoint<graphs::Connection::Input>(0)->As<NodeValueType::Pose>();

		GetRootNode()->ProcessBranch(deltaTime);
	}
}

void shade::animation::state_machine::StateNode::Shutdown()
{
}

shade::animation::state_machine::StateMachineNode::StateMachineNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier) : BaseNode(context, identifier)
{
	REGISTER_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(nullptr);
}

shade::animation::state_machine::StateMachineNode::~StateMachineNode()
{
}

void shade::animation::state_machine::StateMachineNode::Evaluate(const FrameTimer& deltaTime)
{
	// Should be aka default state, because state machine has only states
	GetRootNode()->ProcessBranch(deltaTime);
}

shade::animation::state_machine::StateNode* shade::animation::state_machine::StateMachineNode::CreateState(const std::string& name)
{
	return CreateNode<StateNode>(name);
}

shade::animation::state_machine::OutputTransitionNode::OutputTransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier) : BaseNode(context, identifier)
{
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Bool>(false);
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Float>(0.0);
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Bool>(0.0);
}

shade::animation::state_machine::OutputTransitionNode::~OutputTransitionNode()
{

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
