#pragma once
#include <shade/core/serializing/Serializer.h>

namespace shade
{
	namespace file
	{
		static const unsigned int CRC32Table[256] =
		{
			0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL, 0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L,
			0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL, 0x97D2D988L, 0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L,
			0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL, 0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L,
			0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL, 0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L,
			0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L, 0xA2677172L, 0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
			0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L, 0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L,
			0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L, 0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L, 0xB8BDA50FL,
			0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L, 0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL,
			0x76DC4190L, 0x01DB7106L, 0x98D220BCL, 0xEFD5102AL, 0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L,
			0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L, 0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
			0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL, 0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
			0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL, 0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L,
			0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L, 0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL,
			0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L, 0x44042D73L, 0x33031DE5L, 0xAA0A4C8FL, 0xDD0D7CC9L,
			0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L, 0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
			0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L, 0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL,
			0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL, 0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L,
			0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L, 0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L,
			0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL, 0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L,
			0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL, 0x67DD4ACCL, 0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
			0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L, 0xD1BB67F1L, 0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL,
			0xD80D2BDA, 0xAF0A1B4CL, 0x36034AF6, 0x41047A60L, 0xDF60EFC3L, 0xA867DF55L, 0x316E8EEDL, 0x4669BE79L,
			0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L, 0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL,
			0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L, 0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL,
			0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL, 0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
			0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L, 0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L,
			0x86D3D2D4L, 0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL, 0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L,
			0x88085AE6L, 0xFF0F6A70L, 0x66063BDEL, 0x11010B5CL, 0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L,
			0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L, 0xA7672661L, 0xD06016F7L, 0x4969474DL, 0x3E6E77DBL,
			0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L, 0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
			0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L, 0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL,
			0xB3667A2EL, 0xC4614ABCL, 0x5D681B02L, 0x2A6F2B94L, 0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL
		};

		using version_t = std::uint16_t; // Type for version
		using magic_t = std::string;     // Type for magic string (format identifier)
		using flag_t = int;              // Type for file operation flags

		namespace utils
		{
			/**
			 * @brief Generates a CRC32 checksum for the data in the given stringstream.
			 * @tparam Bitdepth The type of the CRC (e.g., uint32_t or uint64_t).
			 * @param buffer The input stringstream buffer to compute the checksum for.
			 * @param initialCRC The initial CRC value.
			 * @return The computed CRC32 checksum.
			 */
			template<typename Bitdepth>
			SHADE_INLINE Bitdepth GenerateCheckSumCRC32(std::stringstream& buffer, Bitdepth initialCRC = 0xFFFFFFFF)
			{
				Bitdepth crc = initialCRC;
				std::streampos currentPos = buffer.tellg(); // Save current stream position
				buffer.seekg(0, std::ios::beg); // Move to the beginning of the stream

				char byte;
				while (buffer.get(byte)) // Read each byte
					crc = (crc >> 8) ^ CRC32Table[(crc ^ static_cast<std::uint8_t>(byte)) & 0xFF];

				buffer.clear(); // Clear any flags on the stream
				buffer.seekg(currentPos); // Restore stream position

				return crc ^ 0xFFFFFFFF; // Return the final CRC32 value
			}

			/**
			 * @brief Generates a CRC32 checksum for the data in the given string.
			 * @tparam Bitdepth The type of the CRC (e.g., uint32_t or uint64_t).
			 * @param data The input string data to compute the checksum for.
			 * @param initialCRC The initial CRC value.
			 * @return The computed CRC32 checksum.
			 */
			template<typename Bitdepth>
			SHADE_INLINE Bitdepth GenerateCheckSumCRC32(const std::string& data, Bitdepth initialCRC = 0xFFFFFFFF)
			{
				Bitdepth crc = initialCRC;

				for (char byte : data) // Iterate over each byte in the string
					crc = (crc >> 8) ^ CRC32Table[(crc ^ static_cast<std::uint8_t>(byte)) & 0xFF];

				return crc ^ 0xFFFFFFFF; // Return the final CRC32 value
			}

