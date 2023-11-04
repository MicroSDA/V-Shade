#pragma once
#include <shade/core/memory/Memory.h>
#include <shade/core/scripting/ScriptableEntity.h>
#include <shade/core/components/Components.h>
#include <shade/utils/Logger.h>
#include <Windows.h>

namespace shade
{
	// This class represents a dynamic library which can be loaded and used by the program.
	class SHADE_API DynamicLibrary
	{
	public:
		DynamicLibrary(const std::string& name, std::size_t size, HMODULE module);
		virtual ~DynamicLibrary();
		// Creates a new shared pointer to a DynamicLibrary instance.
		static SharedPointer<DynamicLibrary> Create(const std::string& filePath, const std::string& name);
		// Template function for getting a script from the library.
		template<typename T, typename... Args>
		auto GetScript(const std::string& name, Args&&... args);
	private:
		std::string m_Name;
		std::size_t m_Size;
		HMODULE m_Module;
	private:
		std::unordered_map<std::string, std::vector<ecs::EntityID>> m_ScriptsInstances;
	};

	// Template function for getting a script from the library.
	template<typename T, typename ...Args>
	inline auto DynamicLibrary::GetScript(const std::string& name, Args&& ...args)
	{
		// Define the script type.
		typedef T(*script_t)(Args&& ...);
		// Get the count of the arguments.
		constexpr int argcCount = sizeof...(Args);
		// Return the script.
		return (T)((script_t)GetProcAddress(m_Module, name.c_str()))(std::forward<Args>(args)...);
	}

	// This class is responsible for managing scripts and their libraries.
	class SHADE_API ScriptManager
	{
	public:
		// Initializes the manager with the given folder path.
		static void Initialize(const std::string& folderPath = "/scripts");
		// Template function for instantiating a script from a library.
		template<typename T, typename ...Args>
		static auto InstantiateScript(const std::string& moduleName, const std::string& scriptName, Args&& ...args);

		// Shuts down the script manager.
		static void ShutDown();
	private:
		static std::string m_sFolderPath;
		static std::unordered_map<std::string, SharedPointer<DynamicLibrary>> m_sLibraries;
		static bool m_sIsInitialized;
	};

	// Template function for instantiating a script from a library.
	template<typename T, typename ...Args>
	inline auto ScriptManager::InstantiateScript(const std::string& moduleName, const std::string& scriptName, Args&& ...args)
	{
		// Make sure the manager is initialized.
		assert(m_sIsInitialized);
		// Check if the library exists.
		if (m_sLibraries.find(moduleName) != m_sLibraries.end())
		{
			// If it does, call the GetScript function of DynamicLibrary and return the result.
			auto script = m_sLibraries.at(moduleName)->GetScript<T>(scriptName, std::forward<Args>(args)...);
			script->m_ModuleName = moduleName; script->m_Name = scriptName;
			return script;
		}
			
		// If it doesn't exist, return an empty T.
		return T();
	}
}
