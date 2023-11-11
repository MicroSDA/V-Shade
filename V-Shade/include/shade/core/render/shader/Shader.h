#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <iostream>
namespace shade
{
	// Shader class.
	class SHADE_API Shader
	{
	public:
		// Shader type.
		enum class Type : std::uint32_t
		{
			Undefined,
			Vertex,
			Fragment,
			Geometry,
			Compute,
			SHADER_TYPE_MAX_ENUM
		};
		enum class DataType
		{
			// TODO: Need from Hazel need to refactor 
			None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
		};
		Shader(const std::string& filePath);
		Shader() = default;
		virtual ~Shader() = default;
		// Create shader. 
		static SharedPointer<Shader> Create(const std::string& filePath);

		const std::string& GetFilePath() const;
		const std::string& GetFileDirectory() const;
		const std::string& GetFileName() const;
		void SetFilePath(const std::string& filePath);
		static std::string GetShaderCacheDirectory();
		static std::uint32_t GetDataTypeSize(const DataType& type);

		static Shader::Type GetTypeFromString(std::string& str);
		static std::string GetTypeAsString(Shader::Type type);

		virtual void Recompile() = 0;
		// For internal convert.
		template<typename T>
		T& As();
	protected:
		std::unordered_map<Shader::Type, std::string> m_SourceCode;
		virtual void TryToFindInCacheAndCompile() = 0;
	private:
		// Full path.
		std::string m_FilePath;
		// Path to directory.
		std::string m_Directory;
		// File name wthout ext.
		std::string m_FileName;

		std::unordered_map<Shader::Type, std::unordered_map<std::string, std::unordered_set<std::string>>> m_Headers;

		std::string ReadFile(const std::string& filePath);
		std::unordered_map<Shader::Type, std::string> PreProcess(std::string& source, const std::string& origin);
		void Includer(std::string& source, Shader::Type stage, const std::string& filePath, const std::string& origin);
	};
	template<typename T>
	inline T& Shader::As()
	{
		static_assert(std::is_base_of<Shader, T>::value, "");
		return static_cast<T&>(*this);
	}
}