			/**
			 * @brief Generates a hash value from a stringstream using the standard hash function.
			 * @tparam Bitdepth The type of the hash (e.g., uint32_t or uint64_t).
			 * @param stream The input stringstream to compute the hash for.
			 * @return The computed hash value.
			 */
			template <typename Bitdepth, typename = std::enable_if_t<std::is_same<Bitdepth, std::uint32_t>::value || std::is_same<Bitdepth, std::uint64_t>::value>>
			SHADE_INLINE static Bitdepth GenerateCheckSumHash(const std::stringstream& stream)
			{
				return static_cast<Bitdepth>(std::hash<std::string>{}(stream.str()));
			}

			/**
			 * @brief Generates a hash value from a string using the standard hash function.
			 * @tparam Bitdepth The type of the hash (e.g., uint32_t or uint64_t).
			 * @param stream The input string to compute the hash for.
			 * @return The computed hash value.
			 */
			template <typename Bitdepth, typename = std::enable_if_t<std::is_same<Bitdepth, std::uint32_t>::value || std::is_same<Bitdepth, std::uint64_t>::value>>
			SHADE_INLINE static Bitdepth GenerateCheckSumHash(const std::string& stream)
			{
				return static_cast<Bitdepth>(std::hash<std::string>{}(stream));
			}

			/**
			 * @brief Combines major, minor, and patch version numbers into a single version value.
			 * @param major The major version number.
			 * @param minor The minor version number.
			 * @param patch The patch version number.
			 * @return A combined version value.
			 */
			SHADE_INLINE static version_t VERSION(version_t major, version_t minor, version_t patch)
			{
				return (static_cast<version_t>(major) << 10u) | (static_cast<version_t>(minor) << 4u) | static_cast<version_t>(patch);
			}
		}

		// File operation flags
		static constexpr inline flag_t None = 0x00;
		static constexpr inline flag_t In = 0x01;  // Open file for read
		static constexpr inline flag_t Out = 0x02; // Open file for write
		static constexpr inline flag_t SkipMagicCheck = 0x04;
		static constexpr inline flag_t SkipVersionCheck = 0x08;
		static constexpr inline flag_t SkipChecksumCheck = 0x10;
		static constexpr inline flag_t SkipBufferUseIn = 0x20;
		static constexpr inline flag_t SumCRC32 = 0x40;

		/**
		 * @brief Manages file reading, writing, and processing operations.
		 */
		class SHADE_API File
		{
		public:
			using checksum_t = std::uint32_t;  // Type for checksum values
			using content_size_t = checksum_t; // Type for content size

			/**
			 * @brief Represents the file header information.
			 */
			struct Header
			{
				magic_t          Magic;       // Magic string to identify file format
				version_t        Version = 0u; // File version
				content_size_t   ContentSize = 0u; // Size of the file content
				checksum_t       CheckSum = 0u;    // Checksum of the file content
			};

		public:
			/**
			 * @brief Default constructor that initializes internal buffers and file handle.
			 */
			File();

			/**
			 * @brief Destructor that closes the file if it is still open.
			 */
			virtual ~File();

			/**
			 * @brief Constructor that opens a file with the specified path, flags, magic, and version.
			 * @param filePath The path to the file.
			 * @param flags The flags for file operations.
			 * @param magic The magic string for format identification.
			 * @param version The file version.
			 */
			File(const std::string& filePath, flag_t flags, const magic_t& magic = "@", version_t version = version_t(0));

			/**
			 * @brief Constructor that opens a file with an existing file stream.
			 * @param stream A shared pointer to an existing file stream.
			 * @param flags The flags for file operations.
			 * @param magic The magic string for format identification.
			 * @param version The file version.
			 */
			File(std::shared_ptr<std::fstream> stream, flag_t flags, const magic_t& magic = "@", version_t version = version_t(0));

			/**
			 * @brief Opens a file with the specified path, flags, magic, and version.
			 * @param filePath The path to the file.
			 * @param flags The flags for file operations.
			 * @param magic The magic string for format identification.
			 * @param version The file version.
			 * @return True if the file is opened successfully, false otherwise.
			 * @throws std::runtime_error if 'In || Out' flags are not specified.
			 */
			bool OpenFile(const std::string& filePath, flag_t flags, const magic_t& magic = "@", version_t version = version_t(0));

