#pragma once
#include <glm/glm/glm.hpp>
#include <shade/config/ShadeAPI.h>
#include <shade/core/animation/Pose.h>

namespace shade
{
	// Define default supported types for NodeValue
#ifndef AVALIBLE_NODE_VALUE_TYPES
#define AVALIBLE_NODE_VALUE_TYPES \
        bool,\
        std::int8_t,\
        std::int32_t, \
        std::uint8_t, \
        std::uint32_t, \
        float, \
        double, \
        std::string,\
        animation::Pose*,\
        animation::BoneMask,\
        glm::vec2,\
        glm::vec3,\
        glm::vec4,\
        glm::mat3,\
        glm::mat4,\
        glm::quat

#endif // !AVALIBLE_NODE_VALUE_TYPES

	template<typename T, typename... Ts>
	concept NodeValueOneOf = (std::is_same_v<T, Ts> || ...);

	template<typename T>
	concept IsNodeValueType = NodeValueOneOf<T, AVALIBLE_NODE_VALUE_TYPES>;

	// Enumeration representing possible types of NodeValue
	enum class NodeValueType : std::uint8_t
	{
		Undefined,  ///< Default undefined type
		Bool,       ///< bool type
		Int8,       ///< std::int8_t type
		Int,        ///< std::int32_t type
		Uint8,      ///< std::std::uint8_t type
		Uint,       ///< std::std::uint32_t type
		Float,      ///< Floating-point type
		Double,     ///< Double type
		String,     ///< std::string type
		Pose,       ///< animation::Pose* type
		BoneMask,   ///< animation::BoneMask
		Vector2,    ///< glm::vec2 type
		Vector3,    ///< glm::vec3 type
		Vector4,    ///< glm::vec4 type
		Matrix3,    ///< glm::mat3 type
		Matrix4,    ///< glm::mat4 type
		Quaternion, ///< glm::quat type
	};


