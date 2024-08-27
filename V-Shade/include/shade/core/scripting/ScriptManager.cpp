#include "shade_pch.h"
#include "ScriptManager.h"

#include <Psapi.h> // Header for working with process module information

// Initialize static variables for the ScriptManager class
std::unordered_map<std::string, shade::SharedPointer<shade::scripts::DynamicLibrary>> shade::scripts::ScriptManager::m_sLibraries;
bool shade::scripts::ScriptManager::m_sIsInitialized = false;

shade::scripts::DynamicLibrary::DynamicLibrary(const std::string& name, std::size_t size, HMODULE module)
    : m_Name(name), m_Size(size), m_Module(module)
{
    FillExportedFunctions(); // Populate the map with functions exported by this library
}

shade::scripts::DynamicLibrary::~DynamicLibrary()
{
    if (m_Module)
        FreeLibrary(m_Module); // Free the loaded module to prevent memory leaks
}

shade::SharedPointer<shade::scripts::DynamicLibrary> shade::scripts::DynamicLibrary::Create(const std::string& filePath, const std::string& name)
{
    try
    {
        HMODULE module = LoadLibraryA(filePath.c_str()); // Load the dynamic library
        if (!module)
            throw std::exception(std::format("Failed to load library name = {0}, path = {1} !", name, filePath).c_str()); // Throw an exception if loading fails

        MODULEINFO info;
        GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info)); // Retrieve module information

        SHADE_CORE_TRACE("Script name = {0}, path = {1} has been loaded successfully.", name, filePath); // Log successful load

        auto library = SharedPointer<DynamicLibrary>::Create(name, info.SizeOfImage, module); // Create a DynamicLibrary object

        return library; // Return the created library
    }
    catch (std::exception& exception)
    {
        SHADE_CORE_WARNING(exception.what()); // Log any exception that occurs
        return nullptr; // Return null if an error occurs
    }
}

const std::unordered_map<std::string, shade::scripts::FunctionInfo>& shade::scripts::DynamicLibrary::GetExportedFunctions() const
{
    return m_FunctionsInfo; // Return the map of function information
}

void shade::scripts::DynamicLibrary::FillExportedFunctions()
{
    std::unordered_map<std::string, FunctionInfo> functions; // Create a temporary map to store functions

    auto dosHeader = (PIMAGE_DOS_HEADER)m_Module; // Get DOS header from the module
    auto ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)m_Module + dosHeader->e_lfanew); // Get NT headers
    auto exportDirectoryRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress; // Get the RVA of the export directory

    if (exportDirectoryRVA == 0)
    {
        SHADE_CORE_WARNING("No export directory found for library: {0}", m_Name); // Warn if no export directory is found
    }

    auto exportDirectory = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)m_Module + exportDirectoryRVA); // Get the export directory
    auto nameRVAs = (DWORD*)((BYTE*)m_Module + exportDirectory->AddressOfNames); // Get the addresses of the function names
    auto ordinalBase = exportDirectory->Base; // Get the base of the ordinals
    auto ordinals = (WORD*)((BYTE*)m_Module + exportDirectory->AddressOfNameOrdinals); // Get the ordinals

    for (DWORD i = 0; i < exportDirectory->NumberOfNames; i++)
    {
        FunctionInfo functionInfo;
        functionInfo.name = std::string((char*)((BYTE*)m_Module + nameRVAs[i])); // Get the function name
        functionInfo.ordinal = ordinals[i] + ordinalBase; // Calculate the function's ordinal
        auto functionRVA = ((DWORD*)((BYTE*)m_Module + exportDirectory->AddressOfFunctions))[ordinals[i]]; // Get the function's RVA
        functionInfo.address = (void*)((BYTE*)m_Module + functionRVA); // Calculate the function's address

        functions[functionInfo.name] = functionInfo; // Add the function to the map
    }

    m_FunctionsInfo = functions; // Store the filled map in the class member
}

void shade::scripts::ScriptManager::Initialize(const std::string& folderPath)
{
    if (!m_sIsInitialized)
    {
        if (std::filesystem::exists(folderPath))
        {
            for (const auto& entry : std::filesystem::directory_iterator(folderPath))
            {
                const std::string filePath = entry.path().generic_string(); // Convert file path to string

                if (entry.path().extension().string() == std::string(".dll"))
                {
                    const std::string filename = entry.path().filename().replace_extension().string(); // Get the DLL filename
                    auto it = m_sLibraries.find(filename);

                    if (it == m_sLibraries.end())
                    {
                        auto library = DynamicLibrary::Create(filePath, filename); // Create a DynamicLibrary object and fill its functions

                        if (library)
                            m_sLibraries.emplace(filename, library); // Add the library to the map if it was created successfully
                    }
                }
            }

            m_sIsInitialized = true; // Mark the manager as initialized
        }
    }
    else
    {
        SHADE_CORE_WARNING("Script manager has been already initialized!"); // Warn if the manager was already initialized
    }
}

void shade::scripts::ScriptManager::ShutDown()
{
    m_sLibraries.clear(); // Clear the map of loaded libraries
}

const shade::scripts::FunctionInfo* shade::scripts::ScriptManager::GetFunctionInfo(const std::string& moduleName, const std::string& functionName)
{
    if (m_sLibraries.find(moduleName) != m_sLibraries.end())
    {
        auto& module = m_sLibraries.at(moduleName); // Get the library

        auto functions = module->GetExportedFunctions(); // Get the functions map

        if (functions.find(functionName) != functions.end())
        {
            return &functions.at(functionName); // Return the function information if found
        }
    }
    return nullptr; // Return null if the module or function was not found
}
