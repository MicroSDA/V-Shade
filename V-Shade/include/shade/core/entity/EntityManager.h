#pragma once
#include <shade/core/entity/View.h>
#include <shade/core/entity/System.h>


namespace shade
{
	namespace ecs
	{
		class Entity;
		/* Entity manager, collect all entities */
		class SHADE_API EntityManager
		{
			friend class Entity;

			template<typename Entity, typename... Component>
			friend class BasicView;

			/* Entity manager iterator to iterate through all valid entities */
			template<typename Entity>
			class EntityManagerIterator
			{
			public:
				using iterator_category = std::random_access_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = Entity;
				using pointer = value_type*;
				using reference = value_type&;

				using const_pointer = const value_type*;
				using const_reference = const value_type&;
			public:
				EntityManagerIterator(pointer first = nullptr, pointer last = nullptr, EntityManager* manager = nullptr) :
					m_First(first), m_Last(last), m_Current(first), m_Manager(manager)
				{
					/* Make sure that we are iterating only through valid entity*/
					if (m_Current != m_Last && !m_Manager->IsValidEntity(std::get<0>(*m_Current)))
						++(*this);
				}
			public:
				EntityManagerIterator& operator++(int) noexcept { while (++m_Current != m_Last && !m_Manager->IsValidEntity(std::get<0>(*m_Current))); return (*this); }
				EntityManagerIterator& operator--(int) noexcept { while (--m_Current != m_Last && !m_Manager->IsValidEntity(std::get<0>(*m_Current))); return (*this); }
				EntityManagerIterator& operator++() noexcept { while (++m_Current != m_Last && !m_Manager->IsValidEntity(std::get<0>(*m_Current))); return (*this); }
				EntityManagerIterator& operator--() noexcept { while (--m_Current != m_Last && !m_Manager->IsValidEntity(std::get<0>(*m_Current))); return (*this); }
				bool operator==(const EntityManagerIterator& other) const noexcept { return other.m_Current == m_Current; }
				bool operator!=(const EntityManagerIterator& other) const noexcept { return other.m_Current != m_Current; }

				const auto& operator*() const { return std::get<0>(*m_Current); }
				const auto* operator->() const { return &std::get<0>(m_Current); }

				auto& operator*() { return std::get<0>(*m_Current); }
				auto* operator->() { return &std::get<0>(m_Current); }

				//EntityID& operator*() { return std::get<0>(*m_Current); }
				//EntityID* operator->() { return &std::get<0>(*m_Current); }

				//const reference operator*() const { return *m_Current; }
				//const pointer operator->() const { return m_Current; }

				operator bool() const { if (m_Current) return true; else return false; }
			private:
				pointer const m_First;
				pointer const m_Last;
				pointer m_Current;
				EntityManager* const m_Manager;
			};

