#include "shade_pch.h"
#include "StateMachineNode.h"

#include <shade/core/event/Input.h>
#include <glfw/glfw3.h>

// TIP : Do not create child nodes in constructor !!!

shade::animation::state_machine::OutputTransitionNode::OutputTransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode)
	: BaseNode(context, identifier, pParentNode, "Output transition")
{
	m_IsRenamable = false;
	m_IsRemovable = false;
	m_CanBeOpen = false;

	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Bool>(false);
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Float>(1.0);
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Float>(0.0);
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Bool>(true);
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Bool>(true);
}

bool shade::animation::state_machine::OutputTransitionNode::ShouldTransit() const
{
	return GetEndpoint<graphs::Connection::Input>(0)->As<NodeValueType::Bool>();
}

void shade::animation::state_machine::OutputTransitionNode::SetTransitionReverse(bool set)
{
	m_IsReverse = set;
}

void shade::animation::state_machine::OutputTransitionNode::ReverseTransition()
{
	m_IsReverse = !m_IsReverse;
}

std::size_t shade::animation::state_machine::OutputTransitionNode::SerializeBody(std::ostream& stream) const
{
	std::size_t size = Serializer::Serialize(stream, m_CurveControlPoints);
	size += Serializer::Serialize(stream, m_SyncStyle);
	return size;
}

std::size_t shade::animation::state_machine::OutputTransitionNode::DeserializeBody(std::istream& stream)
{
	std::size_t size = Serializer::Deserialize(stream, m_CurveControlPoints);
	size += Serializer::Deserialize(stream, m_SyncStyle);
	return size;
}

shade::animation::state_machine::TransitionNode::TransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode, const Data& data)
	: BaseNode(context, identifier, pParentNode), m_TransitionData(data)
{
}

shade::animation::state_machine::TransitionNode::TransitionNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode)
	: BaseNode(context, identifier, pParentNode)
{
}

