#include "shade_pch.h"
#include "Entity.h"

shade::ecs::Entity::Entity(EntityID handle, const EntityManager* manager) :
	m_Handle(handle), m_Manager(const_cast<EntityManager*>(manager))
{
}

shade::ecs::EntityID* shade::ecs::Entity::_ChildrenBegin() noexcept
{
	return const_cast<shade::ecs::EntityID*>(const_cast<const shade::ecs::Entity*>(this)->_ChildrenBegin());
}

shade::ecs::EntityID* shade::ecs::Entity::_ChildrenEnd() noexcept
{
	return const_cast<shade::ecs::EntityID*>(const_cast<const shade::ecs::Entity*>(this)->_ChildrenEnd());
}

const shade::ecs::EntityID* shade::ecs::Entity::_ChildrenBegin() const noexcept
{
	assert(IsValid() && " Entity isn't valid !");
	const auto position = EntityTraits<EntityID>::ToID(m_Handle);
	auto& children = std::get<2>(m_Manager->m_Entities[position]);
	return children.data();
}

const shade::ecs::EntityID* shade::ecs::Entity::_ChildrenEnd() const noexcept
{
	assert(IsValid() && " Entity isn't valid !");
	const auto position = EntityTraits<EntityID>::ToID(m_Handle);
	auto& children = std::get<2>(m_Manager->m_Entities[position]);
	return children.data() + children.size();
}

bool shade::ecs::Entity::IsValid() const
{
	return (m_Manager != nullptr && m_Manager->IsValidEntity(m_Handle));
}

shade::ecs::EntityVersion shade::ecs::Entity::GetVersion() const
{
	assert(IsValid() && " Entity isn't valid !");
	return EntityTraits<EntityID>::VersionType((EntityTraits<EntityID>::ToIntegral(m_Handle) >> EntityTraits<EntityID>::EntityShift));
}

shade::ecs::EntityID shade::ecs::Entity::GetID() const
{
	return EntityTraits<EntityID>::ToID(m_Handle);
}

void shade::ecs::Entity::Destroy()
{
	assert(IsValid() && " Entity isn't valid !");
	UnsetParent();

	if (HasChildren())
	{
		for (auto& child : *this)
		{
			m_Manager->RemoveChild(m_Handle, child);
			m_Manager->SetParent(child, shade::ecs::null);
		}
	}
	m_Manager->DestroyEntity(m_Handle);
	m_Handle = shade::ecs::null;
	m_Manager = nullptr;
}

void shade::ecs::Entity::DestroyWithChildren()
{
	assert(IsValid() && " Entity isn't valid !");

	/* Because we iterate through std::vector::iterator and removing elements we need to avoid issue
	   with memory shifting after remove an element, because iterator++ increment current pointer after
	   removing and iterator show wrong element. begin() will always set to next element after remove
	   previous one. As std::vector is tightly packed, iterator will be automatically set to next element
	   after std::vector::erase. See EntityManager::RemoveChild */
	while (begin() != end())
		begin()->DestroyWithChildren();

	Destroy();
}

void shade::ecs::Entity::AddChild(Entity& child)
{
	assert(IsValid() && child.IsValid() && " Entity isn't valid !");
	assert(*this != child && "Couldn't add child, parent and child are the same!");
	assert(!IsChildOf(child) && "Couldn't add child, parent and child allready linked!");
	assert(!m_Manager->HasParent(child) && "Couldn't add child, has allready parent!");

	if (!IsChildOf(child) && !m_Manager->HasParent(child))
	{
		m_Manager->AddChild(m_Handle, child);
		child.SetParent(*this);
	}
}

void shade::ecs::Entity::RemoveChild(Entity& child)
{
	assert(IsValid() && child.IsValid() && " Entity isn't valid !");
	assert(*this != child && "Couldn't add child, parent and child are the same!");

	if (m_Manager->RemoveChild(m_Handle, child))
		child.UnsetParent(true);
	else assert(false && "Child isn't part of current entitie!");
}

void shade::ecs::Entity::RemoveChildren()
{
	while (begin() != end())
		RemoveChild(*begin());
}

void shade::ecs::Entity::RemoveAndDestroyChildren()
{
	while (begin() != end())
	{
		Entity child = *begin();
		RemoveChild(*begin());
		child.Destroy();
	}
}

void shade::ecs::Entity::SetParent(Entity& parent)
{
	assert(IsValid() && " Entity isn't valid !");
	if (HasParent())
	{
		m_Manager->RemoveChild(parent, m_Handle);
	}
	m_Manager->SetParent(m_Handle, parent);
}

void shade::ecs::Entity::UnsetParent(const bool& isRecursive)
{
	assert(IsValid() && " Entity isn't valid !");
	auto parent = m_Manager->GetParent(m_Handle);
	if (HasParent() && !isRecursive)
	{
		m_Manager->RemoveChild(parent, m_Handle);
	}
	m_Manager->SetParent(m_Handle, shade::ecs::null);
}

bool shade::ecs::Entity::HasChildren() const
{
	assert(IsValid() && " Entity isn't valid !");
	return m_Manager->HasChildren(m_Handle);
}

bool shade::ecs::Entity::HasParent() const
{
	assert(IsValid() && " Entity isn't valid !");
	return m_Manager->HasParent(m_Handle);
}

bool shade::ecs::Entity::IsChildOf(const Entity& parent) const
{
	if (HasParent())
	{
		if (GetParent() == parent)
			return true;

		if (GetParent().IsChildOf(parent))
			return true;
	}
	return false;
}

shade::ecs::Entity shade::ecs::Entity::GetParent() const
{
	assert(IsValid() && " Entity isn't valid !");
	return { m_Manager->GetParent(m_Handle), m_Manager };
}

std::size_t shade::ecs::Entity::GetChildrensCount() const
{
	assert(IsValid() && " Entity isn't valid !");
	const auto position = EntityTraits<EntityID>::ToID(m_Handle);
	auto& children = std::get<2>(m_Manager->m_Entities[position]);

	return children.size();
}
