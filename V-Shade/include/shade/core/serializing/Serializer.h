#pragma once
#include <shade/config/ShadeAPI.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <shade/utils/Logger.h>

namespace shade
{
	bool IsAllowedCharacter(const char c); std::string RemoveNotAllowedCharacters(const std::string& input);
	
	namespace serialize
	{
		/**
		 * @brief Provides static methods for serializing and deserializing objects.
		 *
		 * The `Serializer` struct includes template methods for serializing and deserializing objects to and from streams.
		 * It supports types that are trivially copyable and have a standard layout, excluding pointers.
		 */
		struct Serializer
		{
            /**
            * @brief Serializes an object and writes it to the output stream.
            *
            * Serializes the given object and writes its binary representation to the provided output stream.
            * This method is enabled only for types that are trivially copyable, have a standard layout,
            * and are not pointers. If the type does not meet these criteria, a static assertion fails.
            *
            * @tparam T The type of the object to be serialized.
            * @param stream The output stream where the object will be written.
            * @param obj The object to be serialized.
            */
            template<typename T>
            static void Serialize(std::ostream& stream, const T& obj)
            {
                if constexpr (std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && !std::is_pointer_v<T>)
                {
                    // Write the raw binary representation of the object to the stream
                    stream.write(reinterpret_cast<const char*>(&obj), sizeof(T));
                }
                else
                {
                    // Static assertion to ensure the type is serializable
                    static_assert(false, "You are trying to serialize a type that is not trivial or raw pointer and does not have a special Serialize specialization!");
                }
            }

			/**
			 * @brief Serializes an object and writes it to the output stream.
			 *
			 * Serializes the given object and writes its binary representation to the provided output stream.
			 * This method is enabled only for types that are trivially copyable, have a standard layout,
			 * and are not pointers. If the type does not meet these criteria, a static assertion fails.
			 *
			 * @tparam T The type of the object to be serialized.
			 * @param stream The output stream where the object will be written.
			 * @param obj The object to be serialized.
			 */
			template<typename T>
			static void Serialize(std::ostream& stream, const T& obj, std::size_t count)
			{
				if constexpr (std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && !std::is_pointer_v<T>)
				{
					// Write the raw binary representation of the object to the stream
					stream.write(reinterpret_cast<const char*>(&obj), sizeof(T) * count);
				}
				else
				{
					// Static assertion to ensure the type is serializable
					static_assert(false, "You are trying to serialize a type that is not trivial or raw pointer and does not have a special Serialize specialization!");
				}
			}
           
            /**
             * @brief Deserializes an object from the input stream.
             *
             * Reads the binary representation of an object from the provided input stream and deserializes it.
             * This method is enabled only for types that are trivially copyable, have a standard layout,
             * and are not pointers. If the type does not meet these criteria, a static assertion fails.
             *
             * @tparam T The type of the object to be deserialized.
             * @param stream The input stream from which the object will be read.
             * @param obj The object to be deserialized.
             */
            template<typename T>
            static void Deserialize(std::istream& stream, T& obj)
            {
                if constexpr (std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && !std::is_pointer_v<T>)
                {
                    // Read the raw binary representation of the object from the stream
                    stream.read(reinterpret_cast<char*>(&obj), sizeof(T));
                }
                else
                {
                    // Static assertion to ensure the type is deserializable
                    static_assert(false, "You are trying to deserialize a type that is not trivial or raw pointer and does not have a special Deserialize specialization!");
                }
            }

            /**
             * @brief Deserializes an object from the input stream.
             *
             * Reads the binary representation of an object from the provided input stream and deserializes it.
             * This method is enabled only for types that are trivially copyable, have a standard layout,
             * and are not pointers. If the type does not meet these criteria, a static assertion fails.
             *
             * @tparam T The type of the object to be deserialized.
             * @param stream The input stream from which the object will be read.
             * @param obj The object to be deserialized.
             */
            template<typename T>
            static void Deserialize(std::istream& stream, T& obj, std::size_t count)
            {
                if constexpr (std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T> && !std::is_pointer_v<T>)
                {
                    // Read the raw binary representation of the object from the stream
                    stream.read(reinterpret_cast<char*>(&obj), sizeof(T) * count);
                }
                else
                {
                    // Static assertion to ensure the type is deserializable
                    static_assert(false, "You are trying to deserialize a type that is not trivial or raw pointer and does not have a special Deserialize specialization!");
                }
            }

		};
	}
}

