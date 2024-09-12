#include "shade_pch.h"
#include "File.h"

shade::file::File::File() : m_InternalBuffer(std::make_shared<std::stringstream>()), m_FileHandle(std::make_shared<std::fstream>())
{
}

shade::file::File::~File()
{
	// Check if no other references are holding the file handle
	if (m_FileHandle.use_count() < 2)
		CloseFile(); // If so, close the file
}

shade::file::File::File(const std::string& filePath, flag_t flags, const magic_t& magic, version_t version) :
	m_InternalBuffer(std::make_shared<std::stringstream>()), m_FileHandle(std::make_shared<std::fstream>())
{
	OpenFile(filePath, flags, magic, version);
}

shade::file::File::File(std::shared_ptr<std::fstream> stream, flag_t flags, const magic_t& magic, version_t version) :
	m_InternalBuffer(std::make_shared<std::stringstream>())
{
	OpenFile(stream, flags, magic, version);
}

bool shade::file::File::OpenFile(const std::string& filePath, flag_t flags, const magic_t& magic, version_t version)
{
	m_FileHandle->open(filePath, flags & In | flags & Out | std::ios::binary);
	return OpenFile(m_FileHandle, flags, magic, version);
}

bool shade::file::File::OpenFile(std::shared_ptr<std::fstream> stream, flag_t flags, const magic_t& magic, version_t version)
{
	m_FileHandle = stream, m_Flags = flags, m_FileHeader.Magic = magic, m_FileHeader.Version = version;

	// Checks if the flags include 'In' or 'Out' for input or output operations
	if (m_Flags & In || m_Flags & Out)
	{
		if (!*m_FileHandle) return false; // Return false if the file stream is invalid

		if (m_Flags & In) // If input flag is set, read the file header
		{
			ReadFileHeader(); // Read the header of the file
			return true; // Successfully opened and read header
		}
		return true; // Successfully opened the file for output
	}
	else
	{
		// Throw an error if neither input nor output flags are set
		throw std::runtime_error(std::format("Failed to open file, 'In || Out' flags are not specified, path = {}", m_FilePath));
	}
}

void shade::file::File::ReadFileHeader()
{
	magic_t magic;
	// Deserialize (read) the magic string from the file
	serialize::Serializer::Deserialize(*m_FileHandle, magic);

	if (!(m_Flags & SkipMagicCheck)) // If magic check is not skipped
	{
		if (m_FileHeader.Magic != magic) // Compare with expected magic string
			throw std::runtime_error(std::format("Wrong magic value: {} in: {}", magic, m_FilePath.c_str())); // Throw error if mismatch
	}

	m_VersionPosition = m_FileHandle->tellg(); // Save the current position for the version
	version_t version;
	// Deserialize (read) the version from the file
	serialize::Serializer::Deserialize(*m_FileHandle, version);

	if (!(m_Flags & SkipVersionCheck)) // If version check is not skipped
	{
		if (m_FileHeader.Version != version) // Compare with expected version
			throw std::runtime_error(std::format("Wrong version value: {} in: {}", version, m_FilePath.c_str())); // Throw error if mismatch
	}

	m_SizePosition = m_FileHandle->tellg(); // Save the current position for the content size
	// Deserialize (read) the content size from the file
	serialize::Serializer::Deserialize(*m_FileHandle, m_FileHeader.ContentSize);

	m_CheckSumPosition = m_FileHandle->tellg(); // Save the current position for the checksum
	checksum_t checksum;
	// Deserialize (read) the checksum from the file
	serialize::Serializer::Deserialize(*m_FileHandle, checksum);

	m_ContentPosition = m_FileHandle->tellg(); // Save the current position for the file content

	if (!(m_Flags & SkipBufferUseIn)) // If buffer use for input is not skipped
	{
		// Create a buffer string of the size specified in the header
		std::string buffer(m_FileHeader.ContentSize, '\0');
		m_FileHandle->read(buffer.data(), m_FileHeader.ContentSize); // Read the file content into the buffer

		if (!(m_Flags & SkipChecksumCheck)) // If checksum check is not skipped
		{
			// Generate the checksum for the buffer using CRC32 or Hash
			m_FileHeader.CheckSum = (m_Flags & SumCRC32) ?
				utils::GenerateCheckSumCRC32<checksum_t>(buffer) :
				utils::GenerateCheckSumHash<checksum_t>(buffer);

			if (m_FileHeader.CheckSum != checksum) // Compare with expected checksum
				throw std::runtime_error(std::format("Wrong checksum value: {} in: {}", checksum, m_FilePath)); // Throw error if mismatch
		}

		// Move the buffer content into the internal buffer
		m_InternalBuffer->str(std::move(buffer));
	}

	// Check if the internal buffer is in a good state
	if (!m_InternalBuffer->good())
		throw std::runtime_error(std::format("Failed to read file header in: {}", m_FilePath)); // Throw error if the buffer is bad
}

