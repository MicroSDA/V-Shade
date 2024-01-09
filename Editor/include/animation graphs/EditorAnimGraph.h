#pragma once
#include <shade/core/animation/graph/AnimationGraph.h>
#include <shade/core/layer/imgui/ImGuiGraph.h>
#include <shade/core/layer/imgui/ImGuiLayer.h>

namespace editor_anim_grap_nodes
{
	using namespace shade;
	using namespace animation;

	struct BaseNode : public GraphNodeDeligate<GraphNode::NodeIDX,
		GraphNode::EndpointIDX,
		SharedPointer<GraphNode>>
	{
		BaseNode(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
			GraphNodeDeligate<GraphNode::NodeIDX,
			GraphNode::EndpointIDX,
			SharedPointer<GraphNode>>(id, node) {}
		virtual ~BaseNode() = default;

		virtual ConnectionDeligate<GraphNode::NodeIDX, GraphNode::EndpointIDX>  MergeConnections() const override
		{
			shade::ConnectionDeligate<GraphNode::NodeIDX, GraphNode::EndpointIDX> connections;

			for (const auto& conenction : GetNode()->__GET_CONNECTIONS())
				connections.emplace_back(conenction.Source, conenction.SourceEndpoint, conenction.Target, conenction.TargetEndpoint);

			return connections;
		}

		virtual EndpointsDeligate<GraphNode::NodeIDX> MergeEndpoints() const override
		{
			EndpointsDeligate<GraphNode::NodeIDX> endpoints;

			for (GraphNode::EndpointIDX index = 0; index < GetNode()->GetEndpoints()[GraphNode::Connection::Input].GetSize(); ++index)
				endpoints[Endpoint::Input][index].Name = "Should be type name";
			for (GraphNode::EndpointIDX index = 0; index < GetNode()->GetEndpoints()[GraphNode::Connection::Output].GetSize(); ++index)
				endpoints[Endpoint::Output][index].Name = "Should be type name";

			return endpoints;
		}

		virtual void ProcessBodyConent() override { ImGui::Text("Body Content"); }
		virtual void ProcessEndpoint(const GraphNode::EndpointIDX& endpoint, Endpoint::Type type) override { ImGui::Text("Endpoint"); }
	};


	struct Graph : public
	ImGuiGraphDeligate<SharedPointer<AnimationGraph>,
		GraphNode::NodeIDX,
		GraphNode::EndpointIDX,
		SharedPointer<GraphNode>>
	{
		Graph(SharedPointer<AnimationGraph> graph) : ImGuiGraphDeligate(graph) {}
		virtual ~Graph() = default;

		virtual bool Connect(const Connection<GraphNode::NodeIDX, GraphNode::EndpointIDX>& coonection)
		{

		};
	};
	
	struct OutputPoseNode : public BaseNode
	{
		OutputPoseNode(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
			BaseNode(id, node)
		{
			Title = "OUTPUT POSE";
			Style.HeaderColor = ImVec4{ 0.7, 0.7, 0.7, 1.0 };
		}
		virtual ~OutputPoseNode() = default;
		virtual void ProcessBodyConent() override
		{
		}
		virtual void ProcessEndpoint(const GraphNode::EndpointIDX& endpoint, Endpoint::Type type) override
		{
			ImGui::Text("Input Pose");
		}
	};
	struct PoseNode : public BaseNode
	{
		PoseNode(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
			BaseNode(id, node)
		{
			Title = "POSE";
		}
		virtual ~PoseNode() = default;

		virtual void ProcessBodyConent() override
		{
			auto&	node = GetNode()->As<animation::PoseNode>();
			float&	start = node.GetAnimationData().Start;
			float&	end = node.GetAnimationData().End;
			float&	duration = node.GetAnimationData().Duration;


			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

			ImGui::BeginDisabled();
			ImGui::SliderFloat("##TimeLine", &node.GetAnimationData().CurrentPlayTime, start, end);
			ImGui::EndDisabled();

			ImGui::PopItemWidth();

			glm::vec2 startEnd = { start , end };

			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

			ImGui::DragFloat2("##StartEnd", glm::value_ptr(startEnd), 0.01f, 0.0f, duration);

			ImGui::PopItemWidth();

			start = startEnd.x;
			end = startEnd.y;

			std::vector<std::string> items;

			for (const auto& assetData : shade::AssetManager::GetAssetDataList(shade::AssetMeta::Category::Secondary))
			{
				if (assetData.second->GetType() == shade::AssetMeta::Type::Animation)
				{
					items.emplace_back(assetData.first);
				}
			}

			std::string currentItem = "";

			if (node.GetAnimationData().Animation)
			{
				currentItem = node.GetAnimationData().Animation->GetAssetData()->GetId();
			}

			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::BeginCombo("##", currentItem.c_str(), 0)) // The second parameter is the label previewed before opening the combo.
			{
				for (auto& element : items)
				{
					bool isSelected = (currentItem == element);
					if (ImGui::Selectable(element.c_str(), isSelected, 0))
					{
						currentItem = element;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();

				shade::AssetManager::GetAsset<shade::Animation, shade::BaseAsset::InstantiationBehaviour::Synchronous>(currentItem, shade::AssetMeta::Category::Secondary, shade::BaseAsset::LifeTime::KeepAlive, [&](auto& animation) mutable
					{
						node.ResetAnimationData(animation);
					});

			}

			ImGui::PopItemWidth();
		}
		virtual void ProcessEndpoint(const GraphNode::EndpointIDX& endpoint, Endpoint::Type type) override
		{
			switch (type)
			{
			case Endpoint::Output:
			{
				ImGui::Text("Pose"); // break;
			}
			}
		}
	};
	struct BlendNode : public BaseNode
	{
		BlendNode(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
			BaseNode(id, node)
		{
			Title = "BLEND";
		}
		virtual ~BlendNode() = default;

		virtual void ProcessBodyConent() override
		{
			//ImGui::Text("BlendNode");
		}
		virtual void ProcessEndpoint(const GraphNode::EndpointIDX& endpoint, Endpoint::Type type) override
		{
			auto& node = GetNode()->As<BlendNode2D>();

			switch (type)
			{
			case Endpoint::Input:
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
			case Endpoint::Output:
			{
				ImGui::Text("Pose"); break;
			}
			}
		}
	};
	struct ValueNode : public BaseNode
	{
		ValueNode(GraphNode::NodeIDX id, SharedPointer<GraphNode> node) :
			BaseNode(id, node)
		{
			Title = "FLOAT";
			Style.HeaderColor = ImVec4{ 0.3, 0.3, 0.9, 1.0 };
			Size = ImVec2{ 100, 100 };
		}
		virtual ~ValueNode() = default;

		virtual void ProcessBodyConent() override
		{
			auto& node = GetNode()->As<animation::BlendNode2D>();
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat("##value", &node.GetEndpoint<GraphNode::Connection::Output>(0)->As<NodeValueType::Float>(), 0.001, 0.0, 1.0);
			ImGui::PopItemWidth();
		}
		virtual void ProcessEndpoint(const GraphNode::EndpointIDX& endpoint, Endpoint::Type type) override
		{

		}
	};
}
