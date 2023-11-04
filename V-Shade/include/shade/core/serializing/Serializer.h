#pragma once
#include <shade/config/ShadeAPI.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

namespace shade
{
	// NOTE: Prototype, need to be done in future !
	struct FileHeader
	{
		std::string Header;
		std::uint16_t Version;
		std::uint32_t CheckSum;
	};

	class SHADE_API Serializer
	{
	public:
		// Serrialize object into stream.
		template<typename T>
		static std::size_t Serialize(std::ostream& stream, const T&);
		// Deserialize object from stream.
		template<typename T>
		static std::size_t Deserialize(std::istream& stream, T&, std::size_t count = 1);
	};
	// Using specific for scene components serrializing
	class SHADE_API SceneComponentSerializer
	{
	public:
		// Serrialize object into stream.
		template<typename T>
		static std::size_t Serialize(std::ostream& stream, const T&);
		// Deserialize object from stream.
		template<typename T>
		static std::size_t Deserialize(std::istream& stream, T&, std::size_t count = 1);
	};
}

namespace shade
{
	template<typename T>
	inline std::size_t Serializer::Deserialize(std::istream& stream, T& value, std::size_t count)
	{
		return stream.read(reinterpret_cast<char*>(&value), sizeof(T) * count).tellg();
	}
	/* Serrialize std::uint32_t.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const std::uint32_t& value)
	{
		return stream.write(reinterpret_cast<const char*>(&value), sizeof(std::uint32_t)).tellp();
	}
	/* Deserialize std::uint32_t.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, std::uint32_t& value, std::size_t count)
	{
		return stream.read(reinterpret_cast<char*>(&value), sizeof(std::uint32_t) * count).tellg();
	}
	/////////////////////////////////////////////////////////////////////////////////////
	/* Serrialize std::uint64_t.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const std::uint64_t& value)
	{
		return stream.write(reinterpret_cast<const char*>(&value), sizeof(std::uint64_t)).tellp();
	}
	/* Deserialize std::uint64_t.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, std::uint64_t& value, std::size_t count)
	{
		return stream.read(reinterpret_cast<char*>(&value), sizeof(std::uint64_t) * count).tellg();
	}
	/////////////////////////////////////////////////////////////////////////////////////
	/* Serrialize bool.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const bool& value)
	{
		return stream.write(reinterpret_cast<const char*>(&value), sizeof(bool)).tellp();
	}
	/* Deserialize bool.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, bool& value, std::size_t count)
	{
		return stream.read(reinterpret_cast<char*>(&value), sizeof(bool) * count).tellg();
	}
	/////////////////////////////////////////////////////////////////////////////////////
	/* Serrialize float.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const float& value)
	{
		return stream.write(reinterpret_cast<const char*>(&value), sizeof(float)).tellp();
	}
	/* Deserialize float.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, float& value, std::size_t count)
	{
		return stream.read(reinterpret_cast<char*>(&value), sizeof(float) * count).tellg();
	}
	/////////////////////////////////////////////////////////////////////////////////////
	/* Serrialize char.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const char& value)
	{
		return stream.write(&value, sizeof(char)).tellp();
	}
	/* Deserialize char.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, char& value, std::size_t count)
	{
		return stream.read(&value, sizeof(char) * count).tellg();
	}
	/////////////////////////////////////////////////////////////////////////////////////
	/* Serrialize std::string. String's size will be std::uin32_t.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const std::string& string)
	{
		std::uint32_t size = static_cast<std::uint32_t>(string.size());
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect string size = {}", size));

		// If size is more then 0 need to write data.
		if (size)
			return stream.write(string.data(), sizeof(char) * (string.size() + 1)).tellp();
		else
			return stream.write("\0", sizeof(char) * 1).tellp();
	}
	/* Deserialize std::string. String's size will be std::uin32_t.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, std::string& string, std::size_t count)
	{
		const std::streampos begin = stream.tellg();
		char symbol = ' ';
		// Try to find null terminated character.
		do { stream.read(&symbol, sizeof(char)); } while (symbol != '\0');

		// Calculate string's size.
		const std::streampos end = stream.tellg();
		const std::uint32_t size = std::uint32_t((end - begin) - 1);

		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect string size = {}", size));

		if (size)
		{
			string.resize(size); stream.seekg(begin); stream.read(string.data(), sizeof(char) * (string.size())); return stream.seekg(end).tellg();
		}
		else
		{
			return end;
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////
	/* Serrialize std::vector<std::uint32_t>. Vector's size will be std::uin32_t.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const std::vector<std::uint32_t>& array)
	{
		std::uint32_t size = static_cast<std::uint32_t>(array.size());
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		// Write size first.
		stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
		// If size is more then 0 need to write data.
		if (size)
			return stream.write(reinterpret_cast<const char*>(array.data()), array.size() * sizeof(std::uint32_t)).tellp();
		else
			return stream.tellp();
	}
	/* Deserialize std::vector<std::uint32_t>. Vector's size will be std::uin32_t.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, std::vector<std::uint32_t>& array, std::size_t count)
	{
		std::uint32_t size = 0;
		// Read size first.
		stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		// If size is more then 0 need to read data.
		if (size)
		{
			array.resize(size);
			return stream.read(reinterpret_cast<char*>(array.data()), array.size() * sizeof(std::uint32_t)).tellg();
		}
		else
			return stream.tellg();
	}
	/////////////////////////////////////////////////////////////////////////////////////
	/* Serrialize std::vector<std::uint64_t>. Vector's size will be std::uin64_t.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const std::vector<std::uint64_t>& array)
	{
		std::uint64_t size = array.size();
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		// Write size first.
		stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint64_t));
		// If size is more then 0 need to write data.
		if (size)
			return stream.write(reinterpret_cast<const char*>(array.data()), array.size() * sizeof(std::uint64_t)).tellp();
		else
			return stream.tellp();
	}
	/* Deserialize std::vector<std::uint64_t>. Vector's size will be std::uin64_t.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, std::vector<std::uint64_t>& array, std::size_t count)
	{
		std::uint64_t size = 0;
		// Read size first.
		stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint64_t));
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		// If size is more then 0 need to read data.
		if (size)
		{
			array.resize(size);
			return stream.read(reinterpret_cast<char*>(array.data()), array.size() * sizeof(std::uint64_t)).tellg();
		}
		else
			return stream.tellg();
	}
	/////////////////////////////////////////////////////////////////////////////////////
	/* Serrialize std::unordered_map<std::string, std::uint32_t>. Map's size will be std::uin32_t.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const std::unordered_map<std::string, std::uint32_t>& map)
	{
		std::uint32_t size = static_cast<std::uint32_t>(map.size());
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		// Write size first.
		stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
		if (size)
		{
			for (auto& [str, value] : map)
			{
				Serialize<std::string>(stream, str);
				Serialize<std::uint32_t>(stream, value);
			}
		}
		return stream.tellp();
	}
	/* Deserialize std::unordered_map<std::string, std::uint32_t>. Map's size will be std::uin32_t.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, std::unordered_map<std::string, std::uint32_t>& map, std::size_t count)
	{
		std::uint32_t size = 0;
		// Read size first.
		stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		for (std::uint32_t i = 0; i < size; i++)
		{
			std::string key; std::uint32_t value;

			Deserialize<std::string>(stream, key);
			Deserialize<std::uint32_t>(stream, value);

			map.insert({ key, value });
		}
		return stream.tellg();
	}
	/////////////////////////////////////////////////////////////////////////////////////
	/* Serrialize std::unordered_map<std::string, std::uint64_t>. Map's size will be std::uin64_t.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const std::unordered_map<std::string, std::uint64_t>& map)
	{
		std::uint64_t size = map.size();
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		// Write size first.
		stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint64_t));
		if (size)
		{
			for (auto& [str, value] : map)
			{
				Serialize<std::string>(stream, str);
				Serialize<std::uint64_t>(stream, value);
			}
		}
		return stream.tellp();
	}
	/* Deserialize std::unordered_map<std::string, std::uint64_t>. Map's size will be std::uin64_t.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, std::unordered_map<std::string, std::uint64_t>& map, std::size_t count)
	{
		std::uint64_t size = 0;
		// Read size first.
		stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint64_t));
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		for (std::uint64_t i = 0; i < size; i++)
		{
			std::string key; std::uint64_t value;

			Deserialize<std::string>(stream, key);
			Deserialize<std::uint64_t>(stream, value);

			map.insert({ key, value });
		}
		return stream.tellg();
	}
	/////////////////////////////////////////////////////////////////////////////////////
	/* Serrialize std::unordered_map<std::string, std::string>. Map's size will be std::uin32_t.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const std::unordered_map<std::string, std::string>& map)
	{
		std::uint32_t size = static_cast<std::uint32_t>(map.size());
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		// Write size first.
		stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
		if (size)
		{
			for (auto& [str, value] : map)
			{
				Serialize<std::string>(stream, str);
				Serialize<std::string>(stream, value);
			}
		}
		return stream.tellp();
	}
	/* Deserialize std::unordered_map<std::string, std::string>. Map's size will be std::uin32_t.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, std::unordered_map<std::string, std::string>& map, std::size_t count)
	{
		std::uint32_t size = 0;
		// Read size first.
		stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		for (std::uint32_t i = 0; i < size; i++)
		{
			std::string key; std::string value;

			Deserialize<std::string>(stream, key);
			Deserialize<std::string>(stream, value);

			map.insert({ key, value });
		}
		return stream.tellg();
	}
	/* Serrialize glm::vec3.*/
	template<>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const glm::vec3& value)
	{
		return stream.write(reinterpret_cast<const char*>(glm::value_ptr(value)), sizeof(glm::vec3)).tellp();
	}
	/* Deserialize glm::vec3.*/
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, glm::vec3& value, std::size_t count)
	{
		return stream.read(reinterpret_cast<char*>(glm::value_ptr(value)), sizeof(glm::vec3)).tellg();
	}
	/////////////////////////////////////////////////////////////////////////////////////
	template<>
	inline std::size_t SceneComponentSerializer::Serialize(std::ostream& stream, const std::string &value)
	{
		return Serializer::Serialize(stream, value);
	}

	template<>
	inline std::size_t SceneComponentSerializer::Deserialize(std::istream& stream, std::string& value, std::size_t count)
	{
		return Serializer::Deserialize(stream, value);
	}
}