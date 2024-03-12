#pragma once
#include <shade/core/animation/graphs/AnimationGraph.h>
#include <shade/core/layer/imgui/ImGuiLayer.h>



//#pragma once
//#include <animation graphs/EditorAnimGraph.h>
//
////namespace editor_state_machine
////{
////	using namespace shade;
////	using namespace animation;
////	using namespace editor_animation_graph;
////
////	struct StateMachineDeligate : public
////		ImGuiGraphPrototype<PlayerStateMachineComponent,
////		GraphNode::NodeIdentifier,
////		GraphNode::EndpointIdentifier,
////		SharedPointer<GraphNode>>
////	{
////		StateMachineDeligate(PlayerStateMachineComponent machine) : ImGuiGraphPrototype(machine) {}
////		virtual ~StateMachineDeligate() = default;
////
////		virtual bool Connect(const ConnectionPrototype<GraphNode::NodeIdentifier, GraphNode::EndpointIdentifier>& connection) override
////		{
////			return false;
////		};
////		virtual bool Disconnect(const ConnectionPrototype<GraphNode::NodeIdentifier, GraphNode::EndpointIdentifier>& coonection) override
////		{
////			return GetGraph()->RemoveConnection(coonection.InputNodeIdentifier, coonection.InputEndpointIdentifier);
////		}
////		virtual void  PopupMenu() override
////		{
////		
////		};
////		virtual void ProcessSideBar() override {} ;
////	private:
////		std::string  m_Search;
////		bool m_IsSkeletonPopupActive = false;
////	};
////}
//
