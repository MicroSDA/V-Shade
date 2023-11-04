#include "shade_pch.h"
#include "Shader.h"
#include <shade/platforms/render/vulkan/VulkanShader.h>
#include <shade/core/render/RenderAPI.h>

namespace utils
{
    static void CreateChacheDirectory(const std::string& path)
    {
        if (!std::filesystem::exists(path))
            std::filesystem::create_directories(path);

    }
}

shade::Shader::Shader(const std::string& filePath)
{
    std::string api(RenderAPI::GetCurrentAPIAsString());
    utils::CreateChacheDirectory(GetShaderCacheDirectory());

    m_FilePath = filePath;
    m_FileName = std::filesystem::path(filePath).stem().string();
    m_Directory = std::filesystem::path(filePath).remove_filename().string();

    std::string source = ReadFile(m_FilePath);
    m_SourceCode = PreProcess(source, std::filesystem::path(filePath).filename().stem().string());
}

shade::SharedPointer<shade::Shader> shade::Shader::Create(const std::string& filePath)
{
    switch (RenderAPI::GetCurrentAPI())
    {
        case RenderAPI::API::Vulkan: return SharedPointer<VulkanShader>::Create(filePath);
        default:SHADE_CORE_ERROR("Only Vulkan api is supported!"); return nullptr;
    }
}

const std::string& shade::Shader::GetFilePath() const
{
    return m_FilePath;
}

const std::string& shade::Shader::GetFileDirectory() const
{
    return m_Directory;
}

const std::string& shade::Shader::GetFileName() const
{
    return m_FileName;
}

void shade::Shader::SetFilePath(const std::string& filePath)
{
    m_FilePath = filePath;
}

std::string shade::Shader::GetShaderCacheDirectory()
{
    std::string api(RenderAPI::GetCurrentAPIAsString());
    return "resources/" + api + "_cache/";
}

std::uint32_t shade::Shader::GetDataTypeSize(const DataType& type)
{
    switch (type)
    {
        case DataType::Float:    return 4;
        case DataType::Float2:   return 4 * 2;
        case DataType::Float3:   return 4 * 3;
        case DataType::Float4:   return 4 * 4;
        case DataType::Mat3:     return 4 * 3 * 3;
        case DataType::Mat4:     return 4 * 4 * 4;
        case DataType::Int:      return 4;
        case DataType::Int2:     return 4 * 2;
        case DataType::Int3:     return 4 * 3;
        case DataType::Int4:     return 4 * 4;
        case DataType::Bool:     return 1;
        default: return 0;
    }
}
std::string shade::Shader::ReadFile(const std::string& filePath) 
{
    std::ifstream file(filePath, std::ios::binary);
    if (file.is_open())
    {
        std::string source;
        file.seekg(0, std::ios::end);
        std::size_t fileSize = file.tellg();
        if (fileSize)
        {
            file.seekg(0, std::ios::beg);
            source.resize(fileSize, ' ');
            file.read(source.data(), fileSize);
        }

        file.close();
        return source;
    }
    else
    {
        SHADE_CORE_ERROR("Failed to open shader file '{0}'", filePath);
    }
}

