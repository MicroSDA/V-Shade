#include "shade_pch.h"
#include "NativeScript.h"
#include <shade/core/scripting/ScriptManager.h>

void shade::ecs::NativeScript::Serialize(std::ostream& stream) const
{
	// TODO: we have to get GetModuleName() and GetName() from NativeScript itself!!
	serialize::Serializer::Serialize(stream, m_pInstance->GetModuleName());
	serialize::Serializer::Serialize(stream, m_pInstance->GetName());
}

void shade::ecs::NativeScript::Desirialize(std::istream& stream)
{
	std::string module, name;
	serialize::Serializer::Deserialize(stream, module);  serialize::Serializer::Deserialize(stream, name);
	if (ecs::ScriptableEntity* script = shade::scripts::ScriptManager::InstantiateScript<shade::ecs::ScriptableEntity*>(module, name)) Bind(script);
}