	// Template specialization to map NodeValueType to corresponding C++ types
	template <typename NodeValueType T> struct FromNodeValueTypeToType { typedef void Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Bool> { typedef bool Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Int8> { typedef std::int8_t Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Int> { typedef std::int32_t Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Uint8> { typedef std::uint8_t Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Uint> { typedef std::uint32_t Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Float> { typedef float Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Double> { typedef double Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::String> { typedef std::string Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Pose> { typedef animation::Pose* Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::BoneMask> { typedef animation::BoneMask Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Vector2> { typedef glm::vec2 Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Vector3> { typedef glm::vec3 Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Vector4> { typedef glm::vec4 Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Matrix3> { typedef glm::mat3 Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Matrix4> { typedef glm::mat4 Type; };
	template <> struct FromNodeValueTypeToType<NodeValueType::Quaternion> { typedef glm::quat Type; };

	// Template specialization to map C++ types to NodeValueType
	template <typename T> struct FromTypeToNodeValueType { static NodeValueType const Type = NodeValueType::Undefined; };
	template <> struct FromTypeToNodeValueType<bool> { static NodeValueType const Type = NodeValueType::Bool; };
	template <> struct FromTypeToNodeValueType<std::int8_t> { static NodeValueType const Type = NodeValueType::Int8; };
	template <> struct FromTypeToNodeValueType<std::int32_t> { static NodeValueType const Type = NodeValueType::Int; };
	template <> struct FromTypeToNodeValueType<std::uint8_t> { static NodeValueType const Type = NodeValueType::Uint8; };
	template <> struct FromTypeToNodeValueType<std::uint32_t> { static NodeValueType const Type = NodeValueType::Uint; };
	template <> struct FromTypeToNodeValueType<float> { static NodeValueType const Type = NodeValueType::Float; };
	template <> struct FromTypeToNodeValueType<double> { static NodeValueType const Type = NodeValueType::Double; };
	template <> struct FromTypeToNodeValueType<std::string> { static NodeValueType const Type = NodeValueType::String; };
	template <> struct FromTypeToNodeValueType<animation::Pose*> { static NodeValueType const Type = NodeValueType::Pose; };
	template <> struct FromTypeToNodeValueType<animation::BoneMask> { static NodeValueType const Type = NodeValueType::BoneMask; };
	template <> struct FromTypeToNodeValueType<glm::vec2> { static NodeValueType const Type = NodeValueType::Vector2; };
	template <> struct FromTypeToNodeValueType<glm::vec3> { static NodeValueType const Type = NodeValueType::Vector3; };
	template <> struct FromTypeToNodeValueType<glm::vec4> { static NodeValueType const Type = NodeValueType::Vector4; };
	template <> struct FromTypeToNodeValueType<glm::mat3> { static NodeValueType const Type = NodeValueType::Matrix3; };
	template <> struct FromTypeToNodeValueType<glm::mat4> { static NodeValueType const Type = NodeValueType::Matrix4; };
	template <> struct FromTypeToNodeValueType<glm::quat> { static NodeValueType const Type = NodeValueType::Quaternion; };


	// Class representing a value with associated type
	class SHADE_API NodeValue
	{
	public:
		// Default constructor and destructor
		NodeValue() = default;
		~NodeValue() = default;
		//// Copy constructor
		//NodeValue(const NodeValue& other) = default;
		//// Move constructor
		//NodeValue(NodeValue&& other) noexcept = default;
		//// Copy assignment operator
		//NodeValue& operator=(const NodeValue& other) = default;
		//// Move assignment operator
		//NodeValue& operator=(NodeValue&& other) noexcept = default;
	public:
		/**
		 * @brief Template function to initialize the NodeValue with a specific type and value.
		 * @tparam Type - The NodeValueType to set.
		 * @tparam Args - The argument types for constructing the value.
		 * @param args - The actual arguments to construct the value.
		 */
		template <typename NodeValueType Type, typename... Args> requires IsNodeValueType<typename FromNodeValueTypeToType<Type>::Type>
		SHADE_INLINE void Initialize(Args &&... args)
		{
			m_Value = FromNodeValueTypeToType<Type>::Type(std::forward<Args>(args)...);
			m_Type = Type;
		}

		/**
		 * @brief Function to get value type.
		 * @return Internal value type.
		 */
		SHADE_INLINE NodeValueType GetType() const
		{
			return m_Type;
		}

		/**
		 * @brief Template function to cast the internal value to a specified type.
		 * @tparam Type - The type to cast the internal value to.
		 * @return Reference to the internal value casted to the specified type.
		 */
		template <typename NodeValueType Type> requires IsNodeValueType<typename FromNodeValueTypeToType<Type>::Type>
		SHADE_INLINE FromNodeValueTypeToType<Type>::Type& As()
		{
			return GetValue<Type>();
		}

		/**
		 * @brief Const version of the template function to cast the internal value to a specified type.
		 * @tparam Type - The type to cast the internal value to.
		 * @return Const reference to the internal value casted to the specified type.
		 */
		template <typename NodeValueType Type> requires IsNodeValueType<typename FromNodeValueTypeToType<Type>::Type>
		SHADE_INLINE const typename FromNodeValueTypeToType<Type>::Type& As() const
		{
			return GetValue<Type>();
		}

		/**
		 * @brief Conversion operator to implicitly convert NodeValue to a specified type.
		 * @tparam Type - The type to convert NodeValue to.
		 * @return Reference to the internal value casted to the specified type.
		 */
		template <typename Type> requires IsNodeValueType<typename FromTypeToNodeValueType<Type>::Type>
		SHADE_INLINE operator Type& ()
		{
			return GetValue<FromTypeToNodeValueType<Type>::Type>();
		}

		/**
		 * @brief Const version of the conversion operator to implicitly convert NodeValue to a specified type.
		 * @tparam Type - The type to convert NodeValue to.
		 * @return Const reference to the internal value casted to the specified type.
		 */
		template <typename Type> requires IsNodeValueType<typename FromTypeToNodeValueType<Type>::Type>
		SHADE_INLINE operator const Type& () const
		{
			return GetValue<FromTypeToNodeValueType<Type>::Type>();
		}

		/**
		 * @brief Assignment operator to assign a value of a specified type.
		 * @tparam Type - The type of the value to assign.
		 * @param value - The value to assign.
		 * @return Reference to the modified NodeValue.
		 */
		template <typename Type> requires IsNodeValueType<typename FromTypeToNodeValueType<Type>::Type>
		SHADE_INLINE NodeValue& operator=(Type&& value)
		{
			As<FromTypeToNodeValueType<Type>::Type>() = std::forward<Type>(value);
			return *this;
		}

	private:
		/**
		 * @brief Template function to get the value of a specific type.
		 * @tparam Type - The type to retrieve.
		 * @return Reference to the internal value casted to the specified type.
		 */
		template <typename NodeValueType Type> requires IsNodeValueType<typename FromNodeValueTypeToType<Type>::Type>
		SHADE_INLINE FromNodeValueTypeToType<Type>::Type& GetValue()
		{
			// Check if the value has been initialized and if the type matches
			assert(m_Value.index() != 0 && Type == m_Type && "Value has not been initialized or 'TYPE' was mismatched!");
			return std::get<FromNodeValueTypeToType<Type>::Type>(m_Value);
		}

		/**
		 * @brief Const version of the template function to get the value of a specific type.
		 * @tparam Type - The type to retrieve.
		 * @return Const reference to the internal value casted to the specified type.
		 */
		template <typename NodeValueType Type> requires IsNodeValueType<typename FromNodeValueTypeToType<Type>::Type>
		SHADE_INLINE const typename FromNodeValueTypeToType<Type>::Type& GetValue() const
		{
			// Check if the value has been initialized and if the type matches
			assert(m_Value.index() != 0 && Type == m_Type && "Value has not been initialized or 'TYPE' was mismatched!");
			return std::get<FromNodeValueTypeToType<Type>::Type>(m_Value);
		}

	private:
		// Variant to store the actual value, initialized with std::monostate
		std::variant<std::monostate, AVALIBLE_NODE_VALUE_TYPES> m_Value;

		// Enum representing the type of the stored value
		NodeValueType m_Type = NodeValueType::Undefined;
	};

	// Container for node values
	class SHADE_API NodeValues
	{
	public:
		// Alias for shared_ptr to NodeValue
		using Value = std::shared_ptr<NodeValue>;
		// Where Curent value and cashed default;
		using Values = std::pair<std::shared_ptr<NodeValue>, std::shared_ptr<NodeValue>>;
	public:

		// Default constructor and destructor
		NodeValues() = default;
		~NodeValues() = default;

		// Copy constructor
		NodeValues(const NodeValues& other) = default;

		// Move constructor marked as noexcept
		NodeValues(NodeValues&& other) noexcept = default;

		// Copy assignment operator
		NodeValues& operator=(const NodeValues& other) = default;

		// Move assignment operator marked as noexcept
		NodeValues& operator=(NodeValues&& other) noexcept = default;

		/**
		 * @brief Emplaces a new NodeValue with the specified type and arguments.
		 * @tparam Type - The NodeValueType to set.
		 * @tparam Args - The argument types for constructing the value.
		 * @param args - The actual arguments to construct the value.
		 * @return The index at which the new NodeValue is emplaced.
		 */
		template<typename NodeValueType Type, typename... Args>
		std::size_t Emplace(Args&&... args)
		{
			auto value = std::make_shared<NodeValue>();
			value->Initialize<Type>(std::forward<Args>(args)...);
			m_Values.emplace_back(value, value);
			return m_Values.size() - 1;
		}

		/**
		* @brief Removes the NodeValue at the specified index.
		* @param index - The index of the NodeValue to remove.
		*/
		void Remove(std::size_t index) { m_Values.erase(m_Values.begin() + index); }

		/**
		 * @brief Accesses the NodeValue at the specified index.
		 * @param index - The index of the NodeValue to access.
		 * @return Reference to the shared_ptr<NodeValue> at the specified index.
		 */
		SHADE_INLINE Value& At(std::size_t index) { return m_Values.at(index).first; }

		/**
		 * @brief Const version to access the NodeValue at the specified index.
		 * @param index - The index of the NodeValue to access.
		 * @return Const reference to the shared_ptr<NodeValue> at the specified index.
		 */
		SHADE_INLINE const Value& At(std::size_t index) const 
		{ 
			return m_Values.at(index).first; 
		}

		/**
		* @brief Reset to default initialized value.
		* @param index - The index of the NodeValue to access.
		*/

		SHADE_INLINE void Reset(std::size_t index) { m_Values.at(index).first = m_Values.at(index).second; }
		/**
		* @brief Get values count.
		* @return Count of currently created values.
		*/
		SHADE_INLINE std::size_t GetSize() const { return m_Values.size(); }
	public:
		/**
		 * @brief Iterator to the beginning of the NodeValues vector.
		 * @return Iterator pointing to the beginning of the NodeValues vector.
		 */
		SHADE_INLINE std::vector<Values>::iterator begin() { return m_Values.begin(); }

		/**
		 * @brief Iterator to the end of the NodeValues vector.
		 * @return Iterator pointing to the end of the NodeValues vector.
		 */
		SHADE_INLINE std::vector<Values>::iterator end() { return m_Values.end(); }

		/**
		 * @brief Const iterator to the beginning of the NodeValues vector.
		 * @return Const iterator pointing to the beginning of the NodeValues vector.
		 */
		SHADE_INLINE std::vector<Values>::const_iterator cbegin() const { return m_Values.cbegin(); }

		/**
		 * @brief Const iterator to the end of the NodeValues vector.
		 * @return Const iterator pointing to the end of the NodeValues vector.
		 */
		SHADE_INLINE std::vector<Values>::const_iterator cend() const { return m_Values.cend(); }

	public:
		/**
		 * @brief Accesses the NodeValue at the specified index using the subscript operator.
		 * @param index - The index of the NodeValue to access.
		 * @return Reference to the shared_ptr<NodeValue> at the specified index.
		 */
		SHADE_INLINE Value& operator[](std::size_t index) { return m_Values[index].first; }

		/**
		 * @brief Const version to access the NodeValue at the specified index using the subscript operator.
		 * @param index - The index of the NodeValue to access.
		 * @return Const reference to the shared_ptr<NodeValue> at the specified index.
		 */
		SHADE_INLINE const Value& operator[](std::size_t index) const { return m_Values[index].first; }

	private:
		// Vector to store shared_ptrs to NodeValue
		std::vector<Values> m_Values;
	};

}