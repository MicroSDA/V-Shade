#include "shade_pch.h"
#include "Serializer.h"

std::unordered_map<std::string, std::pair<std::string, std::uint32_t>> shade::File::m_PathMap;

shade::File::File(const std::string& filePath, FileFlag flag, const magic_t& magic, version_t version)
{
	OpenEngineFile(filePath, flag, magic, version);
}

shade::File::~File()
{
	CloseFile();
}

bool shade::File::OpenEngineFile(const std::string& filePath, FileFlag flag, const magic_t& magic, version_t version)
{
	m_Flag = flag;	m_Path = filePath;

	try
	{
		if ((flag & shade::File::In))
		{
			if (std::filesystem::exists(filePath))
			{
				m_File.open(filePath, std::ios::in | std::ios::binary);
				if (!m_File.is_open()) return false;

				SHADE_CORE_DEBUG("Open the file directly from file system for reading, path = {}", m_Path);

				ReadHeader(m_File, version, magic, flag); return true;
			}
			else
			{
				const auto packed = m_PathMap.find(filePath);

				if (packed != m_PathMap.end())
				{
					m_File.open(packed->second.first, std::ios::in | std::ios::binary);
					if (!m_File.is_open()) return false;

					// Read packet header without content
					ReadHeader(m_File, version, "@s_packet", In | SkipSize | SkipChecksum, true);
					// Set file start position inside packet
					m_File.seekp(packed->second.second);

					SHADE_CORE_DEBUG("Open the file from packet for reading, path = {}", m_Path);

					// Read file header
					ReadHeader(m_File, version, magic, flag); return true;
				}
				else
				{
					SHADE_CORE_WARNING("Couldn't find the file, file doesn't exist, path = {}", m_Path);
					return false;
				}
			}
		}
		else if ((flag & shade::File::Out))
		{
			m_File.open(filePath, std::ios::out | std::ios::binary);
			if (!m_File.is_open()) throw std::runtime_error(std::format("Failed to open file, path = {}", filePath));

			SHADE_CORE_DEBUG("Open the file directly from file system for writing, path = {}", m_Path);

			WriteHeader(m_File, version, magic, flag); return true;
		}
		else
		{
			throw std::runtime_error(std::format("Failed to open file, 'In || Out' flags are not specified, path = {}", filePath));
		}
	}
	catch (std::exception& exception)
	{
		SHADE_CORE_ERROR("Exception: {}", exception.what()); m_File.close(); return false;
	}
}

bool shade::File::OpenFile(const std::string& filePath)
{
	assert(false);
	return false;
}

bool shade::File::IsOpen() const
{
	return m_File.is_open();
}

bool shade::File::Eof()
{
	return (m_Stream.peek() == EOF);
}

void shade::File::CloseFile()
{
	if (m_File.is_open())
	{
		if ((m_Flag & shade::File::Out))
		{
			m_File << m_Stream.rdbuf();
			
			if(!(m_Flag & shade::File::SkipChecksum))
				UpdateChecksum();

			if (!(m_Flag & shade::File::SkipSize))
				UpdateSize();
		}

		m_File.close();
	}
}

std::size_t shade::File::_GetSize()
{
	std::size_t current = m_Stream.tellp();
	m_Stream.seekp(0, std::ios_base::end);
	std::size_t size = m_Stream.tellp();
	m_Stream.seekp(current);
	return size;
}

std::size_t shade::File::TellPosition()
{
	return m_ContentPosition + m_Stream.tellp() ;
}

std::stringstream& shade::File::GetStream()
{
	return m_Stream;
}

shade::File::version_t shade::File::VERSION(version_t major, version_t minor, version_t patch)
{
	return (static_cast<version_t>(major) << 10u) | (static_cast<version_t>(minor) << 4u) | static_cast<version_t>(patch);
}

