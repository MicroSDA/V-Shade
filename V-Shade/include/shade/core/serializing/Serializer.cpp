#include "shade_pch.h"
#include "Serializer.h"

shade::File::File(const std::string& filePath, version_t version, const magic_t& magic, Flag flag) 
{
	OpenEngineFile(filePath, version, magic, flag);
}

shade::File::~File()
{
	CloseFile();
}

bool shade::File::OpenEngineFile(const std::string& filePath, version_t version, const magic_t& magic, Flag flag)
{
	m_Flag = flag;	m_Path = filePath; 

	try
	{
		switch (m_Flag)
		{
			case Flag::ReadFile:
			{
				m_File.open(filePath, std::ios::in | std::ios::binary);
				if (!m_File.is_open()) throw std::runtime_error(std::format("Failed to open file, path = {}", filePath));

				ReadHeader(m_File, version, magic); return true;
			}
			case Flag::WriteFile:
			{
				m_File.open(filePath, std::ios::out | std::ios::binary);
				if (!m_File.is_open()) throw std::runtime_error(std::format("Failed to open file, path = {}", filePath));

				WriteHeader(m_File, version, magic); return true;
			}
		}
	}
	catch (std::exception& e)
	{
		SHADE_CORE_ERROR("Exception: {}", e.what()); m_File.close(); return false;
	}
}

bool shade::File::OpenFile(const std::string& filePath)
{
	// TDOD: 
	return false;
}

bool shade::File::IsOpen() const
{
	return m_File.is_open();
}

void shade::File::CloseFile()
{
	if (m_File.is_open())
	{
		if (m_Flag == Flag::WriteFile)
			UpdateChecksum();

		m_File.close();
	}
}

std::size_t shade::File::_GetSize()
{
	std::streampos current = m_Stream.tellp();
	m_Stream.seekg(0, std::ios_base::end);
	std::streampos size = m_Stream.tellp();
	m_Stream.seekg(current);
	return size;
}

shade::File::version_t shade::File::VERSION(version_t major, version_t minor, version_t patch)
{
	return (static_cast<version_t>(major) << 10u) | (static_cast<version_t>(minor) << 4u) | static_cast<version_t>(patch);
}

void testfn(std::ios::event ev, std::ios_base& stream, int index)
{
	switch (ev)
	{
	case stream.copyfmt_event:
		std::cout << "copyfmt_event\n"; break;
	case stream.imbue_event:
		std::cout << "imbue_event\n"; break;
	case stream.erase_event:
		std::cout << "erase_event\n"; break;
	}
}

void shade::File::ReadHeader(std::istream& stream, version_t version, const magic_t& magic)
{
	Serializer::Deserialize(stream, m_Header.Magic);
	Serializer::Deserialize(stream, m_Header.Version);
	Serializer::Deserialize(stream, m_Header.Size);
	Serializer::Deserialize(stream, m_Header.CheckSum);

	m_ContentPosition = m_File.tellg();

	std::string buffer(m_Header.Size, '\0');
	m_File.seekp(m_ContentPosition);
	m_File.read(buffer.data(), m_Header.Size);
	m_Stream << buffer;

	if (m_Header.Magic != magic)
		throw std::runtime_error(std::format("Wrong magic value : {}", m_Header.Magic));

	if (m_Header.Version != version)
		throw std::runtime_error(std::format("Wrong version value : {}", m_Header.Version));

	if (m_Header.CheckSum != GenerateCheckSum<checksum_t>(buffer))
		throw std::runtime_error(std::format("Wrong checksum value : {}", m_Header.CheckSum));

	if (!stream.good())
		throw std::runtime_error("Failed to read file header");
}

void shade::File::WriteHeader(std::ostream& stream, version_t version, const magic_t& magic)
{
	Serializer::Serialize(stream, magic);
	Serializer::Serialize(stream, version);
	// Size
	Serializer::Serialize(stream, std::uint32_t(0u));
	// Version
	Serializer::Serialize(stream, std::uint32_t(0u));

	m_ContentPosition = stream.tellp();
}

void shade::File::UpdateChecksum()
{
	content_size_t size = m_Stream.str().size();
	checksum_t checksum = GenerateCheckSum<checksum_t>(m_Stream);
	
	m_File << m_Stream.rdbuf();

	m_File.seekp(static_cast<checksum_t>(m_ContentPosition - sizeof(checksum_t) - sizeof(content_size_t)));

	Serializer::Serialize(m_File, size);
	Serializer::Serialize(m_File, checksum);
}

shade::File::checksum_t shade::File::GetChecksum()
{
	return GenerateCheckSum<checksum_t>(m_Stream);
}
