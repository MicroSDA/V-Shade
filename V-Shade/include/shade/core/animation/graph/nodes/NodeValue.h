#pragma once
#include <glm/glm/glm.hpp>
#include <shade/config/ShadeAPI.h>
#include <shade/core/animation/Pose.h>

// TODO: Include Pose, pose has to be as NodeValuType

namespace shade
{
	namespace animation
	{

#define NODE_VALUE_AVALIBLE_LIST int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, bool, float, double, std::string, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4, Pose*
#define REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(node_value, type) \
		template<> struct FromTypeToNodeValueType<type> { static NodeValueType const Type = NodeValueType::##node_value; };
#define REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(node_value, type) \
		template<> struct FromNodeValueTypeToType<NodeValueType::##node_value> { typedef type Type; };
	
		template<typename T, typename... Ts>
		concept NodeValueOneOf = (std::is_same_v<T, Ts> || ...);

		template<typename T>
		concept IsNodeValueType = NodeValueOneOf<T, NODE_VALUE_AVALIBLE_LIST>;

		enum class NodeValueType : std::uint8_t
		{
			UNDEFINED,

			INT_8,
			INT,

			UINT_8,
			UINT,

			BOOL,
			FLOAT,
			DOUBLE,

			STRING,

			VECTOR_2,
			VECTOR_3,
			VECTOR_4,

			MATRIX_3,
			MATRIX_4,

			POSE
		};

		// TODO: Add name and hash !
		template<typename T> struct FromTypeToNodeValueType { static NodeValueType const Type = NodeValueType::UNDEFINED; };

		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(INT_8,	std::int8_t)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(INT,		std::int32_t)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(UINT_8,	std::uint8_t)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(UINT,		std::uint32_t)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(BOOL,		bool)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(FLOAT,	float)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(DOUBLE,	double)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(STRING,	std::string)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(VECTOR_2,	glm::vec2)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(VECTOR_3,	glm::vec3)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(VECTOR_4,	glm::vec4)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(MATRIX_3,	glm::mat3)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(MATRIX_4,	glm::mat4)
		REGISTER_FROM_TYPE_TO_NODE_VALUE_TYPE(POSE,		Pose*)

		template<typename NodeValueType T>	struct FromNodeValueTypeToType { typedef void Type; };

		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(INT_8,	std::int8_t)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(INT,		std::int32_t)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(UINT_8,	std::uint8_t)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(UINT,		std::uint32_t)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(BOOL,		bool)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(FLOAT,	float)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(DOUBLE,	double)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(STRING,	std::string)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(VECTOR_2, glm::vec2)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(VECTOR_3, glm::vec3)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(VECTOR_4, glm::vec4)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(MATRIX_3, glm::mat3)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(MATRIX_4, glm::mat4)
		REGISTER_FROM_NODE_VALUE_TYPE_TO_TYPE(POSE,		Pose*)

		class SHADE_API NodeValue
		{
		public:
			NodeValue() : m_Type(NodeValueType::UNDEFINED) { }
			virtual ~NodeValue() = default;
		public:
			template<typename NodeValueType T, typename... Args> requires IsNodeValueType<typename FromNodeValueTypeToType<T>::Type>
			inline void Set(Args&&... args) { m_Value = FromNodeValueTypeToType<T>::Type(std::forward<Args>(args)...); m_Type = T; }

			template<typename NodeValueType T> requires IsNodeValueType<typename FromNodeValueTypeToType<T>::Type>
			inline FromNodeValueTypeToType<T>::Type& Get() { assert(m_Type != NodeValueType::UNDEFINED && "Or value has not been set !"); return std::get<FromNodeValueTypeToType<T>::Type>(m_Value); }

			inline NodeValueType GetType() const { return m_Type; }
		private:
			std::variant<NODE_VALUE_AVALIBLE_LIST> m_Value;
			NodeValueType m_Type;
		};
	}
}