void shade::file::File::WriteFileHeader()
{
	m_FileHandle->seekp(0, std::ios::beg); // Set the file pointer to the beginning
	// Serialize (write) the magic string to the file
	serialize::Serializer::Serialize(*m_FileHandle, m_FileHeader.Magic);

	m_VersionPosition = m_FileHandle->tellp(); // Save the current position for the version
	// Serialize (write) the version to the file
	serialize::Serializer::Serialize(*m_FileHandle, m_FileHeader.Version);

	m_SizePosition = m_FileHandle->tellp(); // Save the current position for the content size
	// Serialize (write) a placeholder for content size
	serialize::Serializer::Serialize(*m_FileHandle, checksum_t(0));

	m_CheckSumPosition = m_FileHandle->tellp(); // Save the current position for the checksum
	// Serialize (write) a placeholder for checksum
	serialize::Serializer::Serialize(*m_FileHandle, checksum_t(0));

	m_ContentPosition = m_FileHandle->tellp(); // Save the current position for the file content
}

void shade::file::File::CloseFile()
{
	if (m_FileHandle->is_open())
	{
		if (m_Flags & Out) // If the file is opened for output
		{
			WriteFileHeader(); // Write the header to the file
			*m_FileHandle << m_InternalBuffer->rdbuf(); // Write the internal buffer to the file
			UpdateSize(); // Update the size in the file header

			if (!(m_Flags & SkipChecksumCheck)) // If checksum check is not skipped
				UpdateChecksum(); // Update the checksum in the file header
		}
		m_FileHandle->close(); // Close the file handle
	}
}

void shade::file::File::UpdateChecksum()
{
	// Generate the checksum from the internal buffer using CRC32 or Hash
	const checksum_t checksum = (m_Flags & SumCRC32) ? utils::GenerateCheckSumCRC32<checksum_t>(*m_InternalBuffer) : utils::GenerateCheckSumHash<checksum_t>(*m_InternalBuffer);

	m_FileHandle->seekp(m_CheckSumPosition); // Move the file pointer to the checksum position
	// Serialize (write) the new checksum to the file
	serialize::Serializer::Serialize(*m_FileHandle, checksum);
}

void shade::file::File::UpdateSize()
{
	const content_size_t size = GetSize(); // Get the current size of the content
	m_FileHandle->seekp(m_SizePosition); // Move the file pointer to the size position
	// Serialize (write) the new size to the file
	serialize::Serializer::Serialize(*m_FileHandle, size);
}

bool shade::file::File::IsOpen() const
{
	return m_FileHandle->is_open();
}

void shade::file::File::SetPosition(std::size_t pos)
{
	// If buffer use is skipped and file is open for input
	(m_Flags & SkipBufferUseIn && m_Flags & In) ? m_FileHandle->seekp(pos + m_ContentPosition, std::ios::beg) /* Move the file pointer directly */ : m_InternalBuffer->seekp(pos, std::ios::beg); // Move the internal buffer pointer
}

std::size_t shade::file::File::GetSize()
{
	m_InternalBuffer->seekg(0, std::ios::end);		// Move to the end of the internal buffer
	std::size_t size = m_InternalBuffer->tellg();	// Get the size from the current position
	m_InternalBuffer->seekg(0, std::ios::beg);		// Move back to the beginning of the internal buffer

	return size;
}

shade::file::File shade::file::FileManager::LoadFile(const std::string& filePath, const magic_t& magic, flag_t flags)
{
	//Check if the file exists on disk
	if (std::filesystem::exists(filePath))
	{
		// If the file exists, create and return a File object with specified flags
		return File(filePath, file::In | flags, magic, utils::VERSION(0, 0, 1));
	}
	else
	{
		// Check if the file path is in the packed files map
		const auto packed = m_PathMap.find(filePath);

		// If the file path is found in the map
		if (packed != m_PathMap.end())
		{
			// Open the packed file for reading with appropriate flags
			if (File packedFile = File(packed->second.first, file::In | file::SkipChecksumCheck | file::SkipBufferUseIn, "@vspack", utils::VERSION(0, 0, 1)))
			{
				// Adjust the file handle position to locate the specific file in the packed file
				packedFile.GetFileHandle()->seekp(packed->second.second + packedFile.GetFileHandle()->tellg());
				// Return a new File object for the specific file within the packed file
				return File(packedFile.GetFileHandle(), file::In, magic, utils::VERSION(0, 0, 1));
			}
		}
	}

	// Return an invalid File object if the file cannot be loaded
	return File(); // Indicates failure to load the file
}

shade::file::File shade::file::FileManager::SaveFile(const std::string& filePath, const magic_t& magic, flag_t flags)
{
	return File(filePath, file::Out | flags, magic, utils::VERSION(0, 0, 1));
}