			/**
			 * @brief Opens a file stream from an existing std::shared_ptr<std::fstream>.
			 * @param stream The shared pointer to an existing fstream object.
			 * @param flags The flags indicating the file mode (input/output).
			 * @param magic The magic string for identifying file type. Default is "@".
			 * @param version The file version. Default is 0.
			 * @return True if the file is opened successfully; otherwise, false.
			 * @throws std::runtime_error if the 'In || Out' flags are not specified.
			 */
			bool OpenFile(std::shared_ptr<std::fstream> stream, flag_t flags, const magic_t& magic = "@", version_t version = version_t(0));

			/**
			 * @brief Closes the file and writes any buffered content to disk, updating the size and checksum.
			 */
			void CloseFile();

			/**
			 * @brief Checks if the file is currently open.
			 * @return True if the file is open, otherwise false.
			 */
			bool IsOpen() const;

			/**
			 * @brief Checks if the end of the internal buffer has been reached.
			 * @return True if end of file is reached, otherwise false.
			 */
			SHADE_INLINE bool Eof()
			{
				return (m_InternalBuffer->peek() == EOF);
			}

			/**
			 * @brief Gets a reference to the internal buffer (std::stringstream).
			 * @return A shared pointer to the internal buffer.
			 */
			SHADE_INLINE std::shared_ptr<std::stringstream>& GetInternalBuffer()
			{
				return m_InternalBuffer;
			}

			/**
			 * @brief Returns the current write position in the internal buffer.
			 * @return The current write position.
			 */
			SHADE_INLINE std::size_t TellPosition()
			{
				return m_InternalBuffer->tellp();
			}

			/**
			 * @brief Sets the write/read position in the internal buffer or file.
			 * @param pos The new position.
			 */
			void SetPosition(std::size_t pos);

			/**
			 * @brief Gets the file handle used for direct file I/O.
			 * @return A shared pointer to the file handle (std::fstream).
			 */
			SHADE_INLINE std::shared_ptr<std::fstream>& GetFileHandle()
			{
				return m_FileHandle;
			}

			/**
			 * @brief Gets the size of the internal buffer content.
			 * @return The size of the internal buffer.
			 */
			std::size_t GetSize();

			/**
			 * @brief Gets the file header.
			 * @return The file header.
			 */
			Header GeFileHeader() const
			{
				return m_FileHeader;
			}

			/**
			 * @brief Writes data of type T to the internal buffer.
			 * @param value The value to write.
			 * @tparam T The type of data to write.
			 */
			template<typename T>
			SHADE_INLINE void Write(const T& value)
			{
				assert((m_Flags & Out) && "Cannot write into file, 'Out' flag is not set !");
				serialize::Serializer::Serialize(*m_InternalBuffer, value);
			}

			/**
			 * @brief Reads data of type T from the internal buffer or file.
			 * @param value The value to read into.
			 * @tparam T The type of data to read.
			 */
			template<typename T>
			SHADE_INLINE void Read(T& value)
			{
				assert((m_Flags & In) && "Cannot read from file, 'In' flag is not set !");
				(m_Flags & SkipBufferUseIn) ?
					serialize::Serializer::Deserialize(*m_FileHandle, value) :
					serialize::Serializer::Deserialize(*m_InternalBuffer, value);
			}
			/**
			 * @brief Checks if the file is open by using a conversion operator to bool.
			 * @return True if the file is open, otherwise false.
			 */
			SHADE_INLINE operator bool() const
			{
				return IsOpen();
			}

		private:
			/**
			 * @brief Reads the file header and performs checks (magic, version, checksum).
			 * @throws std::runtime_error if the magic, version, or checksum values are incorrect.
			 */
			void ReadFileHeader();
			/**
			 * @brief Writes the file header including the magic string, version, and placeholders for size and checksum.
			 */

			void WriteFileHeader();

			/**
			 * @brief Updates the checksum value in the file header.
			 */
			void UpdateChecksum();

			/**
			 * @brief Updates the file size in the header after writing.
			 */
			void UpdateSize();
		private:
			// Internal buffer for file operations
			std::shared_ptr<std::stringstream> m_InternalBuffer;

			// Handle to the file stream for direct file I/O
			std::shared_ptr<std::fstream> m_FileHandle;

			// Various positions within the file for metadata
			std::streampos m_VersionPosition, m_SizePosition, m_CheckSumPosition, m_ContentPosition;

			// File metadata and flags
			flag_t m_Flags;
			Header m_FileHeader;
			std::string m_FilePath;
		};

