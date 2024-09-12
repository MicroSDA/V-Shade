#include "shade_pch.h"
#include "BlendTree2D.h"
#include <shade/core/math/Math.h>

shade::animation::BlendTree2D::BlendTree2D(graphs::GraphContext* context, graphs::NodeIdentifier identifier, graphs::BaseNode* pParentNode) :
    BaseNode(context, identifier, pParentNode, "BlendTree2D")
{
    m_CanBeOpen = false;

    REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Vector2>(0.f);
    REGISTER_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(nullptr); // Out pose y
}

void shade::animation::BlendTree2D::AddInputPose()
{
    REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(nullptr);
    REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Vector2>(0.f); // threshold x	
}

void shade::animation::BlendTree2D::RemoveInputPose(const graphs::EndpointIdentifier identifier)
{
    const graphs::EndpointIdentifier posId = identifier;
    const graphs::EndpointIdentifier vecId = identifier + 1;

    // Copy old connections
    std::vector<graphs::Connection> oldConnections = GetConnections();

    for (graphs::Connection& connection : oldConnections)
    {
        if (connection.ConnectedToEndpoint >= posId)
        {
            DisconnectNodes(connection.ConnectedToEndpoint, connection.PConnectedFrom, connection.ConnectedFromEndpoint);
        }
    }

    REMOVE_ENDPOINT<graphs::Connection::Input>(posId); // Remove pose
    REMOVE_ENDPOINT<graphs::Connection::Input>(posId); // Remove vec2

    for (graphs::Connection& connection : oldConnections)
    {
        if (connection.ConnectedToEndpoint > vecId)
        {
            ConnectNodes(connection.ConnectedToEndpoint - 2, connection.PConnectedFrom, connection.ConnectedFromEndpoint);
            
        }
    }
}

void shade::animation::BlendTree2D::Evaluate(const FrameTimer& delatTime)
{
    auto& controller = GetGraphContext()->As<AnimationGraphContext>().Controller;
    const auto& skeleton = GetGraphContext()->As<AnimationGraphContext>().Skeleton;

    StackArray<Pose*, 20>   validPoses;  std::vector<glm::vec2> points;

    const glm::vec2& input = GetEndpoints()[graphs::Connection::Type::Input].At(0)->As<NodeValueType::Vector2>();

    for (std::size_t i = 1; i < GetEndpoints()[graphs::Connection::Type::Input].GetSize(); i += 2)
    {
        if (animation::Pose* pose = GetEndpoints()[graphs::Connection::Type::Input].At(i)->As<NodeValueType::Pose>())
        {
            validPoses.Emplace(pose);
            points.push_back(GetEndpoints()[graphs::Connection::Type::Input].At(i + 1)->As<NodeValueType::Vector2>());
        }
    }
 
    if (Pose* basePose = validPoses.GetSize() ? validPoses[0] : nullptr)
    {
        std::vector<float> weights = math::Calculate2DWeightsPolar(points, input);

        for (std::size_t i = 0; i < validPoses.GetSize() - 1; ++i)
        {
            if (weights[i + 1] <= 0.f)
                continue;

            basePose = controller->Blend(skeleton, basePose, validPoses[i + 1], weights[i + 1], BoneMask{ nullptr });
        }

        GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, basePose);
    }
    else
    {
        GET_ENDPOINT<graphs::Connection::Output, NodeValueType::Pose>(0, nullptr);
    }
}

void shade::animation::BlendTree2D::Deserialize(std::istream& stream)
{
    // Deserialize Identifier
    graphs::NodeIdentifier id;			serialize::Serializer::Deserialize(stream, id);
    // Deserialize Name
    std::string name; 					serialize::Serializer::Deserialize(stream, name);

    // Deserialize Screen position
    glm::vec2 screenPosition;			serialize::Serializer::Deserialize(stream, screenPosition);
    // Deserialize count of internal nodes
    std::uint32_t internalNodesCount;	serialize::Serializer::Deserialize(stream, internalNodesCount);

    //------------------------------------------------------------------------
    // Body section
    //------------------------------------------------------------------------
    DeserializeBody(stream); SetName(name); SetNodeIdentifier(id); GetScreenPosition() = screenPosition;
    //------------------------------------------------------------------------
    // !Body section
    //------------------------------------------------------------------------

    auto& inputs = GetEndpoints().at(shade::graphs::Connection::Type::Input);
    auto& outputs = GetEndpoints().at(shade::graphs::Connection::Type::Output);

    //------------------------------------------------------------------------
    // Endpoints section
    //------------------------------------------------------------------------

    std::uint32_t inputEndpointsCount;	serialize::Serializer::Deserialize(stream, inputEndpointsCount);

    for (std::uint32_t i = 0; i < inputEndpointsCount; ++i)
    {
        if (i > 0)
        {
            if (i % 2 != 0)
            {
                REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Pose>(nullptr);
            }
            else
            {
                REGISTER_ENDPOINT<graphs::Connection::Input, NodeValueType::Vector2>(0.f);
            }

            serialize::Serializer::Deserialize(stream, inputs.At(i)); // d means default value
        }
        else
        {
            serialize::Serializer::Deserialize(stream, inputs.At(i)); // input value
        }
    }

    std::uint32_t outputEndpointsCount;  serialize::Serializer::Deserialize(stream, outputEndpointsCount); // Не правельно вытягивает количстов оутупотов

    for (std::uint32_t i = 0; i < outputEndpointsCount; ++i)
    {
        serialize::Serializer::Deserialize(stream, outputs.At(i)); // d means default value
    }

    //------------------------------------------------------------------------
    // !Endpoints section
    //------------------------------------------------------------------------

    for (std::size_t i = 0; i < internalNodesCount; ++i)
    {
        graphs::NodeType type;	serialize::Serializer::Deserialize(stream, type);
        BaseNode* pNode = CreateNodeByType(type);

        serialize::Serializer::Deserialize(stream, *pNode);
    }

    // Deserialize root node id  
    shade::graphs::NodeIdentifier rootId;	  serialize::Serializer::Deserialize(stream, rootId);

    if (rootId != shade::graphs::INVALID_NODE_IDENTIFIER)
        SetRootNode(GetGraphContext()->FindInternalNode(this, rootId));
}
