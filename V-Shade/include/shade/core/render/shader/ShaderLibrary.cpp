#include "shade_pch.h"
#include "ShaderLibrary.h"

std::unordered_map<std::string, shade::SharedPointer<shade::Shader>> shade::ShaderLibrary::m_sLibrary;

shade::SharedPointer<shade::Shader> shade::ShaderLibrary::Create(const Shader::Specification& specification)
{
    auto shader = m_sLibrary.find(specification.Name);
    if (shader == m_sLibrary.end())
    {
       return m_sLibrary[specification.Name] = Shader::Create(specification);
    }
    else
    {
        SHADE_CORE_WARNING("Shader '{0}' is already exist in library!", specification.Name);
        return m_sLibrary[specification.Name];
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
