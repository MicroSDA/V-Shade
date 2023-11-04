#include "shade_pch.h"
#include "NativeScript.h"

std::size_t shade::ecs::NativeScript::SerializeAsComponent(std::ostream& stream) const
{
	std::uint32_t size = 0u;
	size +=Serializer::Serialize(stream, m_pInstance->GetModuleName());
	size +=Serializer::Serialize(stream, m_pInstance->GetName());
	return size;
}

std::size_t shade::ecs::NativeScript::DeserializeAsComponent(std::istream& stream)
{
	std::uint32_t size = 0u;
	return size;
}
