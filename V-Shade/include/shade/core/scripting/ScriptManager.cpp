#include "shade_pch.h"
#include "ScriptManager.h"

// TODO: Remove 
#include <Psapi.h>

shade::DynamicLibrary::DynamicLibrary(const std::string& name, std::size_t size,  HMODULE module):
	m_Name(name), m_Size(size), m_Module(module)
{
}

shade::DynamicLibrary::~DynamicLibrary()
{
	if(m_Module)
		FreeLibrary(m_Module);
}

shade::SharedPointer<shade::DynamicLibrary> shade::DynamicLibrary::Create(const std::string& filePath, const std::string& name)
{
	try
	{
		HMODULE module = LoadLibraryA(filePath.c_str());
		// If load was failed return nullptr 
		if (!module)
			throw std::exception(std::format("Failed to load library name = {0}, path = {1} !", name, filePath).c_str());

		MODULEINFO info; GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info));

		SHADE_CORE_TRACE("Script name = {0}, path = {1} has been loaded successfully.", name, filePath);
		return SharedPointer<DynamicLibrary>::Create(name, info.SizeOfImage, module);
	}
	catch (std::exception& exeption)
	{
		SHADE_CORE_WARNING(exeption.what()); return nullptr;
	}
}

std::unordered_map<std::string, shade::SharedPointer<shade::DynamicLibrary>> shade::ScriptManager::m_sLibraries;
bool shade::ScriptManager::m_sIsInitialized = false;

void shade::ScriptManager::Initialize(const std::string& folderPath)
{
	if (!m_sIsInitialized)
	{
		//auto path = std::filesystem::current_path().relative_path();
		if (std::filesystem::exists(folderPath))
		{
			for (const auto& entry : std::filesystem::directory_iterator(folderPath))
			{
				// Get file path and inverse '\' to '/'.
				const std::string filePath = entry.path().generic_string();

				if (entry.path().extension().string() == std::string(".dll"))
				{
					const std::string filename = entry.path().filename().replace_extension().string();
					auto it = m_sLibraries.find(filename);

					if (it == m_sLibraries.end())
					{
						auto library = DynamicLibrary::Create(filePath, filename);
						if (library)
							m_sLibraries.emplace(filename, library);
					}
				}
			}

			m_sIsInitialized = true;
		}
	}
	else
	{
		SHADE_CORE_WARNING("Script manager has been already initialized!")
	}
}

void shade::ScriptManager::ShutDown()
{
	m_sLibraries.clear();
}

