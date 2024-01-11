#pragma once
#include <shade/core/animation/graph/AnimationGraph.h>
#include <shade/core/layer/imgui/ImGuiGraph.h>
#include <shade/core/layer/imgui/ImGuiLayer.h>

namespace editor_animation_graph
{
	using namespace shade;
	using namespace animation;


	struct BaseNodeDeligate : public GraphNodePrototype<GraphNode::NodeIDX,
		GraphNode::EndpointIDX,
		SharedPointer<GraphNode>>
	{
		BaseNodeDeligate(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
			GraphNodePrototype<GraphNode::NodeIDX,
			GraphNode::EndpointIDX,
			SharedPointer<GraphNode>>(id, node) {}
		virtual ~BaseNodeDeligate() = default;

		virtual ConnectionsPrototype<GraphNode::NodeIDX, GraphNode::EndpointIDX>  ReceiveConnections() const override
		{
			shade::ConnectionsPrototype<GraphNode::NodeIDX, GraphNode::EndpointIDX> connections;

			for (const auto& conenction : GetNode()->__GET_CONNECTIONS())
				connections.emplace_back(conenction.InputNodeIdx, conenction.InputEndpoint, conenction.OutputNodeIdx, conenction.OutputEndpoint);
			//connections.emplace_back(conenction.Source, conenction.SourceEndpoint, conenction.Target, conenction.TargetEndpoint);

			return connections;
		}

		virtual EndpointsPrototype<GraphNode::NodeIDX> ReceiveEndpoints() const override
		{
			EndpointsPrototype<GraphNode::NodeIDX> endpoints;

			for (GraphNode::EndpointIDX index = 0; index < GetNode()->GetEndpoints()[GraphNode::Connection::Input].GetSize(); ++index)
				endpoints[EndpointPrototype::Input].emplace(index, EndpointPrototype(EndpointPrototype::Input));
			for (GraphNode::EndpointIDX index = 0; index < GetNode()->GetEndpoints()[GraphNode::Connection::Output].GetSize(); ++index)
				endpoints[EndpointPrototype::Output].emplace(index, EndpointPrototype(EndpointPrototype::Output));

			return endpoints;
		}

		virtual void ProcessBodyConent() override { ImGui::Text("Body Content"); }
		virtual void ProcessEndpoint(const GraphNode::EndpointIDX& endpoint, EndpointPrototype::EndpointType type) override
		{
			ImGui::Text("Endpoint");
		}
	};

	struct OutputPoseNodeDeligate : public BaseNodeDeligate
	{
		OutputPoseNodeDeligate(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
			BaseNodeDeligate(id, node)
		{
			Style.Title = "OUTPUT POSE";
			Style.HeaderColor = ImVec4{ 0.7, 0.7, 0.7, 1.0 };
		}
		virtual ~OutputPoseNodeDeligate() = default;
		virtual void ProcessEndpoint(const GraphNode::EndpointIDX& endpoint, EndpointPrototype::EndpointType type) override
		{
			ImGui::Text("Input Pose");
		}
	};

	struct PoseNodeDeligate : public BaseNodeDeligate
	{
		PoseNodeDeligate(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
			BaseNodeDeligate(id, node)
		{
			Style.Title = "POSE";
		}
		virtual ~PoseNodeDeligate() = default;

		virtual void ProcessBodyConent() override
		{
			PoseNode& node = GetNode()->As<animation::PoseNode>();
			float& start = node.GetAnimationData().Start;
			float& end = node.GetAnimationData().End;
			float& duration = node.GetAnimationData().Duration;

			ImGui::BeginDisabled();
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::SliderFloat("##TimeLine", &node.GetAnimationData().CurrentPlayTime, start, end);
			ImGui::PopItemWidth();
			glm::vec2 startEnd = { start , end };
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat2("##StartEnd", glm::value_ptr(startEnd), 0.01f, 0.0f, duration);
			ImGui::PopItemWidth();
			ImGui::EndDisabled();
		}
		virtual void ProcessEndpoint(const GraphNode::EndpointIDX& endpoint, EndpointPrototype::EndpointType type) override
		{
			switch (type)
			{
			case EndpointPrototype::Output:
			{
				ImGui::Text("Pose"); // break;
			}
			}
		}
		virtual void ProcessSideBar() override;
	private:
		std::string m_Search;
		bool m_IsAnimationPopupActive = false;
	};
	struct BlendNodeDeligate : public BaseNodeDeligate
	{
		BlendNodeDeligate(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
			BaseNodeDeligate(id, node)
		{
			Style.Title = "BLEND";
		}
		virtual ~BlendNodeDeligate() = default;

		virtual void ProcessBodyConent() override
		{
			//ImGui::Text("BlendNode");
		}
		virtual void ProcessEndpoint(const GraphNode::EndpointIDX& endpoint, EndpointPrototype::EndpointType type) override
		{
			auto& node = GetNode()->As<BlendNode2D>();

			switch (type)
			{
			case EndpointPrototype::Input:
			{
				if (endpoint == 0) // Blend Weight
				{
					ImGui::Text("Blend Weight");
					ImGui::BeginDisabled();
					ImGui::DragFloat("##", &node.GetEndpoint<GraphNode::Connection::Input>(endpoint)->As<NodeValueType::Float>(), 0.001);
					ImGui::EndDisabled();
				}
				// Input Poses
				if (endpoint > 0) ImGui::Text("Input Pose");

				break;
			}
			case EndpointPrototype::Output:
			{
				ImGui::Text("Pose"); break;
			}
			}
		}
	};
	struct ValueNodeDeligate : public BaseNodeDeligate
	{
		ValueNodeDeligate(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
			BaseNodeDeligate(id, node)
		{
			Style.Title = "FLOAT";
			Style.HeaderColor = ImVec4{ 0.3, 0.3, 0.9, 1.0 };
			Style.Size = ImVec2{ 100, 100 };
		}
		virtual ~ValueNodeDeligate() = default;

		virtual void ProcessBodyConent() override
		{
			auto& node = GetNode()->As<animation::BlendNode2D>();
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat("##value", &node.GetEndpoint<GraphNode::Connection::Output>(0)->As<NodeValueType::Float>(), 0.001, 0.0, 1.0);
			ImGui::PopItemWidth();
		}
		virtual void ProcessEndpoint(const GraphNode::EndpointIDX& endpoint, EndpointPrototype::EndpointType type) override
		{

		}
	};

	struct GraphDeligate : public
		ImGuiGraphPrototype<Asset<AnimationGraph>,
		GraphNode::NodeIDX,
		GraphNode::EndpointIDX,
		SharedPointer<GraphNode>>
	{
		GraphDeligate(Asset<AnimationGraph> graph) : ImGuiGraphPrototype(graph) {}
		virtual ~GraphDeligate() = default;

		virtual bool Connect(const ConnectionPrototype<GraphNode::NodeIDX, GraphNode::EndpointIDX>& connection) override
		{
			SHADE_CORE_TRACE("OutputNodeIdentifier = {0},OutputEndpointIdentifier = {1},InputNodeIdentifier = {2},InputEndpointIdentifier = {3}",
				connection.OutputNodeIdentifier, connection.OutputEndpointIdentifier, connection.InputNodeIdentifier, connection.InputEndpointIdentifier)

				return GetGraph()->AddConnection(connection.InputNodeIdentifier, connection.InputEndpointIdentifier, connection.OutputNodeIdentifier, connection.OutputEndpointIdentifier);
		};
		virtual bool Disconnect(const ConnectionPrototype<GraphNode::NodeIDX, GraphNode::EndpointIDX>& coonection) override
		{
			return GetGraph()->RemoveConnection(coonection.InputNodeIdentifier, coonection.InputEndpointIdentifier);
		}
		virtual void  PopupMenu() override
		{
			if (ImGui::MenuItem("Blend"))
			{
				auto node = GetGraph()->CreateNode<animation::BlendNode2D>();
				this->EmplaceNode<BlendNodeDeligate>(node->GetNodeIndex(), node);
			}
			if (ImGui::MenuItem("Pose"))
			{
				auto node = GetGraph()->CreateNode<animation::PoseNode>();
				this->EmplaceNode<PoseNodeDeligate>(node->GetNodeIndex(), node);
			}
			if (ImGui::MenuItem("Flaot"))
			{
				auto node = GetGraph()->CreateNode<animation::ValueNode>();
				this->EmplaceNode<ValueNodeDeligate>(node->GetNodeIndex(), node);
			}
		};
		virtual void ProcessSideBar(GraphNodePrototype<GraphNode::NodeIDX, GraphNode::EndpointIDX, SharedPointer<GraphNode>>& selectedNode) override
		{
			selectedNode.ProcessSideBar();
		}
	};

}