void shade::animation::state_machine::TransitionNode::Initialize()
{
	//SetName(GetTransitionData().SourceState->GetName() + " -> " +GetTransitionData().DestinationState->GetName());
	SetRootNode(CreateNode<OutputTransitionNode>());
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

std::size_t shade::animation::state_machine::TransitionNode::SerializeBody(std::ostream& stream) const
{
	return Serializer::Serialize(stream, m_TransitionData.DestinationState->GetNodeIdentifier());
}

std::size_t shade::animation::state_machine::TransitionNode::DeserializeBody(std::istream& stream)
{
	graphs::NodeIdentifier id; std::size_t size = Serializer::Deserialize(stream, id);
	BaseNode* pParrent = GetParrentGraph()->GetParrentGraph();
	m_TransitionData.DestinationState = &GetGraphContext()->FindInternalNode(pParrent, id)->As<StateNode>();
	return size;
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
	SetRootNode(CreateNode<OutputPoseNode>());
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

shade::animation::state_machine::EntryStateNode::EntryStateNode(
	shade::graphs::GraphContext* context,
	graphs::NodeIdentifier identifier,
	graphs::BaseNode* pParentNode,
	const std::string& name)
	: StateNode(context, identifier, pParentNode, name)
{
	m_IsRenamable = false;
	m_IsRemovable = false;
	m_CanBeOpen = false;
}

void shade::animation::state_machine::EntryStateNode::Evaluate(const FrameTimer& deltaTime)
{
	GetRootNode()->As<OutputPoseNode>().GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(0,
		GetParrentGraph()->As<StateMachineNode>().GET_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(0));
}

shade::animation::state_machine::TransitionNode* shade::animation::state_machine::StateNode::EmplaceTransition(StateNode* destination)
{
	auto it = std::find_if(m_Transitions.begin(), m_Transitions.end(), [this, destination](const TransitionNode* tr)
		{
			return tr->GetTransitionData().SourceState == this && tr->GetTransitionData().DestinationState == destination;
		});

	if (it != m_Transitions.end())
		return nullptr;
	else
	{
		auto transition = CreateNode<TransitionNode>(TransitionNode::Data{ this, destination });
		return m_Transitions.emplace_back(transition);
	}
}

shade::animation::state_machine::TransitionNode* shade::animation::state_machine::StateNode::AddTransition(TransitionNode* transition)
{
	auto it = std::find_if(m_Transitions.begin(), m_Transitions.end(), [this, transition](const TransitionNode* tr)
		{
			return tr->GetTransitionData().SourceState == this && tr->GetTransitionData().DestinationState == transition->GetTransitionData().DestinationState;
		});

	if (it != m_Transitions.end())
		return nullptr;
	else
	{
		return m_Transitions.emplace_back(transition);
	}
}

shade::animation::state_machine::TransitionNode* shade::animation::state_machine::StateNode::CreateTransition(StateNode* destination)
{
	auto it = std::find_if(m_Transitions.begin(), m_Transitions.end(), [this, destination](const TransitionNode* tr)
		{
			return tr->GetTransitionData().SourceState == this && tr->GetTransitionData().DestinationState == destination;
		});

	if (it != m_Transitions.end())
		return nullptr;
	else
	{
		return CreateNode<TransitionNode>(TransitionNode::Data{ this, destination });
	}
}

bool shade::animation::state_machine::StateNode::RemoveTransition(TransitionNode* transition)
{
	auto it = std::find_if(m_Transitions.begin(), m_Transitions.end(), [this, transition](const TransitionNode* tr)
		{
			return tr == transition;
		});

	if (it != m_Transitions.end())
	{
		m_Transitions.erase(it); return true;
	}
	return false;
}

shade::animation::state_machine::StateMachineNode::StateMachineNode(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode)
	: BaseNode(context, identifier, pParentNode, "State machine")
{
	REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(nullptr);

	REGISTER_ENDPOINT<graphs::Connection::Output, NodeValueType::Float>(0.0);
	REGISTER_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(nullptr);  // transition blend !	
}

shade::animation::state_machine::StateNode* shade::animation::state_machine::StateMachineNode::CreateState(const std::string& name)
{
	auto state = CreateNode<StateNode>(name);
	if (m_pCurrentState == nullptr) m_pCurrentState = state;
	return state;
}

bool shade::animation::state_machine::StateMachineNode::RemoveNode(BaseNode* pNode)
{
	for (BaseNode* node : GetInternalNodes())
	{
		StateNode& pState = node->As<StateNode>();
		auto it = pState.GetTransitions().begin();

		while (it != pState.GetTransitions().end())
		{
			auto state = std::find_if(pState.GetTransitions().begin(), pState.GetTransitions().end(), [pNode](const TransitionNode* pTransition)
				{
					return (pTransition->GetTransitionData().DestinationState == pNode);
				});

			if (state != pState.GetTransitions().end())
			{
				it = pState.GetTransitions().erase(state);
			}
			else
			{
				it++;
			}
		}
	}
	if (pNode == GetRootNode())
	{
		SetRootNode(nullptr);
	}
	// Need to set new root 
	return GetGraphContext()->RemoveNode(pNode);
}

void shade::animation::state_machine::StateMachineNode::Initialize()
{
	SetRootNode(CreateNode<EntryStateNode>());
	m_pCurrentState = &GetRootNode()->As<StateNode>();
}

//void shade::animation::state_machine::StateMachineNode::Evaluate(const FrameTimer& deltaTime)
//{
//	if (StateNode* pState = m_pCurrentState)
//	{
//		if (m_pActiveTransition)
//		{
//			OutputTransitionNode& transition = m_pActiveTransition->GetRootNode()->As<OutputTransitionNode>();
//				
//			if (m_pActiveTransition->CanBeInterrupted())
//			{
//				StateNode* dstState = m_pActiveTransition->GetTransitionData().DestinationState;
//
//				// Go through all state transitions
//				for (TransitionNode* pTransition : dstState->GetTransitions())
//				{
//					pTransition->ProcessBranch(deltaTime);
//
//					//m_pActiveTransition = pTransition;
//					if (pTransition->GetTransitionData().DestinationState == pState && pTransition->ShouldTransit())
//					{
//						if(!transition.IsReverse()) transition.ReverseTransition();
//						
//						break;
//					}
//				}
//
//
//				StateNode* srcState = m_pActiveTransition->GetTransitionData().SourceState;
//
//				// Go through all state transitions
//				for (TransitionNode* pTransition : srcState->GetTransitions())
//				{
//					// Process transition 
//					pTransition->ProcessBranch(deltaTime);
//					// If we has to transit
//					if (pTransition->GetTransitionData().SourceState == pState && pTransition->ShouldTransit())
//					{
//						if (transition.IsReverse()) transition.ReverseTransition();
//						break;
//					}
//				}
//			}
//			// When transition active, set blended pose to state machine 
//			if (Pose* blendPose = Transit(m_pActiveTransition, deltaTime, nullptr))
//			{
//				GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(1, blendPose);
//			}
//			else
//			{
//				// When transition has been done, set current state and pose to state machine 
//				SetCurrentState(!transition.IsReverse() ? m_pActiveTransition->GetTransitionData().DestinationState : m_pActiveTransition->GetTransitionData().SourceState);
//				m_pActiveTransition = nullptr;
//				transition.SetTransitionReverse(false); 
//			}
//		}
//		else // If there's no active transition 
//		{
//			// Process current state branch
//			pState->ProcessBranch(deltaTime);
//
//			// Go through all state transitions
//			for (TransitionNode* pTransition : pState->GetTransitions())
//			{
//				// Process transition 
//				pTransition->ProcessBranch(deltaTime);
//
//				// If we has to transit
//				if (pTransition->ShouldTransit())
//				{
//					m_pActiveTransition = pTransition;
//					m_pActiveTransition->GetRootNode()->As<OutputTransitionNode>().ResetTransitionAccumulator();
//					return;
//				}
//			}
//			
//			const animation::Pose* statePose = pState->GetOutputPose();
//
//			if (m_IsBlendWithEntryPoint && pState!= GetRootNode())
//			{
//				auto& context = GetGraphContext()->As<AnimationGraphContext>();
//				const animation::Pose* entryPointPose = GetRootNode()->As<EntryStateNode>().GetOutputPose();
//				
//				if (statePose && entryPointPose)
//				{
//					// Set current pose to state machine output
//					GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(1, 
//						context.Controller->Blend(context.Skeleton, statePose, entryPointPose, m_EntryPointClampMax, animation::BoneMask{ nullptr }));
//				}
//				else
//				{
//					GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(1, statePose);
//				}
//			}
//			else
//			{
//				GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(1, statePose);
//			}
//		}
//	}
//}

//namespace untils
//{
//	float ScaleToRange(float value, float inputMin, float inputMax, float outputMin, float outputMax)
//	{
//		float normalizedValue = (value - inputMin) / (inputMax - inputMin);
//		return glm::mix(outputMin, outputMax, value);
//	}
//}
//
//shade::animation::Pose* shade::animation::state_machine::StateMachineNode::Transit(TransitionNode* pTransition, const FrameTimer& deltaTime, Pose* pPTPose)
//{	
//	OutputTransitionNode& transition		= pTransition->GetRootNode()->As<OutputTransitionNode>();
//	TransitionNode::Data& transitionData	= pTransition->GetTransitionData();
//
//	pTransition->SetInterrupted(transition.GetEndpoint<graphs::Connection::Input>(4)->As<NodeValueType::Bool>());
//	// If accumulator more than duration that means transition has been done, or duration is 0 so transition should be immediately
//	if ((transition.IsReverse() && transition.GetTransitionAccumulator() <= 0.f) || (!transition.IsReverse() && transition.GetTransitionAccumulator() > transition.GetTransitionDuration()) || !transition.GetTransitionDuration())
//	{
//		// Reset transition data
//		transitionData.SourceState->SetTransitionSyncData(TransitionSyncData{}); transitionData.DestinationState->SetTransitionSyncData(TransitionSyncData{});
//		// Reset accamulator
//		transition.ResetTransitionAccumulator();
//
//		//GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Float>(1, 1.0);
//	}
//	else // We need make a transition
//	{
//		auto& controller = GetGraphContext()->As<AnimationGraphContext>().Controller;
//		auto& skeleton = GetGraphContext()->As<AnimationGraphContext>().Skeleton;
//
//		const animation::Pose* sPose = transitionData.SourceState->GetOutputPose();
//		const animation::Pose* dPose = transitionData.DestinationState->GetOutputPose();
//
//		// TODO: При переходе из транзишена в транзишен нужно добавить стейту Указатель на тот стейт из которого оно пришло, если возвращяемся назад в предыдухий стейт то 	GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Float>(1, blendFactor); == 1,0 - blend factor
//
//		float blendFactor = math::CalculateBezierFactor(transition.GetTransitionAccumulator(), 0.f, transition.GetTransitionDuration(), 0.f, 1.f, transition.GetCurveControllPoints());
//
//		if (m_IsEntryPointBlendClamp)
//		{
//			if(!transition.IsReverse() && transitionData.DestinationState == GetRootNode())
//				blendFactor = m_EntryPointClampMax - untils::ScaleToRange(blendFactor, 0.f, 1.f, 0.f, m_EntryPointClampMax);
//			else
//				blendFactor = untils::ScaleToRange(blendFactor, 0.f, 1.f, 0.f, m_EntryPointClampMax);
//		}
//	
//		std::cout << blendFactor << std::endl;
//		//blendFactor = m_IsEntryPointBlendClamp ? untils::ScaleToRange((!transition.IsReverse() && transitionData.DestinationState == GetRootNode()) ? blendFactor : blendFactor, 0.f, 1.f, 0.f, m_EntryPointClampMax) : blendFactor;
//
//		GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Float>(0, blendFactor);
//
//		if (sPose && dPose) // Почему то dPose nullptr в transitionData.DestinationState->GetOutputPose(); нужно проплеить все стейты при десериализации ? после востановления конекшенов ?
//		{ 
//			TransitionStatus status = transition.GetTransitionAccumulator() ? TransitionStatus::InProcess : TransitionStatus::Start;
//			SyncStyle style =  transition.GetSynStyle();
//			
//			
//			//float blendFactor = glm::clamp(transition.GetTransitionAccumulator(), transition.GetTransitionDuration(), 0.f, 1.f);
//		
//			auto [sMultiplier, dMultiplier] = controller->GetTimeMultiplier(sPose->GetDuration(), dPose->GetDuration(), blendFactor);
//
//			const bool isReset = transition.GetEndpoint<graphs::Connection::Input>(3)->As<NodeValueType::Bool>();
//
//			switch (style)
//			{
//				case SyncStyle::Async:
//				case SyncStyle::SourceFrozen:
//				{
//					sMultiplier = 1.f; dMultiplier = 1.f;
//					break;
//				}
//				case SyncStyle::SourceToDestinationTimeSync:
//				{
//					dMultiplier = 1.f;
//					break;
//				}
//				case SyncStyle::DestinationToSourceTimeSync:
//				{
//					sMultiplier = 1.f;
//					break;
//				}
//				case SyncStyle::DestinationAndSourceTimeSync:
//				{
//					//
//					break;
//				}
//				case SyncStyle::KeyFrameSync:
//				{
//					break;
//				}
//			}
//
//			transitionData.SourceState->SetTransitionSyncData({
//					.Preferences = {.Style = style,.ResetFromStart = false, .Offset = 0.f },
//					.BlendFactor = blendFactor,
//					.CurrentTransitionTime = transition.GetTransitionAccumulator(),
//					.TimeMultiplier = sMultiplier,
//					.PStateAnimationDuration = dPose->GetDuration(),
//					.PStateAnimationCurrentPlayTime = dPose->GetCurrentPlayTime(),
//					.Status = status
//				});
//
//			transitionData.DestinationState->SetTransitionSyncData({
//				.Preferences = {
//					.Style = SyncStyle::Async,
//					.ResetFromStart = transition.GetEndpoint<graphs::Connection::Input>(3)->As<NodeValueType::Bool>(), 
//					.Offset = transition.GetEndpoint<graphs::Connection::Input>(2)->As<NodeValueType::Float>() 
//				},
//				.BlendFactor = blendFactor,
//				.CurrentTransitionTime = transition.GetTransitionAccumulator(),
//				.TimeMultiplier = dMultiplier,
//				.PStateAnimationDuration = sPose->GetDuration(),
//				.PStateAnimationCurrentPlayTime = sPose->GetCurrentPlayTime(),
//				.Status = status
//				});
//			
//			// Сначала евалуейт а потом уже бленд ! вне if (sPose && dPose) 
//			transitionData.SourceState->Evaluate(deltaTime); transitionData.DestinationState->Evaluate(deltaTime);
//
//			transition.ProcessTransitionAccumulator(deltaTime);
//
//			return (transitionData.DestinationState == GetRootNode()) ? controller->Blend(skeleton, dPose, sPose, blendFactor, animation::BoneMask{ nullptr }) : controller->Blend(skeleton, sPose, dPose, blendFactor, animation::BoneMask{ nullptr });
//			
//		}
//		else if(sPose)
//		{
//			transitionData.SourceState->Evaluate(deltaTime);
//			transition.ProcessTransitionAccumulator(deltaTime);
//			return const_cast<shade::animation::Pose*>(sPose);
//		}
//		else if (dPose)
//		{
//			transitionData.DestinationState->Evaluate(deltaTime);
//			transition.ProcessTransitionAccumulator(deltaTime);
//			return const_cast<shade::animation::Pose*>(dPose);
//		}
//	}
//
//	return nullptr;
//}



void shade::animation::state_machine::StateMachineNode::Evaluate(const FrameTimer& deltaTime)
{
	if (StateNode* pState = m_pCurrentState)
	{
		if (m_pActiveTransition)
		{
			OutputTransitionNode& transition = m_pActiveTransition->GetRootNode()->As<OutputTransitionNode>();

			if (m_pActiveTransition->CanBeInterrupted())
			{
				StateNode* dstState = m_pActiveTransition->GetTransitionData().DestinationState;

				// Process transitions from destination state
				for (TransitionNode* pTransition : dstState->GetTransitions())
				{
					pTransition->ProcessBranch(deltaTime);

					if (pTransition->GetTransitionData().DestinationState == pState && pTransition->ShouldTransit())
					{
						if (!transition.IsReverse())
							transition.ReverseTransition();
						break;
					}
				}

				StateNode* srcState = m_pActiveTransition->GetTransitionData().SourceState;

				// Process transitions from source state
				for (TransitionNode* pTransition : srcState->GetTransitions())
				{
					pTransition->ProcessBranch(deltaTime);

					if (pTransition->GetTransitionData().SourceState == pState && pTransition->ShouldTransit())
					{
						if (transition.IsReverse())
							transition.ReverseTransition();
						break;
					}
				}
			}

			// Handle active transition
			if (Pose* blendPose = Transit(m_pActiveTransition, deltaTime, nullptr))
			{
				GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(1, blendPose);
			}
			else
			{
				// Transition completed, set the current state and pose
				SetCurrentState(!transition.IsReverse() ?
					m_pActiveTransition->GetTransitionData().DestinationState :
					m_pActiveTransition->GetTransitionData().SourceState);
				m_pActiveTransition = nullptr;
				transition.SetTransitionReverse(false);
			}
		}
		else // No active transition
		{
			// Process current state branch
			pState->ProcessBranch(deltaTime);

			// Process all state transitions
			for (TransitionNode* pTransition : pState->GetTransitions())
			{
				pTransition->ProcessBranch(deltaTime);

				if (pTransition->ShouldTransit())
				{
					m_pActiveTransition = pTransition;
					m_pActiveTransition->GetRootNode()->As<OutputTransitionNode>().ResetTransitionAccumulator();
					return;
				}
			}

			const animation::Pose* statePose = pState->GetOutputPose();
			HandlePoseBlending(pState, statePose);
		}
	}
}

void shade::animation::state_machine::StateMachineNode::HandlePoseBlending(StateNode* pState, const animation::Pose* statePose)
{
	if (m_IsBlendWithEntryPoint && pState != GetRootNode())
	{
		auto& context = GetGraphContext()->As<AnimationGraphContext>();
		const animation::Pose* entryPointPose = GetRootNode()->As<EntryStateNode>().GetOutputPose();

		if (statePose && entryPointPose)
		{
			GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(1,
				context.Controller->Blend(context.Skeleton, statePose, entryPointPose, m_EntryPointClampMax, animation::BoneMask{ nullptr }));
		}
		else
		{
			GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(1, statePose);
		}
	}
	else
	{
		GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(1, statePose);
	}
}

namespace untils
{
	float ScaleToRange(float value, float inputMin, float inputMax, float outputMin, float outputMax)
	{
		float normalizedValue = (value - inputMin) / (inputMax - inputMin);
		return glm::mix(outputMin, outputMax, normalizedValue); // исправлен параметр, используем normalizedValue
	}
}

shade::animation::Pose* shade::animation::state_machine::StateMachineNode::Transit(TransitionNode* pTransition, const FrameTimer& deltaTime, Pose* pPTPose)
{
	OutputTransitionNode& transition = pTransition->GetRootNode()->As<OutputTransitionNode>();
	TransitionNode::Data& transitionData = pTransition->GetTransitionData();

	// Set interrupted flag
	pTransition->SetInterrupted(transition.GetEndpoint<graphs::Connection::Input>(4)->As<NodeValueType::Bool>());

	// Check if transition is completed or immediate
	if (IsTransitionCompletedOrImmediate(transition))
	{
		ResetTransitionData(transitionData);
		transition.ResetTransitionAccumulator();
	}
	else
	{
		return HandleTransitionInProcess(transition, transitionData, deltaTime);
	}

	return nullptr;
}

// Helper function to determine if transition is completed or should be immediate
bool shade::animation::state_machine::StateMachineNode::IsTransitionCompletedOrImmediate(const OutputTransitionNode& transition)
{
	return (transition.IsReverse() && transition.GetTransitionAccumulator() <= 0.f) ||
		(!transition.IsReverse() && transition.GetTransitionAccumulator() > transition.GetTransitionDuration()) ||
		!transition.GetTransitionDuration();
}

// Helper function to reset transition data
void shade::animation::state_machine::StateMachineNode::ResetTransitionData(TransitionNode::Data& transitionData)
{
	transitionData.SourceState->SetTransitionSyncData(TransitionSyncData{});
	transitionData.DestinationState->SetTransitionSyncData(TransitionSyncData{});
}

// Handle transition in process
shade::animation::Pose* shade::animation::state_machine::StateMachineNode::HandleTransitionInProcess(OutputTransitionNode& transition, TransitionNode::Data& transitionData, const FrameTimer& deltaTime)
{
	auto& context = GetGraphContext()->As<AnimationGraphContext>();
	auto& controller = context.Controller;
	auto& skeleton = context.Skeleton;

	const animation::Pose* sPose = transitionData.SourceState->GetOutputPose();
	const animation::Pose* dPose = transitionData.DestinationState->GetOutputPose();

	float blendFactor = math::CalculateBezierFactor(transition.GetTransitionAccumulator(), 0.f, transition.GetTransitionDuration(), 0.f, 1.f, transition.GetCurveControllPoints());

	/*if (m_IsEntryPointBlendClamp)
	{
		if (!transition.IsReverse() && transitionData.DestinationState == GetRootNode())
			blendFactor = m_EntryPointClampMax - untils::ScaleToRange(blendFactor, 0.f, 1.f, 0.f, m_EntryPointClampMax);
		else
			blendFactor = untils::ScaleToRange(blendFactor, 0.f, 1.f, 0.f, m_EntryPointClampMax);
	}*/

	if (m_IsEntryPointBlendClamp)
	{
		blendFactor = AdjustBlendFactorForEntryPoint(transition, transitionData.DestinationState, blendFactor);
	}

	GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Float>(0, blendFactor);

	if (sPose && dPose)
	{
		return ProcessTransitionPoses(controller.Raw(), skeleton, transition, transitionData, sPose, dPose, blendFactor, deltaTime);
	}
	else if (sPose)
	{
		transitionData.SourceState->Evaluate(deltaTime);
		transition.ProcessTransitionAccumulator(deltaTime);
		return const_cast<shade::animation::Pose*>(sPose);
	}
	else if (dPose)
	{
		transitionData.DestinationState->Evaluate(deltaTime);
		transition.ProcessTransitionAccumulator(deltaTime);
		return const_cast<shade::animation::Pose*>(dPose);
	}

	return nullptr;
}

// Helper function to adjust blend factor for entry point transitions
float shade::animation::state_machine::StateMachineNode::AdjustBlendFactorForEntryPoint(const OutputTransitionNode& transition, const StateNode* dstState, float blendFactor)
{
	return (!transition.IsReverse() && dstState == GetRootNode() && dstState->GetOutputPose()) ?
		m_EntryPointClampMax - untils::ScaleToRange(blendFactor, 0.f, 1.f, 0.f, m_EntryPointClampMax) :
		untils::ScaleToRange(blendFactor, 0.f, 1.f, 0.f, m_EntryPointClampMax);
}

void shade::animation::state_machine::StateMachineNode::AdjustMultipliersForSyncStyle(SyncStyle style, float& sMultiplier, float& dMultiplier)
{
	switch (style)
	{
	case SyncStyle::Async:
	case SyncStyle::SourceFrozen:
		sMultiplier = 1.f;
		dMultiplier = 1.f;
		break;

	case SyncStyle::SourceToDestinationTimeSync:
		dMultiplier = 1.f; // Дестинация синхронизирована по времени с источником
		break;

	case SyncStyle::DestinationToSourceTimeSync:
		sMultiplier = 1.f; // Источник синхронизирован по времени с дестинацией
		break;

	case SyncStyle::DestinationAndSourceTimeSync:
		// Оба состояния синхронизированы, не изменяем множители
		break;

	case SyncStyle::KeyFrameSync:
		// Обработка синхронизации по ключевым кадрам (если требуется)
		break;
	}
}
void shade::animation::state_machine::StateMachineNode::SetTransitionSyncData(
	OutputTransitionNode& transition,
	TransitionNode::Data& transitionData, TransitionStatus status, SyncStyle style,
	float blendFactor, float sMultiplier, float dMultiplier,
	const animation::Pose* sPose, const animation::Pose* dPose)
{
	// Установка данных синхронизации для исходного состояния
	transitionData.SourceState->SetTransitionSyncData({
		.Preferences = {
			.Style = style,
			.ResetFromStart = false,
			.Offset = 0.f
		},
		.BlendFactor = blendFactor,
		.CurrentTransitionTime = transition.GetTransitionAccumulator(),
		.TimeMultiplier = sMultiplier,
		.PStateAnimationDuration = dPose->GetDuration(),
		.PStateAnimationCurrentPlayTime = dPose->GetCurrentPlayTime(),
		.Status = status
		});

	// Установка данных синхронизации для целевого состояния
	transitionData.DestinationState->SetTransitionSyncData({
		.Preferences = {
			.Style = SyncStyle::Async,
			.ResetFromStart = transition.GetEndpoint<graphs::Connection::Input>(3)->As<NodeValueType::Bool>(),
			.Offset = transition.GetEndpoint<graphs::Connection::Input>(2)->As<NodeValueType::Float>()
		},
		.BlendFactor = blendFactor,
		.CurrentTransitionTime = transition.GetTransitionAccumulator(),
		.TimeMultiplier = dMultiplier,
		.PStateAnimationDuration = sPose->GetDuration(),
		.PStateAnimationCurrentPlayTime = sPose->GetCurrentPlayTime(),
		.Status = status
		});
}
// Process and blend transition poses
shade::animation::Pose* shade::animation::state_machine::StateMachineNode::ProcessTransitionPoses(AnimationController* controller, const Asset<Skeleton>& skeleton, OutputTransitionNode& transition, TransitionNode::Data& transitionData, const animation::Pose* sPose, const animation::Pose* dPose, float blendFactor, const FrameTimer& deltaTime)
{
	TransitionStatus status = transition.GetTransitionAccumulator() ? TransitionStatus::InProcess : TransitionStatus::Start;
	SyncStyle style = transition.GetSynStyle();

	auto [sMultiplier, dMultiplier] = controller->GetTimeMultiplier(sPose->GetDuration(), dPose->GetDuration(), blendFactor);
	AdjustMultipliersForSyncStyle(style, sMultiplier, dMultiplier);

	SetTransitionSyncData(transition, transitionData, status, style, blendFactor, sMultiplier, dMultiplier, sPose, dPose);
	transitionData.SourceState->Evaluate(deltaTime);
	transitionData.DestinationState->Evaluate(deltaTime);
	transition.ProcessTransitionAccumulator(deltaTime);

	return (transitionData.DestinationState == GetRootNode()) ?
		controller->Blend(skeleton, dPose, sPose, blendFactor, animation::BoneMask{ nullptr }) :
		controller->Blend(skeleton, sPose, dPose, blendFactor, animation::BoneMask{ nullptr });
}




std::size_t shade::animation::state_machine::StateNode::Serialize(std::ostream& stream) const
{
	// Serialzie Identifier
	std::size_t size = shade::Serializer::Serialize(stream, GetNodeIdentifier());
	// Serialzie Name
	size += shade::Serializer::Serialize(stream, GetName());

	// Serialzie screen position
	size += shade::Serializer::Serialize(stream, GetScreenPosition());
	// Serialzie count of internal nodes
	size += shade::Serializer::Serialize(stream, std::uint32_t(GetInternalNodes().size() - GetTransitions().size()));

	//------------------------------------------------------------------------
	// Body section
	//------------------------------------------------------------------------
	size += SerializeBody(stream);
	//------------------------------------------------------------------------
	// !Body section
	//------------------------------------------------------------------------

	const auto& inputs = GetEndpoints().at(shade::graphs::Connection::Type::Input);
	const auto& outputs = GetEndpoints().at(shade::graphs::Connection::Type::Output);

	//------------------------------------------------------------------------
	// Endpoints section
	//------------------------------------------------------------------------

	size += shade::Serializer::Serialize(stream, std::uint32_t(inputs.GetSize()));
	for (const auto& [i, d] : inputs)
	{
		size += shade::Serializer::Serialize(stream, d); // d means default value
	}

	size += shade::Serializer::Serialize(stream, std::uint32_t(outputs.GetSize()));
	for (const auto& [i, d] : outputs)
	{
		size += shade::Serializer::Serialize(stream, d); // d means default value
	}

	//------------------------------------------------------------------------
	// !Endpoints section
	//------------------------------------------------------------------------

	// Serialize states
	for (const BaseNode* pNode : GetInternalNodes())
	{
		if (GetTransitions().end() == std::find_if(GetTransitions().begin(), GetTransitions().end(), [pNode](const TransitionNode* transition)
			{
				return transition == pNode;
			}))
		{
			// Serialize type
			size += shade::Serializer::Serialize(stream, pNode->GetNodeType());
			// Serialize node
			size += shade::Serializer::Serialize(stream, *pNode);
		}
	}
	// Serialize root node id 
	size += shade::Serializer::Serialize(stream, (GetRootNode()) ? GetRootNode()->GetNodeIdentifier() : shade::graphs::INVALID_NODE_IDENTIFIER);

	return size;
}

std::size_t shade::animation::state_machine::StateMachineNode::Serialize(std::ostream& stream) const
{
	// Serialzie Identifier
	std::size_t size = shade::Serializer::Serialize(stream, GetNodeIdentifier());
	// Serialzie Name
	size += shade::Serializer::Serialize(stream, GetName());

	// Serialzie screen position
	size += shade::Serializer::Serialize(stream, GetScreenPosition());
	// Serialzie count of internal nodes
	size += shade::Serializer::Serialize(stream, std::uint32_t(GetInternalNodes().size()));

	//------------------------------------------------------------------------
	// Body section
	//------------------------------------------------------------------------
	size += SerializeBody(stream);
	//------------------------------------------------------------------------
	// !Body section
	//------------------------------------------------------------------------

	const auto& inputs = GetEndpoints().at(shade::graphs::Connection::Type::Input);
	const auto& outputs = GetEndpoints().at(shade::graphs::Connection::Type::Output);

	//------------------------------------------------------------------------
	// Endpoints section
	//------------------------------------------------------------------------

	size += shade::Serializer::Serialize(stream, std::uint32_t(inputs.GetSize()));
	for (const auto& [i, d] : inputs)
	{
		size += shade::Serializer::Serialize(stream, d); // d means default value
	}

	size += shade::Serializer::Serialize(stream, std::uint32_t(outputs.GetSize()));
	for (const auto& [i, d] : outputs)
	{
		size += shade::Serializer::Serialize(stream, d); // d means default value
	}

	//------------------------------------------------------------------------
	// !Endpoints section
	//------------------------------------------------------------------------

	// Serialize states
	for (const BaseNode* pNode : GetInternalNodes())
	{
		// SerialzieSerialzie type
		size += shade::Serializer::Serialize(stream, pNode->GetNodeType());
		size += shade::Serializer::Serialize(stream, *pNode);
	}
	// Serialize transitions
	for (const BaseNode* node : GetInternalNodes())
	{
		// Serialzie count of transition nodes
		size += shade::Serializer::Serialize(stream, std::uint32_t(node->As<StateNode>().GetTransitions().size()));

		for (const BaseNode* transition : node->As<StateNode>().GetTransitions())
		{
			// Serialzie type
			size += shade::Serializer::Serialize(stream, transition->GetNodeType());
			// Serialzie transition
			size += shade::Serializer::Serialize(stream, *transition);
		}
	}

	// Serialize root node id 
	size += shade::Serializer::Serialize(stream, (GetRootNode()) ? GetRootNode()->GetNodeIdentifier() : shade::graphs::INVALID_NODE_IDENTIFIER);

	return size;
}

std::size_t shade::animation::state_machine::StateMachineNode::Deserialize(std::istream& stream)
{
	// Deserialize Identifier
	shade::graphs::NodeIdentifier id; std::size_t size = shade::Serializer::Deserialize(stream, id);
	// Deserialize Name
	std::string name; 					size += shade::Serializer::Deserialize(stream, name);

	// Deserialize Screen position
	glm::vec2 screenPosition;			size += shade::Serializer::Deserialize(stream, screenPosition);
	// Deserialize count of internal nodes
	std::uint32_t internalNodesCount;	size += shade::Serializer::Deserialize(stream, internalNodesCount);

	//------------------------------------------------------------------------
	// Body section
	//------------------------------------------------------------------------
	size += DeserializeBody(stream); SetName(name); SetNodeIdentifier(id); GetScreenPosition() = screenPosition;
	//------------------------------------------------------------------------
	// !Body section
	//------------------------------------------------------------------------

	auto& inputs = GetEndpoints().at(shade::graphs::Connection::Type::Input);
	auto& outputs = GetEndpoints().at(shade::graphs::Connection::Type::Output);

	//------------------------------------------------------------------------
	// Endpoints section
	//------------------------------------------------------------------------

	std::uint32_t inputEndpointsCount;	size += shade::Serializer::Deserialize(stream, inputEndpointsCount);

	for (std::uint32_t i = 0; i < inputEndpointsCount; ++i)
	{
		size += shade::Serializer::Deserialize(stream, inputs.At(i)); // d means default value
	}

	std::uint32_t outputEndpointsCount;  size += shade::Serializer::Deserialize(stream, outputEndpointsCount);

	for (std::uint32_t i = 0; i < outputEndpointsCount; ++i)
	{
		size += shade::Serializer::Deserialize(stream, outputs.At(i)); // d means default value
	}

	//------------------------------------------------------------------------
	// !Endpoints section
	//------------------------------------------------------------------------

	for (std::size_t i = 0; i < internalNodesCount; ++i)
	{
		// Deserialize Type
		graphs::NodeType type;	size += shade::Serializer::Deserialize(stream, type);
		BaseNode* pNode = CreateNodeByType(type);
		// Deserialize node
		size += Serializer::Deserialize(stream, *pNode);
	}

	// Deserialize transitions
	for (BaseNode* node : GetInternalNodes())
	{
		// Deserialzie count of transition nodes
		std::uint32_t transitionCount; size += shade::Serializer::Deserialize(stream, transitionCount);

		for (std::size_t j = 0; j < transitionCount; ++j)
		{
			graphs::NodeType type;	size += shade::Serializer::Deserialize(stream, type);
			TransitionNode* pTransition = node->As<StateNode>().CreateTransition(nullptr);
			size += shade::Serializer::Deserialize(stream, pTransition->As<BaseNode>());

			node->As<StateNode>().AddTransition(pTransition);
		}
	}
	// Deserialize root node id  
	shade::graphs::NodeIdentifier rootId;	  size += shade::Serializer::Deserialize(stream, rootId);

	if (rootId != shade::graphs::INVALID_NODE_IDENTIFIER)
		SetRootNode(GetGraphContext()->FindInternalNode(this, rootId));

	return size;
}
