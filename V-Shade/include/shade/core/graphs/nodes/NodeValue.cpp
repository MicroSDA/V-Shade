#include "shade_pch.h"
#include "NodeValue.h"
#include <variant>
#include <sstream>
#include <iomanip>
#include <iostream>

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

// Some compilers might require this explicit deduction guide
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void shade::NodeValue::Serialize(std::ostream& stream) const
{
	serialize::Serializer::Serialize(stream, GetType());
	 std::visit(overloaded
		{
			[&stream](const std::monostate&)			{},
			[&stream](const bool& v)					{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const std::int8_t& v)				{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const std::int32_t& v)			{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const std::uint8_t& v)			{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const std::uint32_t& v)			{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const float& v)					{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const double& v)					{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const std::string& v)				{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const animation::Pose* v)			{ serialize::Serializer::Serialize<std::uint32_t>(stream, 0); },
			[&stream](const animation::BoneMask& v)		{ serialize::Serializer::Serialize<std::uint32_t>(stream, 0); },
			[&stream](const glm::vec2& v)				{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const glm::vec3& v)				{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const glm::vec4& v)				{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const glm::mat3& v)				{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const glm::mat4& v)				{ serialize::Serializer::Serialize(stream, v); },
			[&stream](const glm::quat& v)				{ serialize::Serializer::Serialize(stream, v); }
		}, m_Value);
}

void shade::NodeValue::Deserialize(std::istream& stream)
{
	shade::NodeValueType type; serialize::Serializer::Deserialize(stream, type);

	switch (type)
	{
	case shade::NodeValueType::Undefined:
		break;
	case shade::NodeValueType::Bool:
		Initialize<NodeValueType::Bool>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Bool>());
		break;
	case shade::NodeValueType::Int8:
		Initialize<NodeValueType::Int8>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Int8>());
		break;
	case shade::NodeValueType::Int:
		Initialize<NodeValueType::Int>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Int>());
		break;
	case shade::NodeValueType::Uint8:
		Initialize<NodeValueType::Uint8>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Uint8>());
		break;
	case shade::NodeValueType::Uint:
		Initialize<NodeValueType::Uint>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Uint>());
		break;
	case shade::NodeValueType::Float:
		Initialize<NodeValueType::Float>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Float>());
		break;
	case shade::NodeValueType::Double:
		Initialize<NodeValueType::Double>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Double>());
		break;
	case shade::NodeValueType::String:
		Initialize<NodeValueType::String>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::String>());
		break;
	case shade::NodeValueType::Pose:
		Initialize<NodeValueType::Pose>(); std::uint32_t dummy; serialize::Serializer::Deserialize(stream, dummy);
		break;
		case shade::NodeValueType::BoneMask:
		Initialize<NodeValueType::BoneMask>(nullptr); serialize::Serializer::Deserialize(stream, dummy);
		break;
	case shade::NodeValueType::Vector2:
		Initialize<NodeValueType::Vector2>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Vector2>());
		break;
	case shade::NodeValueType::Vector3:
		Initialize<NodeValueType::Vector3>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Vector3>());
		break;
	case shade::NodeValueType::Vector4:
		Initialize<NodeValueType::Vector4>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Vector4>());
		break;
	case shade::NodeValueType::Matrix3:
		Initialize<NodeValueType::Matrix3>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Matrix3>());
		break;
	case shade::NodeValueType::Matrix4:
		Initialize<NodeValueType::Matrix4>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Matrix4>());
		break;
	case shade::NodeValueType::Quaternion:
		Initialize<NodeValueType::Quaternion>(); serialize::Serializer::Deserialize(stream, As<shade::NodeValueType::Quaternion>());
		break;
	}
}