		/**
		 * @brief Manages file operations including loading, packing, and searching for files.
		 *
		 * The `FileManager` class provides functionality for loading files either directly or from packed archives,
		 * finding files with specific extensions in directories, and packing files into archive formats.
		 * It also maintains a mapping of file paths to their locations within packed files.
		 */
		class SHADE_API FileManager
		{
		public:
			/**
			 * @brief Structure to specify the file packing details.
			 *
			 * This structure contains mappings for file formats and their associated paths,
			 * both for where the original files are located and where the packed files are stored.
			 */
			struct PackSpecification
			{
				/**
				 * @brief Mapping from format to file paths for the format.
				 *
				 * This map associates each file format with a list of paths where files of that format are located.
				 */
				std::unordered_map<std::string, std::vector<std::string>> FormatPath;

				/**
				 * @brief Mapping from format to packet paths.
				 *
				 * This map associates each file format with the path to the packet file where files of that format are packed.
				 */
				std::unordered_map<std::string, std::string> FormatPacketPath;
			};

		public:
			/**
			 * @brief Loads a file from disk or from a packed file if it exists.
			 *
			 * Attempts to load a file either directly from the disk or from a packed archive if the file path is
			 * found in the path map. If the file is loaded from a packed archive, it adjusts the file handle to
			 * point to the specific file within the archive.
			 *
			 * @param filePath The path to the file to be loaded.
			 * @param magic The magic number used to identify the file format.
			 *
			 * @return File A `File` object representing the loaded file.
			 *         Returns an invalid `File` object if the file cannot be loaded.
			 */
			static File LoadFile(const std::string& filePath, const magic_t& magic, flag_t flags = None);

			static File SaveFile(const std::string& filePath, const magic_t& magic, flag_t flags = None);

			/**
			 * @brief Finds files with specific extensions in a directory and its subdirectories.
			 *
			 * Searches through the specified directory and its subdirectories to find files with the given extensions.
			 * It returns a map where the key is the file extension and the value is a list of file paths with that extension.
			 *
			 * @param directory The directory to search for files.
			 * @param extensions A vector of file extensions to look for.
			 *
			 * @return std::unordered_map<std::string, std::vector<std::string>> A map where the key is the file extension and the value is a list of file paths with that extension.
			 */
			static std::unordered_map<std::string, std::vector<std::string>> FindFilesWithExtension(const std::filesystem::path& directory, const std::vector<std::string>& extensions);

			/**
			 * @brief Finds files that do not have specified extensions.
			 *
			 * Searches through the specified directory and its subdirectories to find files that do not have the excluded extensions.
			 * It returns a map where the key is the file extension and the value is a list of file paths that do not have the excluded extensions.
			 *
			 * @param directory The directory to search for files.
			 * @param excludeExtensions A vector of file extensions to exclude from the search.
			 *
			 * @return std::unordered_map<std::string, std::vector<std::string>> A map where the key is the file extension and the value is a list of file paths that do not have the excluded extensions.
			 */
			static std::unordered_map<std::string, std::vector<std::string>> FindFilesWithExtensionExclude(const std::filesystem::path& directory, const std::vector<std::string>& excludeExtensions);

			/**
			 * @brief Initializes the file manager by scanning for packets and building the path map.
			 *
			 * Scans the specified directory for packet files (.vspack), opens each packet file, and builds a map
			 * that maps individual file paths to their locations within the packet files.
			 *
			 * @param directory The directory to start scanning for packets. Defaults to the current directory.
			 */
			static void Initialize(const std::string& directory = "./");

			/**
			 * @brief Packs files into packets based on the provided specification.
			 *
			 * Creates or opens packet files for each format specified in the PackSpecification. Files are read from their
			 * source paths and written into the appropriate packet files. The mapping of file paths to positions within the
			 * packet is recorded in the packet files.
			 *
			 * @param specification The specification detailing which files to pack and where to store them.
			 */
			static void PackFiles(const PackSpecification& specification);

		private:
			/**
			 * @brief Static map storing the mapping of file paths to packet paths and positions within packets.
			 *
			 * This map is used to keep track of the locations of files within packet files.
			 *
			 * Key: File path
			 * Value: Pair containing packet path and position within the packet
			 */
			static inline std::unordered_map<std::string, std::pair<std::string, std::uint32_t>> m_PathMap;
		};
	}
}
