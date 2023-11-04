#pragma once
#include <shade/core/entity/Common.h>

namespace shade
{
	namespace ecs
	{
		/* Sparse set class, allow us to get tightly packed array */
		template<typename T>
		class SparseSet
		{
		public:
			/* SparseSet iterator to iterate through tightly packed array */
			template<typename T>
			class SparseSetIterator final
			{
				friend class SparseSet<T>;
			public:
				using iterator_category = std::random_access_iterator_tag;
				using difference_type = std::ptrdiff_t;
				using value_type = T;
				using pointer = value_type*;
				using reference = value_type&;
			public:
				SparseSetIterator(pointer first = nullptr, pointer last = nullptr) :
					m_First(first), m_Last(last), m_Current(first) {};
				~SparseSetIterator() = default;
			public:
				SparseSetIterator& operator++(int) noexcept { return ++m_Current, * this; }
				SparseSetIterator& operator--(int) noexcept { return --m_Current, * this; }
				SparseSetIterator& operator++() noexcept { return ++m_Current, * this; }
				SparseSetIterator& operator--() noexcept { return --m_Current, * this; }
				bool operator==(const SparseSetIterator& other) const noexcept { return other.m_Current == m_Current; }
				bool operator!=(const SparseSetIterator& other) const noexcept { return other.m_Current != m_Current; }
				reference operator*() { return *m_Current; }
				pointer operator->() { return m_Current; }
				const reference operator*() const { return *m_Current; }
				const pointer operator->() const { return m_Current; }
				operator bool() const { if (m_Current) return true; else return false; }
			private:
				pointer const m_First;
				pointer const m_Last;
				pointer m_Current;
			};

			/* Sparse set */
		public:
			using iterator = SparseSetIterator<T>;
			using const_iterator = SparseSetIterator<const T>;
		public:
			SparseSet() = default;
			~SparseSet() = default;
		public:
			/* Add element into array */
			void Push(const T& value)
			{
				const auto position = m_Packed.size();
				m_Packed.push_back(value);
				if (!(value < m_Sparse.size()))
					m_Sparse.resize(value + 1);
				m_Sparse[value] = position;
			}
			/* Remove element from array */
			void Pop(const T& value)
			{
				const auto last = m_Packed.back();
				std::swap(m_Packed.back(), m_Packed[m_Sparse[value]]);
				std::swap(m_Sparse[last], m_Sparse[value]);
				m_Packed.pop_back();
			}
			/* Sort array */
			void Sort()
			{
				std::sort(m_Packed.begin(), m_Packed.end());
				for (auto position = 0; position < m_Packed.size(); position++) {
					m_Sparse[m_Packed[position]] = position;
				}
			}
			/* Return true if array contains element */
			bool Contains(const T& value) const { return (value < m_Sparse.size() && m_Sparse[value] < m_Packed.size() && m_Packed[m_Sparse[value]] == value); }
			/* Get element position in tightly packed array */
			std::size_t GetPosition(const T& value) const { return m_Sparse[value]; }
			/* Return size of tightly packed array */
			std::size_t GetSize() const { return m_Packed.size(); }
			/* Get data pointer of tightly packed array */
			const T* GetData() const { return m_Packed.data(); }
			/* Get const data pointer of tightly packed array */
			T* GetData() { return m_Packed.data(); }
		public:
			/* Begin of tightly packed array iterator */
			iterator begin() noexcept { return iterator(_Begin(), _End()); };
			/* End of tightly packed array iterator */
			iterator end() noexcept { return iterator(_End(), _End()); };
			/* Const begin of tightly packed array iterator */
			const_iterator cbegin() const noexcept { return begin(); };
			/* Const begin of tightly packed array iterator */
			const_iterator cend() const noexcept { return end(); };
		private:
			std::vector<T> m_Sparse;
			std::vector<T> m_Packed;
		private:
			T* _Begin() noexcept { return m_Packed.data(); };
			T* _End()   noexcept { return m_Packed.data() + m_Packed.size(); };
		};
	}
}