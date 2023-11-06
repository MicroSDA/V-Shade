#include "shade_pch.h"
#include "EntityManager.h"
#include <shade/core/entity/Entity.h>

shade::ecs::EntityManager::EntityManager(void(*onCreateEntity)(Entity&))
{
	m_OnEntityCreate = onCreateEntity;
}

shade::ecs::EntityManager::~EntityManager()
{
	DestroyAllEntites();
}

shade::ecs::Entity shade::ecs::EntityManager::CreateEntity()
{
	EntityTraits<EntityID>::EntityType handle;
	if (m_Destroyed == ecs::null)
	{
		handle = std::get<0>(m_Entities.emplace_back(std::tuple<EntityID, EntityID, std::vector<EntityID>>{ EntityTraits<EntityID>::EntityType(static_cast<EntityTraits<EntityID>::EntityType>(m_Entities.size())), ecs::null, {}}));
	}
	else
	{
		const auto current = EntityTraits<EntityID>::ToID(m_Destroyed);
		const auto version = EntityTraits<EntityID>::ToIntegral(std::get<0>(m_Entities[current])) & (EntityTraits<EntityID>::VersionMask << EntityTraits<EntityID>::EntityShift);

		m_Destroyed = EntityTraits<EntityID>::EntityType(EntityTraits<EntityID>::ToIntegral(std::get<0>(m_Entities[current])) & EntityTraits<EntityID>::EntityMask);
		handle = std::get<0>(m_Entities[current]) = EntityTraits<EntityID>::EntityType(current | version);
		// Set parrent as null
		std::get<1>(m_Entities[current]) = ecs::null;
	}

	Entity entity = Entity(handle, this);
	if (m_OnEntityCreate)
		m_OnEntityCreate(entity);

	m_EntitiesCount++;

	return entity;
}

void shade::ecs::EntityManager::DestroyAllEntites()
{
	for (auto& entity : *this)
		DestroyEntity(entity);
	m_Entities.clear();
	m_Pools.clear();
	m_Destroyed = ecs::null;
	m_EntitiesCount = 0;
}

void shade::ecs::EntityManager::SetOnEntityCreate(void(*function)(Entity&))
{
	m_OnEntityCreate = function;
}

std::size_t shade::ecs::EntityManager::EntitiesCount() const
{
	return m_EntitiesCount;

	// Doesnt work !
	//return (m_Destroyed == ecs::null) ? m_Entities.size() : m_Entities.size() - (EntityTraits<EntityID>::ToID(m_Destroyed - 1));
}
 
bool shade::ecs::EntityManager::IsValidEntity(const EntityID& entity) const
{
	const auto position = EntityTraits<EntityID>::ToID(entity);
	// In case we are iterating through View : component sparse set contains just id without version and we need to convert handle into id
	// We need to convert both to overlap all cases.
	return (entity != ecs::null && position < m_Entities.size() && EntityTraits<EntityID>::ToID(std::get<0>(m_Entities[position])) == EntityTraits<EntityID>::ToID(entity));
}

void shade::ecs::EntityManager::DestroyEntity(const EntityID& entity)
{
	/* Extract id */
	auto handle = EntityTraits<EntityID>::EntityType(entity) & EntityTraits<EntityID>::EntityMask;
	/* Extract version */
	auto version = EntityTraits<EntityID>::VersionType((EntityTraits<EntityID>::ToIntegral(handle) >> EntityTraits<EntityID>::EntityShift) + 1);
	/* Mark entity as destroyed */
	std::get<0>(m_Entities[handle]) = EntityTraits<EntityID>::EntityType(EntityTraits<EntityID>::ToIntegral(m_Destroyed) | (EntityTraits<EntityID>::ToIntegral(version) << EntityTraits<EntityID>::EntityShift));
	m_Destroyed = EntityTraits<EntityID>::EntityType(entity);
	/* Remove entity from all pools and destroy all related components */
	for (auto position = m_Pools.begin(); position != m_Pools.end(); ++position)
	{
		if (auto& pData = position->second; pData && pData->Contains(handle))
		{
			auto system = m_Systems.find(pData->GetID());
			if (system != m_Systems.end())
				pData->m_Destroy(handle, pData.get(), system->second.get());
			else
				pData->m_Destroy(handle, pData.get(), nullptr);
		}
	}

	m_EntitiesCount--;
}

void shade::ecs::EntityManager::AddChild(const EntityID& entity, const EntityID& child)
{
	const auto position = EntityTraits<EntityID>::ToID(entity);
	std::get<2>(m_Entities[position]).emplace_back(child);
}

bool shade::ecs::EntityManager::RemoveChild(const EntityID& entity, const EntityID& child)
{
	const auto position = EntityTraits<EntityID>::ToID(entity);
	auto it = std::find(std::get<2>(m_Entities[position]).begin(), std::get<2>(m_Entities[position]).end(), child);
	if (it != std::get<2>(m_Entities[position]).end())
	{
		std::get<2>(m_Entities[position]).erase(it);
		return true;
	}
	return false;
}

bool shade::ecs::EntityManager::HasChildren(const EntityID& entity) const
{
	const auto position = EntityTraits<EntityID>::ToID(entity);
	return std::get<2>(m_Entities[position]).size();
}

shade::ecs::EntityID shade::ecs::EntityManager::GetParent(const EntityID& entity) const
{
	const auto position = EntityTraits<EntityID>::ToID(entity);
	return std::get<1>(m_Entities[position]);
}

bool shade::ecs::EntityManager::HasParent(const EntityID& entity)
{
	const auto position = EntityTraits<EntityID>::ToID(entity);
	return std::get<1>(m_Entities[position]) != ecs::null;
}

void shade::ecs::EntityManager::SetParent(const EntityID& entity, const EntityID& parent)
{
	const auto position = EntityTraits<EntityID>::ToID(entity);
	std::get<1>(m_Entities[position]) = parent;
}

const shade::ecs::EntityManager::EntityData* shade::ecs::EntityManager::_EntitiesBegin() const noexcept
{
	return m_Entities.data();
}

const shade::ecs::EntityManager::EntityData* shade::ecs::EntityManager::_EntitiesEnd() const noexcept
{
	return m_Entities.data() + m_Entities.size();
}

shade::ecs::EntityManager::EntityData* shade::ecs::EntityManager::_EntitiesBegin() noexcept
{
	return const_cast<ecs::EntityManager::EntityData*>(const_cast<const ecs::EntityManager*>(this)->_EntitiesBegin());
}

shade::ecs::EntityManager::EntityData* shade::ecs::EntityManager::_EntitiesEnd() noexcept
{
	return const_cast<ecs::EntityManager::EntityData*>(const_cast<const ecs::EntityManager*>(this)->_EntitiesEnd());
}
