#pragma once
#include <shade/core/graphs/GraphContext.h>
#include <shade/core/animation/Skeleton.h>
#include <shade/core/animation/AnimationController.h>
#include <ankerl/unordered_dense.h>

namespace shade
{
	namespace animation
	{
		struct SynchronizingGroup
		{
		public:
			graphs::BaseNode const* pLeader = nullptr;
			// Add Sync way !, style or whatever !!
			// We have two options, Set global sync pattern to groups or set individual for each pose node
			// Hard to know at this moment what will be better!
			ankerl::unordered_dense::set<const graphs::BaseNode*> Nodes;

			SHADE_INLINE void AddNode(const graphs::BaseNode* pNode)
			{
				Nodes.emplace(pNode);
				pLeader = (!pLeader) ? pNode : pLeader;
			}
			SHADE_INLINE void RemoveNode(const graphs::BaseNode* pNode)
			{
				auto node = Nodes.find(pNode);
				if (node != Nodes.end())
				{
					Nodes.erase(node);
					// Set last as leader.
					pLeader = pLeader == pNode ? *Nodes.end() : pLeader;
				}
			}
		};

		struct SynchronizingGroups
		{
		public:
			SHADE_INLINE bool AddGroup(const char* name)
			{
				if (Groups.find(name) == Groups.end())
				{
					Groups.emplace(name, SynchronizingGroup{});
					return true;
				}
				return false;
			}
			SHADE_INLINE bool RemoveGroup(const char* name)
			{
				if (Groups.find(name) != Groups.end())
				{
					Groups.erase(name);
					return true;
				}
				return false;
			}
			SHADE_INLINE const SynchronizingGroup* GetGroup(const char* name) const
			{
				if (Groups.find(name) != Groups.end())
				{
					return &Groups.at(name);
				}
				return nullptr;
			}
			SHADE_INLINE SynchronizingGroup* GetGroup(const char* name)
			{
				return const_cast<SynchronizingGroup*>(
					static_cast<const SynchronizingGroup*>(static_cast<const SynchronizingGroups*>(this)->GetGroup(name))
					);
			}
			SHADE_INLINE ankerl::unordered_dense::map<std::string, SynchronizingGroup>& GetGroups()
			{
				return Groups;
			}
			SHADE_INLINE void SetGroupLeader(const char* name, const graphs::BaseNode* pNode)
			{
				auto group = Groups.find(name);
				if (group != Groups.end())
				{
					if (group->second.Nodes.find(pNode) != group->second.Nodes.end())
					{
						group->second.pLeader = pNode;
					}
				}
			}
		private:
			ankerl::unordered_dense::map<std::string, SynchronizingGroup> Groups;
		};
		
		/// @brief Structure representing the context of a graph
		struct AnimationGraphContext : public graphs::GraphContext
		{
		public:
			///@brief Current entity
			Asset<Skeleton> Skeleton;
			///@brief AnimationController
			mutable SharedPointer<AnimationController> Controller;
			///@brief For animation synchronizing
			mutable SynchronizingGroups SyncGroups;
		};
	}
}