namespace shade
{
    namespace serialize
    {
        /**
         * @brief Serialize a std::string to an output stream.
         *
         * Serializes a `std::string` by first writing its size (as `std::uint32_t`) followed by the string data.
         * If the string size is `UINT32_MAX`, an exception is thrown.
         *
         * @param stream The output stream to write to.
         * @param string The string to be serialized.
         *
         * @throw std::out_of_range if the string size exceeds UINT32_MAX.
         */
        template<>
        inline void Serializer::Serialize(std::ostream& stream, const std::string& string)
        {
            std::uint32_t size = static_cast<std::uint32_t>(string.size());
            if (size == UINT32_MAX)
                throw std::out_of_range(std::format("Incorrect string size = {}", size));

            // Write the string data including the null terminator
            if (size)
                stream.write(string.data(), sizeof(char) * (string.size() + 1));
            else
                stream.write("\0", sizeof(char) * 1);
        }

        /**
         * @brief Deserialize a std::string from an input stream.
         *
         * Reads a `std::string` from the input stream by first locating the null terminator.
         * The size of the string is calculated based on the distance between the initial position and the position at the null terminator.
         * If the size is incorrect or negative, an exception is thrown.
         *
         * @param stream The input stream to read from.
         * @param string The string to be deserialized.
         *
         * @throw std::out_of_range if the calculated string size is incorrect.
         */
        template<>
        inline void Serializer::Deserialize(std::istream& stream, std::string& string)
        {
            const std::streampos begin = stream.tellg();
            char symbol = ' ';
            // Locate the null terminator
            do { stream.read(&symbol, sizeof(char)); } while (symbol != '\0' && stream.good());

            // Calculate the size of the string
            const std::streampos end = stream.tellg();
            const std::uint32_t size = std::uint32_t((end - begin) - 1);

            if (size + 1 == UINT32_MAX || size < 0)
                throw std::out_of_range(std::format("Incorrect string size = {}", size + 1));

            if (size)
            {
                string.resize(size);
                stream.seekg(begin);
                stream.read(string.data(), sizeof(char) * (string.size()));
                stream.seekg(end);
            }
        }

        /**
         * @brief Serialize a std::vector<std::uint32_t> to an output stream.
         *
         * Serializes a `std::vector<std::uint32_t>` by first writing its size (as `std::uint32_t`),
         * followed by the vector's data.
         * If the vector size is `UINT32_MAX`, an exception is thrown.
         *
         * @param stream The output stream to write to.
         * @param array The vector to be serialized.
         *
         * @throw std::out_of_range if the vector size exceeds UINT32_MAX.
         */
        template<>
        inline void Serializer::Serialize(std::ostream& stream, const std::vector<std::uint32_t>& array)
        {
            std::uint32_t size = static_cast<std::uint32_t>(array.size());
            if (size == UINT32_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Write the size of the vector
            stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
            // Write the vector's data
            if (size)
                stream.write(reinterpret_cast<const char*>(array.data()), array.size() * sizeof(std::uint32_t));
        }

        /**
         * @brief Deserialize a std::vector<std::uint32_t> from an input stream.
         *
         * Reads a `std::vector<std::uint32_t>` from the input stream by first reading its size (as `std::uint32_t`),
         * followed by the vector's data.
         * If the size is `UINT32_MAX`, an exception is thrown.
         *
         * @param stream The input stream to read from.
         * @param array The vector to be deserialized.
         *
         * @throw std::out_of_range if the vector size exceeds UINT32_MAX.
         */
        template<>
        inline void Serializer::Deserialize(std::istream& stream, std::vector<std::uint32_t>& array)
        {
            std::uint32_t size = 0;
            // Read the size of the vector
            stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
            if (size == UINT32_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Read the vector's data
            if (size)
            {
                array.resize(size);
                stream.read(reinterpret_cast<char*>(array.data()), array.size() * sizeof(std::uint32_t));
            }
        }

        /**
         * @brief Serialize a std::vector<std::uint64_t> to an output stream.
         *
         * Serializes a `std::vector<std::uint64_t>` by first writing its size (as `std::uint64_t`),
         * followed by the vector's data.
         * If the vector size is `UINT64_MAX`, an exception is thrown.
         *
         * @param stream The output stream to write to.
         * @param array The vector to be serialized.
         *
         * @throw std::out_of_range if the vector size exceeds UINT64_MAX.
         */
        template<>
        inline void Serializer::Serialize(std::ostream& stream, const std::vector<std::uint64_t>& array)
        {
            std::uint64_t size = array.size();
            if (size == UINT64_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Write the size of the vector
            stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint64_t));
            // Write the vector's data
            if (size)
                stream.write(reinterpret_cast<const char*>(array.data()), array.size() * sizeof(std::uint64_t));
        }

        /**
         * @brief Deserialize a std::vector<std::uint64_t> from an input stream.
         *
         * Reads a `std::vector<std::uint64_t>` from the input stream by first reading its size (as `std::uint64_t`),
         * followed by the vector's data.
         * If the size is `UINT64_MAX`, an exception is thrown.
         *
         * @param stream The input stream to read from.
         * @param array The vector to be deserialized.
         *
         * @throw std::out_of_range if the vector size exceeds UINT64_MAX.
         */
        template<>
        inline void Serializer::Deserialize(std::istream& stream, std::vector<std::uint64_t>& array)
        {
            std::uint64_t size = 0;
            // Read the size of the vector
            stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint64_t));
            if (size == UINT64_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Read the vector's data
            if (size)
            {
                array.resize(size);
                stream.read(reinterpret_cast<char*>(array.data()), array.size() * sizeof(std::uint64_t));
            }
        }

