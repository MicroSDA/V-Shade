#pragma once
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
	std::size_t GetCurrentTimeStamp()
	{
		return static_cast<size_t>(std::chrono::duration_cast<T>(std::chrono::system_clock::now().time_since_epoch()).count());
	}
	
	template<typename T>
	T GenerateRandomValue(T min, T max)
	{
		static_assert(std::is_scalar<T>::value, "T must be a scalar type");

		std::random_device rd; std::mt19937 gen(rd());
		
		if constexpr (std::is_integral<T>::value) 
		{
			std::uniform_int_distribution<T> dist(min, max); return dist(gen);
		}
		else if constexpr (std::is_floating_point<T>::value)
		{
			std::uniform_real_distribution<T> dist(min, max); return dist(gen);
		}
		else
		{
			return static_cast<T>(0);
		}
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

#define VECTOR_BASE_ITERATOR_HELPER(type, arr) \
public: \
	inline std::vector<type>::iterator begin() noexcept { return arr.begin(); };\
	inline std::vector<type>::iterator end() noexcept { return arr.end(); }; \
	inline std::vector<type>::const_iterator cbegin() const noexcept { return arr.begin(); }; \
	inline std::vector<type>::const_iterator cend() const noexcept { return arr.end(); }; \

#define SHADE_CAST_HELPER(type) \
public: \
    template<typename T> \
    SHADE_INLINE T& As() \
    { \
        static_assert(std::is_base_of<type, T>::value, "Is not base"); \
        return static_cast<T&>(*this); \
    } \
    template<typename T> \
    SHADE_INLINE const T& As() const \
    { \
        static_assert(std::is_base_of<type, T>::value, "Is not base"); \
        return static_cast<const T&>(*this); \
    }

