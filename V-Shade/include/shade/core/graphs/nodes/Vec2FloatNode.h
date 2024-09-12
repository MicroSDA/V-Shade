#pragma once
#include <shade/core/graphs/nodes/BaseNode.h>

namespace shade
{
	namespace graphs
	{
		class SHADE_API Vec2FloatNode : public BaseNode
		{
			NODE_STATIC_TYPE_HELPER(Vec2FloatNode)

		public:
			Vec2FloatNode(GraphContext* context, NodeIdentifier identifier, BaseNode* pParentNode)
				: BaseNode(context, identifier, pParentNode, "Vec2")
			{
				m_CanBeOpen = false;
				REGISTER_ENDPOINT<Connection::Output, NodeValueType::Vector2>(glm::vec2{0.f,0.f});
			}; virtual ~Vec2FloatNode() = default;

			virtual void OnConnect(Connection::Type connectionType, NodeValueType type, EndpointIdentifier endpoint) override {};
			virtual void Evaluate(const FrameTimer& deltaTime) override {}
		};
	}
}