#pragma once
#include <shade/core/entity/Storage.h>
#include <shade/core/entity/EntityManager.h>

namespace shade
{
	namespace ecs
	{
		class Entity;

		/* View class that allow us to iterate through all entites with given set of components */
		template<typename Entity, typename... Component>
		class BasicView
		{
		public:
			using OtherPools = std::array<const SparseSet<Entity>*, (sizeof...(Component) - 1)>;
			using Candidate = SparseSet<Entity>;
			using Pools = std::unordered_map<TypeHash, std::shared_ptr<Storage<Entity>>>; // Was unique_ptr
			/* View iterator to to iterate through all valid entities with given set of components */
			template<typename Entity>
			class BasicViewIterator
			{
			public:
				using iterator_category = std::random_access_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = Entity;
				using pointer = value_type*;
				using reference = value_type&;
			public:
				BasicViewIterator(pointer first = nullptr, pointer last = nullptr, EntityManager* manager = nullptr, const OtherPools pools = {}) :
					m_First(first), m_Last(last), m_Current(first), m_Manager(manager), m_Pools(pools)
				{
					/* Make sure that first entity has set of given components */
					if (m_Current != m_Last && !InOtherPools())
						++(*this);
					if (m_Current)
						m_Entity.m_Handle = *m_Current;
					m_Entity.m_Manager = m_Manager;
				}
				~BasicViewIterator() = default;
			public:
				BasicViewIterator& operator++(int) noexcept { while (++m_Current != m_Last && !InOtherPools()); m_Entity.m_Handle = *m_Current; m_Entity.m_Manager = m_Manager; return (*this); }
				BasicViewIterator& operator--(int) noexcept { while (--m_Current != m_Last && !InOtherPools()); m_Entity.m_Handle = *m_Current; m_Entity.m_Manager = m_Manager; return (*this); }
				BasicViewIterator& operator++() noexcept { while (++m_Current != m_Last && !InOtherPools()); m_Entity.m_Handle = *m_Current; m_Entity.m_Manager = m_Manager; return (*this); }
				BasicViewIterator& operator--() noexcept { while (--m_Current != m_Last && !InOtherPools()); m_Entity.m_Handle = *m_Current; m_Entity.m_Manager = m_Manager; return (*this); }
				bool operator==(const BasicViewIterator& other) const noexcept { return other.m_Current == m_Current; }
				bool operator!=(const BasicViewIterator& other) const noexcept { return other.m_Current != m_Current; }
				ecs::Entity& operator*() { return m_Entity; }
				ecs::Entity* operator->() { return &m_Entity; }
				const ecs::Entity& operator*() const { return m_Entity; }
				const ecs::Entity* operator->() const { return &m_Entity; }
				operator bool() const { if (m_Current) return true; else return false; }
			private:
				pointer const m_First;
				pointer const m_Last;
				pointer m_Current;
				const OtherPools m_Pools;
				EntityManager* const m_Manager;
				ecs::Entity m_Entity;
			private:
				/* Check if entity exist in other needed pools*/
				[[nodiscard]] bool InOtherPools() const
				{
					return std::all_of(m_Pools.cbegin(), m_Pools.cend(), [entt = *m_Current](const SparseSet<Entity>* current) { return current->Contains(entt); });
				}
			};
		public:
			using iterator = BasicViewIterator<Entity>;
			using const_iterator = BasicViewIterator<const Entity>;
		public:
			BasicView(const SparseSet<Entity>* candidate = nullptr, const Pools* pools = nullptr, EntityManager* manager = nullptr) :
				m_Candidate(candidate), m_Pools(pools), m_Manager(manager)
			{}
			virtual ~BasicView() = default;
			/* Execute for each entity with given set of components */
			template<typename Function>
			void Each(Function function)
			{
				for (auto& entity : *this)
					function(entity, m_Manager->GetComponent<Component>(entity)...);
			}
		public:
			/* Begin of view iterator */
			iterator begin() noexcept { return iterator(_EntitiesBegin(), _EntitiesEnd(), m_Manager, PrepareOtherPools(m_Candidate, m_Pools)); };
			/* End of view iterator */
			iterator end() noexcept { return iterator(_EntitiesEnd(), _EntitiesEnd(), m_Manager, PrepareOtherPools(m_Candidate, m_Pools)); };
			/* Const begin of view iterator */
			const_iterator cbegin() const noexcept { return const_iterator(_EntitiesBegin(), _EntitiesEnd(), m_Manager, PrepareOtherPools(m_Candidate, m_Pools)); };
			/* Const end of view iterator */
			const_iterator cend() const noexcept { return const_iterator(_EntitiesEnd(), _EntitiesEnd(), m_Manager, PrepareOtherPools(m_Candidate, m_Pools)); };
		private:
			const Candidate* m_Candidate;
			const Pools* m_Pools;
			EntityManager* const m_Manager;
		private:
			const Entity* _EntitiesBegin() const noexcept { return (m_Candidate) ? m_Candidate->GetData() : nullptr; };
			const Entity* _EntitiesEnd()   const noexcept { return (m_Candidate) ? m_Candidate->GetData() + m_Candidate->GetSize() : nullptr; };
			Entity* _EntitiesBegin() noexcept { return const_cast<Entity*>(const_cast<const BasicView*>(this)->_EntitiesBegin()); };
			Entity* _EntitiesEnd()  noexcept { return const_cast<Entity*>(const_cast<const BasicView*>(this)->_EntitiesEnd()); };

			/* Prepare pools of needed components */
			[[nodiscard]] OtherPools PrepareOtherPools(const Candidate* candidate, const Pools* pools) const
			{
				std::size_t position = 0; OtherPools others{};
				if (candidate)
					((static_cast<const SparseSet<Entity>*>((*pools).at(Hash<Component>()).get()) == candidate ? nullptr : (others[position] = static_cast<const SparseSet<Entity>*>((*pools).at(Hash<Component>()).get()), others[position++])), ...);
				return others;
			}
		};
	}
}