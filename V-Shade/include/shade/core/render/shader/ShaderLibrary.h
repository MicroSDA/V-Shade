#pragma once
#include <shade/core/render/shader/Shader.h>
#include <shade/utils/Logger.h>

namespace shade
{
	class SHADE_API ShaderLibrary
	{
	public:
		static SharedPointer<Shader> Create(const Shader::Specification& specification);
		static SharedPointer<Shader> Get(const std::string& name);
		static std::unordered_map<std::string, SharedPointer<Shader>>& GetLibrary();
		static void Remove(const std::string& name);
		static void ShutDown();
	private:
		static std::unordered_map<std::string, SharedPointer<Shader>> m_sLibrary;
	};
}
