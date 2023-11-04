#include "shade_pch.h"
#include "ScriptableEntity.h"

const std::string& shade::ecs::ScriptableEntity::GetModuleName() const
{
	return m_ModuleName;
}

const std::string& shade::ecs::ScriptableEntity::GetName() const
{
	return m_Name;
}

