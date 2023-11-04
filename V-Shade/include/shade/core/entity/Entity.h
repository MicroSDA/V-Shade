#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/entity/EntityManager.h>

namespace shade
{
	namespace ecs
	{
		/* Basic entity class */
		class SHADE_API Entity
		{
			template<typename Entity, typename... Component>
			friend class BasicView;
		public:
			/* Entity iterator to iterate through entities children */
			template<typename T>
			class EntityIterator
			{
			public:
				using iterator_category = std::random_access_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = T;
				using pointer = value_type*;
				using reference = value_type&;
			public:
				EntityIterator(pointer first = nullptr, pointer last = nullptr, EntityManager* manager = nullptr) :
					m_First(first), m_Last(last), m_Current(first), m_Manager(manager)
				{
					/* Getting entity handle */
					m_Entity.m_Manager = m_Manager;

					if (m_Current != nullptr)
						m_Entity.m_Handle = *m_Current;
				}
				~EntityIterator() = default;
			public:
				EntityIterator& operator++(int) noexcept { ++m_Current; m_Entity.m_Handle = *m_Current; m_Entity.m_Manager = m_Manager; return (*this); }
				EntityIterator& operator--(int) noexcept { --m_Current; m_Entity.m_Handle = *m_Current; m_Entity.m_Manager = m_Manager; return (*this); }
				EntityIterator& operator++() noexcept { ++m_Current; m_Entity.m_Handle = *m_Current; m_Entity.m_Manager = m_Manager; return (*this); }
				EntityIterator& operator--() noexcept { --m_Current; m_Entity.m_Handle = *m_Current; m_Entity.m_Manager = m_Manager; return (*this); }
				bool operator==(const EntityIterator& other) const noexcept { return other.m_Current == m_Current; }
				bool operator!=(const EntityIterator& other) const noexcept { return other.m_Current != m_Current; }
				Entity& operator*() { return m_Entity; }
				Entity* operator->() { return &m_Entity; }
				const Entity& operator*() const { return m_Entity; }
				const Entity* operator->() const { return &m_Entity; }
				operator bool() const { if (m_Current) return true; else return false; }
			private:
				pointer const m_First;
				pointer const m_Last;
				pointer m_Current;
				EntityManager* const m_Manager;
				Entity m_Entity;
			};

		public:
			using iterator = EntityIterator<EntityID>;
			using const_iterator = EntityIterator<const EntityID>;
		public:
			// Constructor 1
			//Entity(EntityID handle = ecs::null, EntityManager* manager = nullptr);
			// Constructor 2
			Entity(EntityID handle = ecs::null, const EntityManager* manager = nullptr);

			virtual ~Entity() = default;
			Entity(const Entity&) = default;
		public:
			/* Begin of children iterator */
			iterator begin() noexcept { return iterator(_ChildrenBegin(), _ChildrenEnd(), m_Manager); };
			/* End of children iterator */
			iterator end() noexcept { return iterator(_ChildrenEnd(), _ChildrenEnd(), m_Manager); };
			/* Const begin of children iterator */
			const_iterator begin() const noexcept { return const_iterator(_ChildrenBegin(), _ChildrenEnd(), m_Manager); };
			/* Const end of children iterator */
			const_iterator end() const noexcept { return const_iterator(_ChildrenEnd(), _ChildrenEnd(), m_Manager); };
		public:
			/* Add component to entity */
			template<typename Component, typename... Args>
			Component& AddComponent(Args&&... args)
			{
				assert(IsValid() && " Entity isn't valid !");
				//static const TypeHash hash = Hash<Component>();
				return m_Manager->AddComponent<Component>(m_Handle, std::forward<Args>(args)...);
			}
			/* Get component from entity */
			template<typename Component>
			Component& GetComponent()
			{
				assert(IsValid() && " Entity isn't valid !");
				//static const TypeHash hash = Hash<Component>();
				return m_Manager->GetComponent<Component>(m_Handle);
			}
			/* Get component from entity */
			template<typename Component>
			Component* GetComponentRaw()
			{
				assert(IsValid() && " Entity isn't valid !");
				//static const TypeHash hash = Hash<Component>();
				return m_Manager->GetComponentRaw<Component>(m_Handle);
			}
			/* Remove component from entity */
			template<typename Component>
			void RemoveComponent()
			{
				assert(IsValid() && " Entity isn't valid !");
				//static const TypeHash hash = Hash<Component>();
				m_Manager->RemoveComponent<Component>(m_Handle);
			}
			/* If entity has given component */
			template<typename Component>
			const bool HasComponent() const
			{
				assert(IsValid() && " Entity isn't valid !");
				return m_Manager->HasComponent<Component>(m_Handle);
			}
			/* Is entity valid ( id != null and manager != nullptr )*/
			bool IsValid() const;
			/* Get entity recycled version */
			EntityVersion GetVersion() const;
			/* Get entity ID (handle)*/
			EntityID GetID() const;
			/* Destroy entity with all related components */
			void Destroy();
			/* Destroy entity with all related components and childrens */
			void DestroyWithChildren();
			/* Add child to entity, set curent entity as parent of child */
			void AddChild(Entity& child);
			/* Remove child from entity, unset curent entity as parent of child */
			void RemoveChild(Entity& child);
			/* Remove all childrend from antity and keep them alive */
			void RemoveChildren();
			/* Remove all childrend from antity and destroy them*/
			void RemoveAndDestroyChildren();
			/* Set parent for curent entity */
			void SetParent(Entity& parent);
			/* Unset parent from entity, bool argument for internal use, keep it as it is */
			void UnsetParent(const bool& isRecursive = false);
			/* Return true if entity has children */
			bool HasChildren() const;
			/* Return true if entity has parent */
			bool HasParent() const;
			/* Return true if entity child of parent */
			bool IsChildOf(const Entity& parent) const;
			/* Return valid entity if entity has parent, else eniti with unvalid handle */
			Entity GetParent() const;
			/* Return childrens count */
			std::size_t GetChildrensCount() const;

			/* Overloaded operator bool */
			operator bool() const { return IsValid(); }
			/* Convert entity to EntityID */
			operator EntityID() const { return m_Handle; }
			/* Convert entity to int */
			operator int() const { return static_cast<int>(m_Handle); }
			/* Overloaded operator == */
			bool operator==(const Entity& other) const { return m_Handle == other.m_Handle && m_Manager == other.m_Manager; }
			/* Overloaded operator != */
			bool operator!=(const Entity& other) const { return !(*this == other); }
			/* Concatenate enity id with std::string */
			friend std::string operator+(const std::string& string, const Entity& other) { return std::string(string + std::to_string(static_cast<EntityID>(other.GetID()))); }
			/* Convert entity to std::string */
			operator std::string() { return std::to_string(static_cast<EntityID>(GetID())); }
			/* Convert entity to const std::string */
			operator const std::string() const { return std::to_string(static_cast<EntityID>(GetID())); }
		private:
			EntityID* _ChildrenBegin() noexcept;
			EntityID* _ChildrenEnd()   noexcept;
			const EntityID* _ChildrenBegin() const noexcept;
			const EntityID* _ChildrenEnd()   const noexcept;
		private:
			EntityID m_Handle;
			EntityManager* m_Manager;
		};
	}
}
