#pragma once
#include <shade/core/memory/Memory.h>
#include <shade/core/scripting/ScriptableEntity.h>
#include <shade/core/components/Components.h>
#include <shade/utils/Logger.h>
#include <Windows.h>

namespace shade
{
	namespace scripts
	{
		/// @brief Struct that stores information about an exported function.
		struct FunctionInfo
		{
			std::string name;    ///< The name of the function.
			WORD ordinal;        ///< The ordinal number of the function.
			void* address;       ///< The memory address where the function is located.
		};

		/// @brief Class representing a dynamically loaded library (DLL).
		class SHADE_API DynamicLibrary
		{
		public:
			/// @brief Constructs a DynamicLibrary object.
			/// @param name The name of the library.
			/// @param size The size of the library in memory.
			/// @param module The handle to the loaded module.
			DynamicLibrary(const std::string& name, std::size_t size, HMODULE module);

			/// @brief Destructor, frees the loaded module.
			virtual ~DynamicLibrary();

			/// @brief Creates and returns a shared pointer to a DynamicLibrary object.
			/// @param filePath The path to the DLL file.
			/// @param name The name of the library.
			/// @return Shared pointer to the created DynamicLibrary.
			static SharedPointer<DynamicLibrary> Create(const std::string& filePath, const std::string& name);

			/// @brief Retrieves a script from the loaded library.
			/// @tparam T The type of the script to retrieve.
			/// @tparam Args The argument types for the script.
			/// @param name The name of the script function.
			/// @param args The arguments to pass to the script function.
			/// @return The result of the script function call.
			template<typename T, typename ...Args>
			SHADE_INLINE auto GetScript(const std::string& name, Args&& ...args)
			{
				typedef T(*script_t)(Args&& ...);
				return (T)((script_t)GetProcAddress(m_Module, name.c_str()))(std::forward<Args>(args)...);
			}

			/// @brief Retrieves information about the exported functions in the library.
			/// @return A map of function names to their corresponding `FunctionInfo`.
			const std::unordered_map<std::string, FunctionInfo>& GetExportedFunctions() const;

		private:
			std::string m_Name;  ///< The name of the library.
			std::size_t m_Size;  ///< The size of the library in memory.
			HMODULE m_Module;    ///< The handle to the loaded module.
			std::unordered_map<std::string, FunctionInfo> m_FunctionsInfo; ///< Map of function names to `FunctionInfo`.

			/// @brief Fills `m_FunctionsInfo` with the exported functions from the library.
			void FillExportedFunctions();
		};

		/// @brief Manages the loading, retrieval, and instantiation of scripts from dynamic libraries.
		class SHADE_API ScriptManager
		{
		public:
			/// @brief Initializes the ScriptManager with a specified folder path.
			/// @param folderPath The path to the folder containing the scripts (DLLs).
			static void Initialize(const std::string& folderPath = "/scripts");

			/// @brief Instantiates a script from a specified library and function name.
			/// @tparam T The type of the script to instantiate.
			/// @tparam Args The argument types for the script.
			/// @param moduleName The name of the module (DLL).
			/// @param functionName The name of the function to instantiate.
			/// @param args The arguments to pass to the function.
			/// @return The instantiated script object.
			template<typename T, typename ...Args>
			static SHADE_INLINE auto InstantiateScript(const std::string& moduleName, const std::string& functionName, Args&& ...args)
			{
				assert(m_sIsInitialized);
				if (m_sLibraries.find(moduleName) != m_sLibraries.end())
				{
					auto& library = m_sLibraries.at(moduleName);

					if (library->GetExportedFunctions().find(functionName) != library->GetExportedFunctions().end())
					{
#if 1
						auto& function = library->GetExportedFunctions().at(functionName);
						typedef T(*script_t)(Args&& ...);

						auto script = (T)((script_t)function.address)(std::forward<Args>(args)...);
						script->m_ModuleName = moduleName; script->m_Name = functionName;
						return script;
#else
						auto script = library->GetScript<T>(functionName, std::forward<Args>(args)...);
						script->m_ModuleName = moduleName; script->m_Name = functionName;
						return script;
#endif // 1
					}

					SHADE_CORE_WARNING("Specified function '{0}' was not found in the '{1}' module!", functionName, moduleName);
				}

				SHADE_CORE_WARNING("Specified module '{0}' does not exist in the Script Manager!", moduleName);

				return T();
			}

			/// @brief Shuts down the ScriptManager, freeing all loaded libraries.
			static void ShutDown();

			/// @brief Retrieves information about a specific function in a module.
			/// @param moduleName The name of the module (DLL).
			/// @param functionName The name of the function.
			/// @return Pointer to `FunctionInfo` if found, otherwise nullptr.
			static const FunctionInfo* GetFunctionInfo(const std::string& moduleName, const std::string& functionName);

			/// @brief Retrieves the map of loaded libraries.
			/// @return A const reference to the map of module names to `DynamicLibrary` objects.
			static const SHADE_INLINE std::unordered_map<std::string, SharedPointer<DynamicLibrary>>& GetLibraries()
			{
				return m_sLibraries;
			}

		private:
			static std::string m_sFolderPath; ///< The folder path where the script libraries are stored.
			static std::unordered_map<std::string, SharedPointer<DynamicLibrary>> m_sLibraries; ///< Map of module names to `DynamicLibrary` objects.
			static bool m_sIsInitialized; ///< Flag indicating whether the manager has been initialized.
		};
	}
}
