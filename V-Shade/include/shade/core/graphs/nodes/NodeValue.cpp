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

std::size_t shade::NodeValue::Serialize(std::ostream& stream) const
{
	std::size_t size = Serializer::Serialize(stream, GetType());
	size += std::visit(overloaded
		{
			[&stream](const std::monostate&) -> std::size_t { return 0u; },
			[&stream](const bool& v) -> std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const std::int8_t& v) -> std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const std::int32_t& v) -> std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const std::uint8_t& v) -> std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const std::uint32_t& v) -> std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const float& v) -> std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const double& v) -> std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const std::string& v) -> std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const animation::Pose* v)->std::size_t { return Serializer::Serialize(stream, std::uint32_t(0u)); },
			[&stream](const animation::BoneMask& v)->std::size_t { return Serializer::Serialize(stream, std::uint32_t(0u)); },
			[&stream](const glm::vec2& v)->std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const glm::vec3& v)->std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const glm::vec4& v)->std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const glm::mat3& v)->std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const glm::mat4& v)->std::size_t { return Serializer::Serialize(stream, v); },
			[&stream](const glm::quat& v)->std::size_t { return Serializer::Serialize(stream, v); }
		}, m_Value);

	return size;
}

std::size_t shade::NodeValue::Deserialize(std::istream& stream)
{
	shade::NodeValueType type; shade::Serializer::Deserialize(stream, type);

	switch (type)
	{
	case shade::NodeValueType::Undefined:
		break;
	case shade::NodeValueType::Bool:
		Initialize<NodeValueType::Bool>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Bool>());
		break;
	case shade::NodeValueType::Int8:
		Initialize<NodeValueType::Int8>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Int8>());
		break;
	case shade::NodeValueType::Int:
		Initialize<NodeValueType::Int>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Int>());
		break;
	case shade::NodeValueType::Uint8:
		Initialize<NodeValueType::Uint8>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Uint8>());
		break;
	case shade::NodeValueType::Uint:
		Initialize<NodeValueType::Uint>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Uint>());
		break;
	case shade::NodeValueType::Float:
		Initialize<NodeValueType::Float>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Float>());
		break;
	case shade::NodeValueType::Double:
		Initialize<NodeValueType::Double>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Double>());
		break;
	case shade::NodeValueType::String:
		Initialize<NodeValueType::String>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::String>());
		break;
	case shade::NodeValueType::Pose:
		Initialize<NodeValueType::Pose>(); std::uint32_t dummy; shade::Serializer::Deserialize(stream, dummy);
		break;
		case shade::NodeValueType::BoneMask:
			Initialize<NodeValueType::BoneMask>(nullptr); shade::Serializer::Deserialize(stream, dummy);
		break;
	case shade::NodeValueType::Vector2:
		Initialize<NodeValueType::Vector2>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Vector2>());
		break;
	case shade::NodeValueType::Vector3:
		Initialize<NodeValueType::Vector3>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Vector3>());
		break;
	case shade::NodeValueType::Vector4:
		Initialize<NodeValueType::Vector4>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Vector4>());
		break;
	case shade::NodeValueType::Matrix3:
		Initialize<NodeValueType::Matrix3>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Matrix3>());
		break;
	case shade::NodeValueType::Matrix4:
		Initialize<NodeValueType::Matrix4>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Matrix4>());
		break;
	case shade::NodeValueType::Quaternion:
		Initialize<NodeValueType::Quaternion>(); shade::Serializer::Deserialize(stream, As<shade::NodeValueType::Quaternion>());
		break;
	}

	return 0u;
}
