#include "shade_pch.h"
#include "ShaderLibrary.h"

std::unordered_map<std::string, shade::SharedPointer<shade::Shader>> shade::ShaderLibrary::m_sLibrary;

shade::SharedPointer<shade::Shader> shade::ShaderLibrary::Create(const std::string& name, const std::string& filePath)
{
    auto shader = m_sLibrary.find(name);
    if (shader == m_sLibrary.end())
    {
       return m_sLibrary[name] = Shader::Create(filePath);
    }
    else
    {
        SHADE_CORE_WARNING("Shader '{0}' is already exist in library!", name);
        return m_sLibrary[name];
    }
}

shade::SharedPointer<shade::Shader> shade::ShaderLibrary::Get(const std::string& name)
{
    auto shader = m_sLibrary.find(name);
    if (shader != m_sLibrary.end())
        return shader->second;
    else
    {
        SHADE_CORE_WARNING("Shader '{0}' doensn't exist in library!", name);
        return nullptr;
    }
}

std::unordered_map<std::string, shade::SharedPointer<shade::Shader>>& shade::ShaderLibrary::GetLibrary()
{
    return m_sLibrary;
}

void shade::ShaderLibrary::Remove(const std::string& name)
{
    auto shader = m_sLibrary.find(name);
    if (shader != m_sLibrary.end())
        m_sLibrary.erase(shader);
    else
        SHADE_CORE_WARNING("Shader '{0}' doensn't exist in library!", name);
}

void shade::ShaderLibrary::ShutDown()
{
    m_sLibrary.clear();
}
