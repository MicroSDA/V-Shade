#pragma once
#include <unordered_map>
#include <map>
#include <glm/glm/gtx/hash.hpp>

namespace shade
{
	template <typename T>
	inline void HashCombine(std::size_t& seed, const T& hash)
	{
		std::hash<T> hasher;
		glm::detail::hash_combine(seed, hasher(hash));
	}

	// T suppos to be some sort of std::chrono::seconds or microseconds and so on.
	template<typename T> 
	std::size_t GetCurrentTimeStemp()
	{
		return static_cast<size_t>(std::chrono::duration_cast<T>(std::chrono::system_clock::now().time_since_epoch()).count());
	}

	template<typename T, std::size_t Size>
	class StackArray
	{
	public:
		using iterator = std::array<T, Size>::iterator;
		using const_iterator = std::array<T, Size>::const_iterator;

		StackArray() = default; ~StackArray() = default;
		void PushFront(const T& item)
		{
			for (std::size_t i = m_Array.size() - 1u; i > 0; --i)
				m_Array[i] = m_Array[i - 1u];
			m_Array[0u] = item;

			IncrementSize();
		}
		void PushBack(const T& item)
		{
			for (std::size_t i = 0u; i < m_Array.size() - 1u; ++i)
				m_Array[i] = m_Array[i + 1u];
			m_Array[m_Array.size() - 1u] = item;

			IncrementSize();
		}
		template<typename... Args>
		void EmplaceFront(Args&&... args)
		{
			for (std::size_t i = m_Array.size() - 1u; i > 0u; --i)
				m_Array[i] = m_Array[i - 1u];
			m_Array[0u] = T(std::forward<Args>(args)...);

			IncrementSize();
		}
		template<typename... Args>
		void EmplaceBack(Args&&... args)
		{
			for (std::size_t i = 0u; i < m_Array.size() - 1u; ++i)
				m_Array[i] = m_Array[i + 1u];
			m_Array[m_Array.size() - 1u] = T(std::forward<Args>(args)...);

			IncrementSize();
		}
		T& operator[](std::size_t i) { return m_Array[i]; }
		const T& operator[](std::size_t i) const { return m_Array[i]; }

		std::size_t GetCapasity() const { return m_Array.size(); }
		std::size_t GetSize() const { return m_Size; }

	public:
		auto begin() { return m_Array.begin(); }
		auto end() { return m_Array.end(); }

		auto begin() const { return m_Array.begin(); }
		auto end() const { return m_Array.end(); }

	private:
		void IncrementSize() { m_Size = (m_Size >= GetCapasity()) ? m_Size : m_Size + 1; }
		std::array<T, Size> m_Array;
		std::size_t m_Size = 0u;
	};
}