std::unordered_map<std::string, std::vector<std::string>> shade::file::FileManager::FindFilesWithExtension(const std::filesystem::path& directory, const std::vector<std::string>& extensions)
{
	// Result map to store files by extension
	std::unordered_map<std::string, std::vector<std::string>> result;

	// Iterate over each entry in the directory
	for (const auto& entry : std::filesystem::directory_iterator(directory))
	{
		// If the entry is a directory, recursively search its contents
		if (std::filesystem::is_directory(entry.status()))
		{
			auto subdirectoryFiles = FindFilesWithExtension(entry.path(), extensions);
			// Merge subdirectory results into the result map
			for (const auto& [ext, paths] : subdirectoryFiles)
			{
				result[ext].insert(result[ext].end(), paths.begin(), paths.end());
			}
		}
		// If the entry is a regular file, check its extension
		else if (std::filesystem::is_regular_file(entry.status()))
		{
			for (const auto& ext : extensions)
			{
				if (entry.path().extension() == ext)
				{
					// Add the file path to the result map
					result[ext].push_back(entry.path().generic_string());
				}
			}
		}
	}

	return result;
}

std::unordered_map<std::string, std::vector<std::string>> shade::file::FileManager::FindFilesWithExtensionExclude(const std::filesystem::path& directory, const std::vector<std::string>& excludeExtensions)
{
	std::unordered_map<std::string, std::vector<std::string>> result; // Result map to store files by extension

	// Iterate over each entry in the directory
	for (const auto& entry : std::filesystem::directory_iterator(directory))
	{
		// If the entry is a directory, recursively search its contents
		if (std::filesystem::is_directory(entry.status()))
		{
			auto subdirectoryFiles = FindFilesWithExtensionExclude(entry.path(), excludeExtensions);
			// Merge subdirectory results into the result map
			for (const auto& [ext, paths] : subdirectoryFiles)
			{
				result[ext].insert(result[ext].end(), paths.begin(), paths.end());
			}
		}
		// If the entry is a regular file, check its extension
		else if (std::filesystem::is_regular_file(entry.status()))
		{
			bool excludeFile = false;
			// Check if the file has one of the excluded extensions
			for (const auto& excludeExt : excludeExtensions)
			{
				if (entry.path().extension() == excludeExt)
				{
					excludeFile = true;
					break; // Exit loop if file should be excluded
				}
			}
			if (!excludeFile)
			{
				// Add the file path to the result map
				result[entry.path().extension().string()].push_back(entry.path().generic_string());
			}
		}
	}

	return result; // Return the map of files found
}

void shade::file::FileManager::Initialize(const std::string& directory)
{
	// Find all files with .vspack extension in the given directory
	const auto files = FindFilesWithExtension(directory, { ".vspack" });

	// Open each file to build the path map
	for (const auto& [ext, paths] : files)
	{
		for (const auto& path : paths)
		{
			if (File file = File(path, In | SkipChecksumCheck | SkipBufferUseIn, "@vspack", utils::VERSION(0, 0, 1)))
			{
				// Read the position of the file count
				std::uint32_t pos = 0; file.Read(pos); file.SetPosition(pos);

				// Read the count of files packed in this file
				std::uint32_t filesCount = 0; file.Read(filesCount);

				// Read each file path and position and store them in the path map
				for (std::uint32_t i = 0; i < filesCount; ++i)
				{
					std::string filePath; file.Read(filePath); std::uint32_t position; file.Read(position);
					m_PathMap.emplace(std::piecewise_construct, std::forward_as_tuple(filePath), std::forward_as_tuple(path, position));
				}
			}
		}
	}
}

void shade::file::FileManager::PackFiles(const PackSpecification& specification)
{
	std::unordered_map<std::string, File> packetFiles; // Map to keep track of packet files

	// Iterate over each format and its associated paths
	for (const auto& [ext, from] : specification.FormatPath)
	{
		// Retrieve or create a packet file for the current extension
		auto& packetFile = packetFiles[ext];

		if (!packetFile)
			packetFile.OpenFile(specification.FormatPacketPath.at(ext), Out | SkipChecksumCheck, "@vspack", utils::VERSION(0, 0, 1));

		if (packetFile)
		{
			// Write a placeholder for the position of the path information
			std::uint32_t pathPosPos = packetFile.TellPosition(); packetFile.Write(std::uint32_t(0u));

			std::unordered_map<std::string, std::uint32_t> PathPos;

			// Write each file to the packet and record its position
			for (const auto& currentPath : from)
			{
				PathPos[currentPath] = packetFile.TellPosition();

				if (std::fstream file = std::fstream(currentPath, std::ios::in | std::ios::binary))
				{
					*packetFile.GetInternalBuffer() << file.rdbuf();
				}
			}

			// Record the current position in the packet file
			std::uint32_t curPos = packetFile.TellPosition();

			// Write the number of files and their positions
			packetFile.Write(std::uint32_t(PathPos.size()));

			for (const auto& [path, pos] : PathPos)
			{
				packetFile.Write(path); packetFile.Write(pos);
			}

			// Update the placeholder with the actual position
			packetFile.SetPosition(pathPosPos); packetFile.Write(curPos);
		}
	}
}