			/* Manager */
		public: 
			using Pools = std::unordered_map<TypeHash, std::shared_ptr<Storage<EntityID>>>; // shared_ptr should be unique_ptr
			using Systems = std::unordered_map<TypeID, std::shared_ptr<BasicSystem>>; // shared_ptr should be unique_ptr
			//  0 = Handle, 1 = Parent Handle, 2 = Childs
			using EntityData = std::tuple<EntityID, EntityID, std::vector<EntityID>>;
		private:
			using iterator = EntityManagerIterator<EntityData>;
			using const_iterator = EntityManagerIterator<const EntityData>;
		public:
			EntityManager() = default;
			EntityManager(void(*onCreateEntity)(Entity&));
			virtual ~EntityManager();
		public:
			/* Begin of entities iterator */
			iterator begin() noexcept { return iterator(_EntitiesBegin(), _EntitiesEnd(), this); };
			/* End of entities iterator */
			iterator end() noexcept { return iterator(_EntitiesEnd(), _EntitiesEnd(), this); };
			/* Const begin of entities iterator */
			const_iterator begin() const noexcept { return const_iterator(_EntitiesBegin(), _EntitiesEnd(), const_cast<EntityManager*>(this)); };
			/* Const end of entities iterator */
			const_iterator end() const noexcept { return const_iterator(_EntitiesEnd(), _EntitiesEnd(), const_cast<EntityManager*>(this)); };
		public:
			/* Create an entity */
			Entity CreateEntity();
			/* Destory all entities */
			void DestroyAllEntites();
			/* Set on entiti create callback function */
			void SetOnEntityCreate(void(*function)(Entity&));
			/* Return true if manager has give component pool */
			template<typename Component>
			bool HasComponentPool() const 
			{
				static const TypeHash hash = Hash<Component>();
				return (m_Pools.find(hash) != m_Pools.end());
			}
			/* Return view class that allow us to iterate through all entites with given set of components */
			template<typename... Component>
			BasicView<EntityID, Component...> View() 
			{
				return BasicView<EntityID, Component...>(_GetCandidate<EntityID, Component...>(), &m_Pools, this); 
			}
			template<typename Component>
			void RegisterSystem(void(*onCreate)(Component&), void(*onUpdate)(Component&), void(*onDestroy)(Component&))
			{
				static const TypeID index = TypeInfo<Component>::ID();
				//m_Systems[index] = std::make_shared<System<Component>>(onCreate, onUpdate, onDestroy);
			}
			template<typename Component>
			void OnUpdateSystem()
			{
				static const TypeID index = TypeInfo<Component>::ID();
				if (HasComponentPool<Component>() && m_Systems.find(index) != m_Systems.end())
				{
					auto& components = static_cast<ComponentStorage<Component, EntityID>*>(m_Pools.at(index).get())->m_Components;
					for (auto& component : components)
						static_cast<System<Component>*>(m_Systems.at(index).get())->OnUpdate(*static_cast<Component*>(component.get()));
				}
			}
			/* Return count of valid entities */
			std::size_t EntitiesCount() const;
		protected:
			// TODO: HasComponents!
			/* Return true if entiti has give component */
			template<typename Component>
			bool HasComponent(const EntityID& entity) const
			{
				const auto handle = EntityTraits<EntityID>::ToID(entity);
				static const TypeHash hash = Hash<Component>();
				return (HasComponentPool<Component>() && static_cast<ComponentStorage<Component, EntityID>*>(m_Pools.at(hash).get())->Contains(handle));
			}
			/* Add component to entity */
			template<typename Component, typename... Args>
			Component& AddComponent(const EntityID& entity, Args&&... args)
			{
				static const TypeHash hash = Hash<Component>();
				const auto handle = EntityTraits<EntityID>::ToID(entity);
				if (!HasComponentPool<Component>())
				{
					m_Pools.insert({ hash, std::make_shared<ComponentStorage<Component, EntityID>>() });
				}

				auto& component = static_cast<ComponentStorage<Component, EntityID>*>(m_Pools.at(hash).get())->Add(handle, std::forward<Args>(args)...);

				/*if (m_Systems.find(index) != m_Systems.end())
					static_cast<System<Component>*>(m_Systems.at(index).get())->OnCreate(component);*/

				return component;
			}
			/* Get component from entity */
			template<typename Component>
			Component& GetComponent(const EntityID& entity)
			{
				assert(HasComponentPool<Component>() && "Entity doesn't have the component !");
				const auto handle = EntityTraits<EntityID>::ToID(entity);
				static const TypeHash hash = Hash<Component>();

				return static_cast<ComponentStorage<Component, EntityID>*>(m_Pools.at(hash).get())->Get(handle);
			}
			/* Get component from entity */
			template<typename Component>
			Component* GetComponentRaw(const EntityID& entity)
			{
				assert(HasComponentPool<Component>() && "Entity doesn't have the component !");
				const auto handle = EntityTraits<EntityID>::ToID(entity);
				static const TypeHash hash = Hash<Component>();

				return static_cast<ComponentStorage<Component, EntityID>*>(m_Pools.at(hash).get())->GetRaw(handle);
			}
			template<typename Component>
			Component& GetComponent(const EntityID& entity) const
			{
				assert(HasComponentPool<Component>() && "Entity doesn't have the component !");
				const auto handle = EntityTraits<EntityID>::ToID(entity);
				static const TypeHash hash = Hash<Component>();

				return static_cast<ComponentStorage<Component, EntityID>*>(m_Pools.at(hash).get())->Get(handle);
			}
			/* Get component from entity */
			template<typename Component>
			Component* GetComponentRaw(const EntityID& entity) const
			{
				assert(HasComponentPool<Component>() && "Entity doesn't have the component !");
				const auto handle = EntityTraits<EntityID>::ToID(entity);
				static const TypeHash hash = Hash<Component>();

				return static_cast<ComponentStorage<Component, EntityID>*>(m_Pools.at(hash).get())->GetRaw(handle);
			}
			/* Remove component from entity */
			template<typename Component>
			void RemoveComponent(const EntityID& entity)
			{
				assert(HasComponentPool<Component>() && "Entity doesn't have the component !");
				const auto handle = EntityTraits<EntityID>::ToID(entity);
				static const TypeHash hash = Hash<Component>();

				auto& component = GetComponent<Component>(entity);
				if (m_Systems.find(hash) != m_Systems.end())
					static_cast<ComponentStorage<Component, EntityID>*>(m_Pools.at(hash).get())->Remove(handle, m_Systems.at(hash).get());
				else
					static_cast<ComponentStorage<Component, EntityID>*>(m_Pools.at(hash).get())->Remove(handle);

			}
			
			/* Return true if entity is valid */
			bool IsValidEntity(const EntityID& entity) const;
			/* Destory entity */
			void DestroyEntity(const EntityID& entity);
			/* Add child to entity */
			void AddChild(const EntityID& entity, const EntityID& child);
			/* Remove child from entity */
			bool RemoveChild(const EntityID& entity, const EntityID& child);
			/* Return true if entity has children */
			bool HasChildren(const EntityID& entity) const;
			/* Return parent handle*/
			EntityID GetParent(const EntityID& entity) const;
			/* Return true if entity has parent */
			bool HasParent(const EntityID& entity);
			/* Set parent for entity */
			void SetParent(const EntityID& entity, const EntityID& parent);
		private:
			const EntityData* _EntitiesBegin() const noexcept;
			const EntityData* _EntitiesEnd() const noexcept;

			EntityData* _EntitiesBegin() noexcept;
			EntityData* _EntitiesEnd() noexcept;
			/* Return lowest SparseSet or nullptr */
			template<typename Entity, typename... Component>
			const SparseSet<Entity>* _GetCandidate() const
			{
				if ((HasComponentPool<Component>() && ...))
				{
					return (std::min)({ static_cast<const SparseSet<Entity>*>(m_Pools.at(Hash<Component>()).get())... }, [](const auto& left, const auto& right)
						{
							return left->GetSize() < right->GetSize();
						});
				}
				return static_cast<SparseSet<Entity>*>(nullptr);
			}
		private:
			Pools m_Pools;
			Systems m_Systems;
			EntityID m_Destroyed = ecs::null;
			std::vector<EntityData>	m_Entities;
			std::size_t m_EntitiesCount = 0u;
			void (*m_OnEntityCreate)(Entity&) = nullptr;
		private:
		};
	}
}