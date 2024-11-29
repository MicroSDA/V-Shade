#include "shade_pch.h"
#include "PoseNode.h"
#include <shade/core/animation/AnimationController.h>
#include <shade/core/animation/graphs/nodes/StateMachineNode.h>

namespace shade
{
	namespace animation
	{
		namespace utils
		{
			SHADE_INLINE static const SynchronizingGroup* GetSyncGroup(const char* groupName, const shade::animation::SynchronizingGroups& pools, const graphs::BaseNode* node)
			{
				if (auto group = pools.GetGroup(groupName))
				{
					if (group->Nodes.find(node) != group->Nodes.end())
					{
						return group;
					}
				}
				return nullptr;
			}
		}
	}
}

// Rename to Animation track ! ! !
void shade::animation::PoseNode::Evaluate(const FrameTimer& deltaTime)
{
	auto& controller	= GetGraphContext()->As<AnimationGraphContext>().Controller;
	auto& skeleton		= GetGraphContext()->As<AnimationGraphContext>().Skeleton;
	auto& syncPools		= GetGraphContext()->As<AnimationGraphContext>().SyncGroups;

	if (m_AnimationData.Animation)
	{
		if (auto syncGroup = utils::GetSyncGroup(m_AnimationData.SyncGroupName.c_str(), syncPools, this))
		{
			//Check if we are not leader and leader exists !
			if (syncGroup->pLeader && syncGroup->pLeader != this)
			{
				const auto& leaderAnimationData			= syncGroup->pLeader->As<PoseNode>().GetAnimationData();
				const auto& followerAnimationData		= m_AnimationData;

				/*float leaderCurrentTime		= leaderAnimationData.CurrentTime;
				const auto& leaderMarkers	= leaderAnimationData.SyncMarkers;*/
				
				const auto [TimeMultiplier1, TimeMultiplier2] = controller->GetTimeMultiplier(syncGroup->pLeader->As<PoseNode>().GetAnimationData().Duration, m_AnimationData.Duration, 1.0);





				GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, m_AnimationData, deltaTime, TimeMultiplier1));
				return;
				// So here should be all magic with sync 
				// 1. We dont need transition sync data i belive, maby just current transition time to make a proper synk
				// 2. Also we need to check from transition i guess what our transition is using
				// 3. We need get from transition synkc some transition synk rules, reset dist animation, and use group or not 
			}
		}
	
		if (graphs::GetNodeTypeId<state_machine::StateNode>() == GetParrentGraph()->GetNodeType())
		{
			const state_machine::TransitionSyncData syncData = GetParrentGraph()->As<state_machine::StateNode>().GetTransitionSyncData();

			switch (syncData.Status)
			{
				case state_machine::TransitionStatus::Start:
				{
					if (syncData.Preferences.ResetFromStart)
						m_AnimationData.CurrentPlayTime = m_AnimationData.Start + syncData.Preferences.Offset;
				}
				case state_machine::TransitionStatus::InProcess:
				{
					switch (syncData.Preferences.Style)
					{
					case state_machine::SyncStyle::SourceFrozen:

						m_AnimationData.State = Animation::State::Pause;
						GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, m_AnimationData, deltaTime, syncData.TimeMultiplier));
						break;

					case state_machine::SyncStyle::SourceToDestinationTimeSync:

					case state_machine::SyncStyle::DestinationToSourceTimeSync:

					case state_machine::SyncStyle::DestinationAndSourceTimeSync:

					case state_machine::SyncStyle::KeyFrameSync: break;

					default:
						//m_AnimationData.State = Animation::State::Play;
						GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, m_AnimationData, deltaTime, syncData.TimeMultiplier));
						break;
					}
					break;
				}
				case state_machine::TransitionStatus::End: // When transition end or non transition occurs
				{
					GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, m_AnimationData, deltaTime, syncData.TimeMultiplier));
					break;
				}
			}
		}
		else
		{
			GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, controller->ProcessPose(skeleton, m_AnimationData, deltaTime));
		}
	}
	
}
// TODO: При добавлении анимации нужно хотябы раз ее процеснуть на 0 делта времени что бы поза генерировалась 
void shade::animation::PoseNode::ResetAnimationData(const Asset<Animation>& animation)
{
	bool has = m_AnimationData.HasRootMotion;
	m_AnimationData = AnimationController::AnimationControlData(animation);
	m_AnimationData.HasRootMotion = has;
	/*auto oldState = m_AnimationData.State;

	m_AnimationData.State = Animation::State::Play;
	Evaluate({ 0 });
	m_AnimationData.State = oldState;*/
}

void shade::animation::PoseNode::ResetAnimationData(const AnimationController::AnimationControlData& data)
{
	m_AnimationData = data;
	/*auto oldState = m_AnimationData.State;

	m_AnimationData.State = Animation::State::Play;
	Evaluate({ 0 });
	m_AnimationData.State = oldState;*/
}

const shade::animation::AnimationController::AnimationControlData& shade::animation::PoseNode::GetAnimationData() const
{
	return m_AnimationData;
}

shade::animation::AnimationController::AnimationControlData& shade::animation::PoseNode::GetAnimationData()
{
	return m_AnimationData;
}

void shade::animation::PoseNode::SerializeBody(std::ostream& stream) const
{
	SHADE_CORE_INFO("Serialize '{0}' body section...", GetName());
	serialize::Serializer::Serialize(stream, m_AnimationData);
}

void shade::animation::PoseNode::DeserializeBody(std::istream& stream)
{
	SHADE_CORE_INFO("Deserialize '{0}' body section...", GetName());
	AnimationController::AnimationControlData animationData;
	serialize::Serializer::Deserialize(stream, animationData); ResetAnimationData(animationData);
}