std::unordered_map<std::string, std::vector<std::string>> shade::File::FindFilesWithExtension(const std::filesystem::path& directory, const std::vector<std::string>& extensions)
{
	std::unordered_map<std::string, std::vector<std::string>> result;

	for (const auto& entry : std::filesystem::directory_iterator(directory)) 
	{
		if (std::filesystem::is_directory(entry.status())) 
		{
			// Recursively scan subdirectories
			auto subdirectoryFiles = FindFilesWithExtension(entry.path(), extensions);
			// Merge subdirectory results into the main result
			for (const auto& [ext, paths] : subdirectoryFiles) 
			{
				result[ext].insert(result[ext].end(), paths.begin(), paths.end());
			}
		}
		else if (std::filesystem::is_regular_file(entry.status())) 
		{
			// Check if the file has one of the specified extensions
			for (const auto& ext : extensions) 
			{
				if (entry.path().extension() == ext) 
				{
					// Add the path to the result map
					result[ext].push_back(entry.path().generic_string());
				}
			}
		}
	}

	return result;
}

std::unordered_map<std::string, std::vector<std::string>> shade::File::FindFilesWithExtensionExclude(const std::filesystem::path& directory, const std::vector<std::string>& excludeExtensions) 
{
	std::unordered_map<std::string, std::vector<std::string>> result;

	for (const auto& entry : std::filesystem::directory_iterator(directory)) 
	{
		if (std::filesystem::is_directory(entry.status())) 
		{
			// Recursively scan subdirectories
			auto subdirectoryFiles = FindFilesWithExtensionExclude(entry.path(), excludeExtensions);
			// Merge subdirectory results into the main result
			for (const auto& [ext, paths] : subdirectoryFiles) 
			{
				result[ext].insert(result[ext].end(), paths.begin(), paths.end());
			}
		}
		else if (std::filesystem::is_regular_file(entry.status())) 
		{
			// Check if the file has one of the specified extensions
			bool excludeFile = false;
			for (const auto& excludeExt : excludeExtensions) 
			{
				if (entry.path().extension() == excludeExt) 
				{
					excludeFile = true;
					break;
				}
			}
			if (!excludeFile) 
			{
				// Add the path to the result map
				result[entry.path().extension().string()].push_back(entry.path().generic_string());
			}
		}
	}

	return result;
}

void shade::File::PackFiles(const shade::File::Specification& specification)
{
	File metaFile(specification.MetaFilePath, Out, "@s_m_file", VERSION(0, 0, 1));

	if (metaFile.IsOpen())
	{
		std::unordered_map<std::string, File> packetFiles;

		for (const auto& [ext, from] : specification.FormatPath)
		{
			auto& packet = packetFiles[ext];

			if (!packet.IsOpen())
				packet.OpenEngineFile(specification.FormatPacketPath.at(ext), Out | SkipSize | SkipChecksum, "@s_packet", VERSION(0, 0, 1));

			for (const auto& currentPath : from)
			{
				// Open file searched file
				std::uint32_t position = packet.TellPosition();
				std::fstream file(currentPath, std::ios::in | std::ios::binary);

				if (file.is_open())
				{
					// Write into packed file
					packet.GetStream() << file.rdbuf();
					// Write meta data into meta file 
					metaFile.Write(currentPath); metaFile.Write(specification.FormatPacketPath.at(ext)); metaFile.Write(position);
				}
				else
				{
					SHADE_CORE_WARNING("Couldn't locate the file during packet serialization, the file doesn't exist, path = {}", currentPath);
				}
			}
		}
	}
}

void shade::File::InitializeMetaFile(const std::string& filepath)
{
	File metaFile(filepath, In, "@s_m_file", VERSION(0, 0, 1));

	if (metaFile.IsOpen())
	{
		m_PathMap.clear();

		while (!metaFile.Eof())
		{
			std::string singlePath, packetPath;
			std::uint32_t position;

			metaFile.Read(singlePath); metaFile.Read(packetPath); metaFile.Read(position);

			m_PathMap[singlePath] = { packetPath , position };
		}

		metaFile.CloseFile();
	}
}

shade::File::Header shade::File::GetHeader() const
{
	return m_Header;
}

