#pragma once
#include <shade/config/ShadeAPI.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <shade/utils/Logger.h>

namespace shade
{
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

	// Templated function to generate a checksum of content.
	// This function is used to create a checksum of a given stringstream.
	// @tparam T The type of the checksum value (either std::uint32_t or std::uint64_t).
	// @param content The input string for which the checksum is generated.
	// @return The generated checksum of type T.
	template <typename T, typename = std::enable_if_t<std::is_same<T, std::uint32_t>::value || std::is_same<T, std::uint64_t>::value>>
	T GenerateCheckSum(const std::stringstream& stream) { return static_cast<T>(std::hash<std::string>{}(stream.str())); }
	// Templated function to generate a checksum of content.
	// This function is used to create a checksum of a given string.
	// @tparam T The type of the checksum value (either std::uint32_t or std::uint64_t).
	// @param content The input string for which the checksum is generated.
	// @return The generated checksum of type T.
	template <typename T, typename = std::enable_if_t<std::is_same<T, std::uint32_t>::value || std::is_same<T, std::uint64_t>::value>>
	T GenerateCheckSum(const std::string& stream) { return static_cast<T>(std::hash<std::string>{}(stream)); }

	class SHADE_API File
	{
		#define SHADE_META_FILE_PATH "./data/META.bin"

		using checksum_t	 = std::uint32_t;
		using version_t		 = std::uint16_t;
		using magic_t		 = std::string;
		using content_size_t = checksum_t;

	public:
		using FileFlag = int;

		static constexpr int In				= 0x01;
		static constexpr int Out			= 0x02;
		static constexpr int SkipMagic		= 0x04;
		static constexpr int SkipVersion	= 0x08;
		static constexpr int SkipSize		= 0x10;
		static constexpr int SkipChecksum	= 0x20;

		struct Header
		{
			magic_t          Magic;
			version_t        Version  = 0u;
			content_size_t   Size     = 0u;
			checksum_t		 CheckSum = 0u;
		};

		struct Specification
		{
			// Where format -> Current path
			std::unordered_map<std::string, std::vector<std::string>> FormatPath;
			// Where format -> Packet path
			std::unordered_map<std::string, std::string> FormatPacketPath;
			// Path to meta file
			std::string MetaFilePath = SHADE_META_FILE_PATH;
		};

	public:
		File() = default;
		File(const std::string& filePath, FileFlag flag, const magic_t& magic = "@", version_t version = version_t(0));
		virtual ~File();

	public:
		/**
		* @brief Opens a file for reading or writing, depending on the flag.
		*
		* @param filePath  The path to the file.
		* @param version   The expected file version.
		* @param magic     The magic string to identify the file format.
		* @param flag      The file open flag (ReadFile or WriteFile).
		* @return True if the file was opened successfully; otherwise, false.
		*/
		bool OpenEngineFile(const std::string& filePath, FileFlag flag, const magic_t& magic = "@", version_t version = version_t(0));
		/**
		* @brief Opens a file for general use (not yet implemented).
		*
		* @param filePath  The path to the file.
		* @return True if the file was opened successfully; otherwise, false.
		*/
		bool OpenFile(const std::string& filePath);
		/**
		* @brief Checks if the file is open.
		*
		* @return True if the file is open; otherwise, false.
		*/
		bool IsOpen() const;
		/**
		 * @brief Return true if the end of the file has been reached.
		 */
		bool Eof();
		/**
		 * @brief Closes the file, updating the checksum if it was a write operation.
		 */
		void CloseFile();
		/**
		 * @brief Gets the size of the file.
		 *
		 * @return The size of the file in bytes.
		 */
		std::size_t _GetSize();
		/**
		 * @brief Generates a version number based on major, minor, and patch values.
		 *
		 * @param major Major version.
		 * @param minor Minor version.
		 * @param patch Patch version.
		 * @return The generated version number.
		 */
		static version_t VERSION(version_t major, version_t minor, version_t patch);
		/**
		 * @brief Writes a value of a templated type to the file.
		 *
		 * @param value The value to be written to the file.
		 * @return The number of bytes written.
		 */
		template<typename T>
		std::size_t Write(const T& value);
		/**
		 * @brief Reads a value of a templated type from the file.
		 *
		 * @param value The value read from the file.
		 * @return The number of bytes read.
		 */
		template<typename T>
		std::size_t Read(T& value);

		static std::unordered_map<std::string, std::vector<std::string>> FindFilesWithExtension(const std::filesystem::path& directory, const std::vector<std::string>& extensions = std::vector<std::string>());
		static std::unordered_map<std::string, std::vector<std::string>> FindFilesWithExtensionExclude(const std::filesystem::path& directory, const std::vector<std::string>& excludeExtensions = std::vector<std::string>());

		static void PackFiles(const shade::File::Specification& specification);
		static void InitializeMetaFile(const std::string& filepath = SHADE_META_FILE_PATH);
	private:
		std::string m_Path;
		FileFlag	m_Flag;
		Header m_Header;
		std::fstream m_File;
		std::stringstream m_Stream;
		std::size_t m_ContentPosition;
	private:
		/**
		 * @brief Reads the file header and performs integrity checks.
		 *
		 * @param stream The input stream to read from.
		 * @param version The expected file version.
		 * @param magic The magic string to identify the file format.
		 */
		void ReadHeader(std::istream& stream, version_t version, const magic_t& magic, FileFlag flag);
		/**
		 * @brief Writes the file header.
		 *
		 * @param stream The output stream to write to.
		 * @param version The file version to write to the header.
		 * @param magic The magic string to identify the file format.
		 */
		void WriteHeader(std::ostream& stream, version_t version, const magic_t& magic, FileFlag flag);
		/**
		* @brief Updates the checksum in the file.
		*/
		void UpdateChecksum();
		/**
		 * @brief Calculates and returns the checksum of the file content.
		 *
		 * @return The calculated checksum.
		 */
		checksum_t GetChecksum();
	private:
		// Where string - > path, string - > path inside packet, uint32_t position in packet
		static std::unordered_map<std::string, std::pair<std::string, std::uint32_t>> m_PathMap;
	};
	template<typename T>
	inline std::size_t File::Write(const T& value)
	{
		assert((m_Flag & shade::File::Out) && "Cannot write into file, 'Out' flag is not set !");
		return Serializer::Serialize(m_Stream, value);
	}
	template<typename T>
	inline std::size_t File::Read(T& value)
	{
		assert((m_Flag & shade::File::In) && "Cannot read from file, 'In' flag is not set !");
		return Serializer::Deserialize(m_Stream, value);
	}
}

namespace shade
{
	template<typename T>
	inline std::size_t Serializer::Deserialize(std::istream& stream, T& value, std::size_t count)
	{
		return stream.read(reinterpret_cast<char*>(&value), sizeof(T) * count).tellg();
	}
	template<typename T>
	inline std::size_t Serializer::Serialize(std::ostream& stream, const T& value)
	{
		return stream.write(reinterpret_cast<const char*>(&value), sizeof(T)).tellp();
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
		do { stream.read(&symbol, sizeof(char)); } while (symbol != '\0' && stream.good());

		// Calculate string's size.
		const std::streampos end = stream.tellg();
		const std::uint32_t size = std::uint32_t((end - begin) - 1);

		if (size + 1 == UINT32_MAX || size < 0)
			throw std::out_of_range(std::format("Incorrect string size = {}", size + 1));

		if (size)
		{
			string.resize(size); stream.seekg(begin); stream.read(string.data(), sizeof(char) * (string.size())); return stream.seekg(end).tellg();
		}
		return end;
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