#include "shade_pch.h"
#include "NativeScript.h"
#include <shade/core/scripting/ScriptManager.h>

std::size_t shade::ecs::NativeScript::Serialize(std::ostream& stream) const
{
	// TODO: we have to get GetModuleName() and GetName() from NativeScript itself!!
	std::size_t size = Serializer::Serialize(stream, m_pInstance->GetModuleName());
	size += Serializer::Serialize(stream, m_pInstance->GetName());
	return size;
}

std::size_t shade::ecs::NativeScript::Desirialize(std::istream& stream)
{
	std::string module, name;
	std::size_t size = Serializer::Deserialize(stream, module);  Serializer::Deserialize(stream, name);
	if (ecs::ScriptableEntity* script = shade::scripts::ScriptManager::InstantiateScript<shade::ecs::ScriptableEntity*>(module, name)) Bind(script);
	
	return size;
}