        /**
         * @brief Serialize a std::vector<float> to an output stream.
         *
         * Serializes a `std::vector<float>` by first writing its size (as `std::uint32_t`),
         * followed by the vector's data.
         * If the vector size is `UINT32_MAX`, an exception is thrown.
         *
         * @param stream The output stream to write to.
         * @param array The vector to be serialized.
         *
         * @throw std::out_of_range if the vector size exceeds UINT32_MAX.
         */
        template<>
        inline void Serializer::Serialize(std::ostream& stream, const std::vector<float>& array)
        {
            std::uint32_t size = static_cast<std::uint32_t>(array.size());
            if (size == UINT32_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Write the size of the vector
            stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
            // Write the vector's data
            if (size)
                stream.write(reinterpret_cast<const char*>(array.data()), array.size() * sizeof(float));
        }

        /**
         * @brief Deserialize a std::vector<float> from an input stream.
         *
         * Reads a `std::vector<float>` from the input stream by first reading its size (as `std::uint32_t`),
         * followed by the vector's data.
         * If the size is `UINT32_MAX`, an exception is thrown.
         *
         * @param stream The input stream to read from.
         * @param array The vector to be deserialized.
         *
         * @throw std::out_of_range if the vector size exceeds UINT32_MAX.
         */
        template<>
        inline void Serializer::Deserialize(std::istream& stream, std::vector<float>& array)
        {
            std::uint32_t size = 0;
            // Read the size of the vector
            stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
            if (size == UINT32_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Read the vector's data
            if (size)
            {
                array.resize(size);
                stream.read(reinterpret_cast<char*>(array.data()), array.size() * sizeof(float));
            }
        }

        /**
         * @brief Serialize a std::unordered_map<std::string, std::uint32_t> to an output stream.
         *
         * Serializes a `std::unordered_map<std::string, std::uint32_t>` by first writing its size (as `std::uint32_t`),
         * followed by the map's keys and values.
         * If the map size is `UINT32_MAX`, an exception is thrown.
         *
         * @param stream The output stream to write to.
         * @param map The map to be serialized.
         *
         * @throw std::out_of_range if the map size exceeds UINT32_MAX.
         */
        template<>
        inline void Serializer::Serialize(std::ostream& stream, const std::unordered_map<std::string, std::uint32_t>& map)
        {
            std::uint32_t size = static_cast<std::uint32_t>(map.size());
            if (size == UINT32_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Write the size of the map
            stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
            // Write each key-value pair
            if (size)
            {
                for (auto& [str, value] : map)
                {
                    Serialize<std::string>(stream, str);
                    Serialize<std::uint32_t>(stream, value);
                }
            }
        }

        /**
         * @brief Deserialize a std::unordered_map<std::string, std::uint32_t> from an input stream.
         *
         * Reads a `std::unordered_map<std::string, std::uint32_t>` from the input stream by first reading its size (as `std::uint32_t`),
         * followed by each key-value pair.
         * If the size is `UINT32_MAX`, an exception is thrown.
         *
         * @param stream The input stream to read from.
         * @param map The map to be deserialized.
         *
         * @throw std::out_of_range if the map size exceeds UINT32_MAX.
         */
        template<>
        inline void Serializer::Deserialize(std::istream& stream, std::unordered_map<std::string, std::uint32_t>& map)
        {
            std::uint32_t size = 0;
            // Read the size of the map
            stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
            if (size == UINT32_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Read each key-value pair
            for (std::uint32_t i = 0; i < size; i++)
            {
                std::string key;
                std::uint32_t value;

                Deserialize<std::string>(stream, key);
                Deserialize<std::uint32_t>(stream, value);

                map.insert({ key, value });
            }
        }

        /**
         * @brief Serialize a std::unordered_map<std::string, std::uint64_t> to an output stream.
         *
         * Serializes a `std::unordered_map<std::string, std::uint64_t>` by first writing its size (as `std::uint64_t`),
         * followed by the map's keys and values.
         * If the map size is `UINT64_MAX`, an exception is thrown.
         *
         * @param stream The output stream to write to.
         * @param map The map to be serialized.
         *
         * @throw std::out_of_range if the map size exceeds UINT64_MAX.
         */
        template<>
        inline void Serializer::Serialize(std::ostream& stream, const std::unordered_map<std::string, std::uint64_t>& map)
        {
            std::uint64_t size = map.size();
            if (size == UINT64_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Write the size of the map
            stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint64_t));
            // Write each key-value pair
            if (size)
            {
                for (auto& [str, value] : map)
                {
                    Serialize<std::string>(stream, str);
                    Serialize<std::uint64_t>(stream, value);
                }
            }
        }

        /**
         * @brief Deserialize a std::unordered_map<std::string, std::uint64_t> from an input stream.
         *
         * Reads a `std::unordered_map<std::string, std::uint64_t>` from the input stream by first reading its size (as `std::uint64_t`),
         * followed by each key-value pair.
         * If the size is `UINT64_MAX`, an exception is thrown.
         *
         * @param stream The input stream to read from.
         * @param map The map to be deserialized.
         *
         * @throw std::out_of_range if the map size exceeds UINT64_MAX.
         */
        template<>
        inline void Serializer::Deserialize(std::istream& stream, std::unordered_map<std::string, std::uint64_t>& map)
        {
            std::uint64_t size = 0;
            // Read the size of the map
            stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint64_t));
            if (size == UINT64_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Read each key-value pair
            for (std::uint64_t i = 0; i < size; i++)
            {
                std::string key;
                std::uint64_t value;

                Deserialize<std::string>(stream, key); Deserialize<std::uint64_t>(stream, value);
                map.emplace(key, value);
            }
        }

        /**
         * @brief Serialize a std::unordered_map<std::string, std::string> to an output stream.
         *
         * Serializes a `std::unordered_map<std::string, std::string>` by first writing its size (as `std::uint32_t`),
         * followed by the map's keys and values.
         * If the map size is `UINT32_MAX`, an exception is thrown.
         *
         * @param stream The output stream to write to.
         * @param map The map to be serialized.
         *
         * @throw std::out_of_range if the map size exceeds UINT32_MAX.
         */
        template<>
        inline void Serializer::Serialize(std::ostream& stream, const std::unordered_map<std::string, std::string>& map)
        {
            std::uint32_t size = static_cast<std::uint32_t>(map.size());
            if (size == UINT32_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Write the size of the map
            stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
            // Write each key-value pair
            if (size)
            {
                for (auto& [str, value] : map)
                {
                    Serialize<std::string>(stream, str);
                    Serialize<std::string>(stream, value);
                }
            }
        }

        /**
         * @brief Deserialize a std::unordered_map<std::string, std::string> from an input stream.
         *
         * Reads a `std::unordered_map<std::string, std::string>` from the input stream by first reading its size (as `std::uint32_t`),
         * followed by each key-value pair.
         * If the size is `UINT32_MAX`, an exception is thrown.
         *
         * @param stream The input stream to read from.
         * @param map The map to be deserialized.
         *
         * @throw std::out_of_range if the map size exceeds UINT32_MAX.
         */
        template<>
        inline void Serializer::Deserialize(std::istream& stream, std::unordered_map<std::string, std::string>& map)
        {
            std::uint32_t size = 0;
            // Read the size of the map
            stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
            if (size == UINT32_MAX)
                throw std::out_of_range(std::format("Incorrect array size = {}", size));

            // Read each key-value pair
            for (std::uint32_t i = 0; i < size; i++)
            {
                std::string key;
                std::string value;

                Deserialize<std::string>(stream, key);
                Deserialize<std::string>(stream, value);

                map.insert({ key, value });
            }
        }

        /**
         * @brief Serialize a glm::vec3 to an output stream.
         *
         * Serializes a `glm::vec3` by writing its binary representation to the output stream.
         *
         * @param stream The output stream to write to.
         * @param value The glm::vec3 to be serialized.
         */
        template<>
        inline void Serializer::Serialize(std::ostream& stream, const glm::vec3& value)
        {
            stream.write(reinterpret_cast<const char*>(glm::value_ptr(value)), sizeof(glm::vec3));
        }

        /**
         * @brief Deserialize a glm::vec3 from an input stream.
         *
         * Reads a `glm::vec3` from the input stream by reading its binary representation.
         *
         * @param stream The input stream to read from.
         * @param value The glm::vec3 to be deserialized.
         */
        template<>
        inline void Serializer::Deserialize(std::istream& stream, glm::vec3& value)
        {
            stream.read(reinterpret_cast<char*>(glm::value_ptr(value)), sizeof(glm::vec3));
        }
    }
}