std::unordered_map<shade::Shader::Type, std::string> shade::Shader::PreProcess(std::string& source, const std::string& origin)
{
    std::unordered_map<Shader::Type, std::string> shaderSourceCode;
    // String used to identify the start of the shader type in the source code
    const std::string shaderTypeToken = "#pragma:";
    const std::string shaderVersionToken = "#version";
    // Find the first occurrence of the shaderTypeToken in the source code
    std::size_t position = 0;
    std::size_t start = 0;
    // Keep searching the source code until all occurrences of the shaderTypeToken have been found
    while (position != std::string::npos)
    {
        start = position;
        position = source.find(shaderVersionToken, position);
        // Get the index of the first end-of-line character after the shaderTypeToken
        std::size_t eol = source.find_first_of("\r\n", position);
        // If the variable "eol" doesn't have a value that represents the end of the string,
        if (eol != std::string::npos)
        {
            std::size_t stagePosition = source.find(shaderTypeToken, eol);
            // End of stage token !
            eol = source.find_first_of("\r\n", stagePosition);

            std::string stage = source.substr(stagePosition + shaderTypeToken.size(), eol - stagePosition - shaderTypeToken.size());
            stage.erase(std::remove_if(stage.begin(), stage.end(), ::isspace), stage.end());

            auto shaderStage = GetTypeFromString(stage);
            if (shaderStage != Shader::Type::Undefined)
            {
                // Need to find end of current shader !
                eol = source.find(shaderVersionToken, eol);

                std::string complited = (eol == std::string::npos) ? source.substr(start) : source.substr(start, eol - start);
                Includer(complited, shaderStage, m_Directory, origin);
                // Add complited to the shaderCode using type as the key
                shaderSourceCode[shaderStage] = complited;
                
            }
        }

        position = eol;
    }

    return shaderSourceCode;
}
void shade::Shader::Includer(std::string& source, Shader::Type stage, const std::string& filePath, const std::string& origin)
{
    // Define the include token that indicates that a file is being included in the source code
    const std::string shaderIncludeToken = "#include";
    // Define the initial position to search for the include token in the source code
    std::size_t position = 0;
    // Loop through the source code until all include tokens are processed
    while (position != std::string::npos)
    {
        // Search for the first occurrence of the include token starting from the specified position
        position = source.find(shaderIncludeToken, position);
        // Search for the first occurrence of a line break character after the include token
        std::size_t eol = source.find_first_of("\r\n", position);
        // If a line break character was found
        if (eol != std::string::npos)
        {
            // Get the path to the file being included, which is between the include token and the line break character
            std::size_t begin = position + shaderIncludeToken.size();
            std::string path = source.substr(begin, eol - begin);
            // Remove any quotation marks or white space from the file path
            path.erase(std::remove(path.begin(), path.end(), '\"'), path.end());
            path.erase(std::remove_if(path.begin(), path.end(), ::isspace), path.end());
            //path.erase(path.begin(), std::find_if(path.begin(), path.end(), [](auto c) { return !std::isspace(c); }));

            auto exist = m_Headers[stage][origin].find(std::filesystem::path(path).filename().stem().string());
            if (exist == m_Headers[stage][origin].end())
            {
                m_Headers[stage][origin].insert(std::filesystem::path(path).filename().stem().string());
                // Remove the entire include statement from the source code
                source.erase(position, eol - position);

                std::string includeSource;
                {
                    std::size_t count = 0;
                    std::size_t upDirPosition = path.find("./");

                    while (upDirPosition != std::string::npos)
                    {
                        upDirPosition = path.find("./", upDirPosition + 2);
                        count++;
                    }

                    path.erase(0, count * 2);
                    auto upDirectory = filePath;
                    for (auto i = 0; i < count; i++)
                        upDirectory = std::filesystem::path(upDirectory).parent_path().parent_path().string();

                    // Read the contents of the included file
                    includeSource = ReadFile(upDirectory + "/" + path);
                }
                // If the included file was successfully read, insert its contents into the source code at the position of the include statement
                if (includeSource.size())
                {
                    Includer(includeSource, stage, filePath, origin);
                    source.insert(position, includeSource);
                }
            }
            else
            {
                // Remove the entire include statement from the source code
                source.erase(position, eol - position);
                position++;
            }
        }
    }
}

shade::Shader::Type shade::Shader::GetTypeFromString(std::string& str)
{
    if (str == "vertex")
        return Type::Vertex;
    if (str == "fragment")
        return Type::Fragment;
    if (str == "geometry")
        return Type::Geometry;
    if (str == "compute")
        return Type::Compute;

    SHADE_CORE_WARNING("Undefined shader type!");
    return Type::Undefined;
}

std::string shade::Shader::GetTypeAsString(Shader::Type type)
{
    switch (type)
    {
        case   Shader::Type::Vertex: return "Vertex";
        case   Shader::Type::Fragment: return "Fragment";
        case   Shader::Type::Compute: return "Compute";
        case   Shader::Type::Geometry: return "Geometry";
        default: return "Undefined";
    }
}