void shade::File::ReadHeader(std::istream& stream, version_t version, const magic_t& magic, FileFlag flag, bool skipContent)
{

	if (!(flag & shade::File::SkipMagic))
	{
		Serializer::Deserialize(stream, m_Header.Magic);

		if (m_Header.Magic != magic)
			throw std::runtime_error(std::format("Wrong magic value : {} in : {}", m_Header.Magic, m_Path));
	}

	m_VersionPosition = m_File.tellp();

	if (!(flag & shade::File::SkipVersion))
	{
		Serializer::Deserialize(stream, m_Header.Version);

		if (m_Header.Version != version)
			throw std::runtime_error(std::format("Wrong version value : {} in : {}", m_Header.Version, m_Path));
	}
	
	m_SizePosition = m_File.tellp();

	if (!(flag & shade::File::SkipSize))
	{
		Serializer::Deserialize(stream, m_Header.Size);

		m_CheckSumPosition = m_File.tellp();

		if (!(flag & shade::File::SkipChecksum))
			Serializer::Deserialize(stream, m_Header.CheckSum);

		m_ContentPosition = m_File.tellg();

		if (!skipContent)
		{
			std::string buffer(m_Header.Size, ' ');
			m_File.seekp(m_ContentPosition);
			m_File.read(buffer.data(), m_Header.Size);
			m_Stream << buffer;
			m_Stream.seekp(0, std::ios::beg);

			if (!(flag & shade::File::SkipChecksum))
			{
				if (m_Header.CheckSum != GenerateCheckSum<checksum_t>(buffer))
					throw std::runtime_error(std::format("Wrong checksum value : {} in : {}", m_Header.CheckSum, m_Path));
			}
		}
	}
	else
	{

		if (!skipContent)
		{
			m_CheckSumPosition = m_File.tellp();

			if (!(flag & shade::File::SkipChecksum))
				Serializer::Deserialize(stream, m_Header.CheckSum);

			m_ContentPosition = m_File.tellg();
			m_Stream << m_File.rdbuf();
			m_Stream.seekp(0, std::ios::beg);

			if (!(flag & shade::File::SkipChecksum))
			{
				if (m_Header.CheckSum != GenerateCheckSum<checksum_t>(m_Stream))
					throw std::runtime_error(std::format("Wrong checksum value : {} in : {}", m_Header.CheckSum, m_Path));
			}
		}
	}

	if (!stream.good())
		throw std::runtime_error("Failed to read file header");
}

void shade::File::WriteHeader(std::ostream& stream, version_t version, const magic_t& magic, FileFlag flag)
{
	if (!(flag & shade::File::SkipMagic))
		Serializer::Serialize(stream, magic);

	m_VersionPosition = stream.tellp();

	if (!(flag & shade::File::SkipVersion))
		Serializer::Serialize(stream, version);

	m_SizePosition = stream.tellp();

	if (!(flag & shade::File::SkipSize))
		Serializer::Serialize(stream, std::uint32_t(0u));

	m_CheckSumPosition = stream.tellp();

	if (!(flag & shade::File::SkipChecksum))
		Serializer::Serialize(stream, std::uint32_t(0u));

	m_ContentPosition = stream.tellp();
}

void shade::File::UpdateSize()
{
	content_size_t size = m_Stream.str().size();
	m_File.seekp(m_SizePosition);
	Serializer::Serialize(m_File, size);
}

void shade::File::UpdateChecksum()
{
	checksum_t checksum = GenerateCheckSum<checksum_t>(m_Stream);
	m_File.seekp(m_CheckSumPosition);
	Serializer::Serialize(m_File, checksum);
}

shade::File::checksum_t shade::File::GetChecksum()
{
	return GenerateCheckSum<checksum_t>(m_Stream);
}

bool shade::IsAllowedCharacter(const char c)
{
	const std::string unsupportedCharacters = "\\/:*?\"<>|";
	return std::find(unsupportedCharacters.begin(), unsupportedCharacters.end(), c) == unsupportedCharacters.end();
}

std::string shade::RemoveNotAllowedCharacters(const std::string& input)
{
	std::string result;
	std::copy_if(input.begin(), input.end(), std::back_inserter(result), IsAllowedCharacter);
	return result;
}