#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/utils/Utils.h>

namespace shade
{
	//Smart pointer
	template<typename T>
	class SharedPointer
	{
	public:
		// Default constructor to create an empty SharedPointer.
		SharedPointer() = default;
		// Virtual destructor to ensure proper cleanup of resources.
		virtual ~SharedPointer() = default;
	public:
		// @brief Create a new SharedPointer with the given arguments.
		// @tparam Args... Variadic template arguments for constructing the managed object.
		template<typename ...Args>
		SHADE_INLINE static SharedPointer<T> Create(Args&&... args)
		{
#ifdef SHADE_USE_POINTER_TIME_STAMP
			return SharedPointer(new T(std::forward<Args>(args)...), std::make_shared<std::size_t>(GetCurrentTimeStamp<std::chrono::microseconds>()));
#else
			return SharedPointer(new T(std::forward<Args>(args)...));
#endif // !SHADE_USE_POINTER_TIME_STAMP
		}
		// @brief Copy constructor to create a new SharedPointer from an existing one.
		SHADE_INLINE SharedPointer(const SharedPointer<T>& other)
		{
#ifdef SHADE_USE_POINTER_TIME_STAMP
			m_TimeStamp = other.m_TimeStamp;
#endif	// !SHADE_USE_POINTER_TIME_STAMP
			m_Pointer = other.m_Pointer;	
		}
		// @brief Copy assignment operator to assign the value of an existing SharedPointer.
		SHADE_INLINE SharedPointer& operator=(const SharedPointer<T>& other)
		{
			if (m_Pointer != other.m_Pointer)
			{
				m_Pointer = other.m_Pointer;
#ifdef SHADE_USE_POINTER_TIME_STAMP
				m_TimeStamp = other.m_TimeStamp;
#endif	// !SHADE_USE_POINTER_TIME_STAMP
			}
			return *this;
		}
		// @brief Copy constructor for related types (e.g., base or derived classes).
		// @tparam U Type that is related to T.
		template<typename U, std::enable_if_t<std::is_base_of_v<U, T> || std::is_base_of_v<T, U>, bool> = true>
		SHADE_INLINE SharedPointer(const SharedPointer<U>&other)
		{
			m_Pointer = std::static_pointer_cast<T>(other.m_Pointer);
#ifdef SHADE_USE_POINTER_TIME_STAMP
			m_TimeStamp = other.m_TimeStamp;
#endif	// !SHADE_USE_POINTER_TIME_STAMP
		}
		// @brief Copy assignment operator for related types.
		// @tparam U Type that is related to T.
		template<typename U, std::enable_if_t<std::is_base_of_v<U, T> || std::is_base_of_v<T, U>, bool> = true>
		SHADE_INLINE SharedPointer & operator=(const SharedPointer<U>&other)
		{
			if (m_Pointer != other.m_Pointer)
			{
				m_Pointer = std::static_pointer_cast<T>(other.m_Pointer);
#ifdef SHADE_USE_POINTER_TIME_STAMP
				m_TimeStamp = other.m_TimeStamp;
#endif	// !SHADE_USE_POINTER_TIME_STAMP
			}
			return *this;
		}
		// @brief Constructor for setting the SharedPointer to nullptr.
		SHADE_INLINE SharedPointer(std::nullptr_t)
		{
			m_Pointer = nullptr;
#ifdef SHADE_USE_POINTER_TIME_STAMP
			m_TimeStamp = nullptr;
#endif	// !SHADE_USE_POINTER_TIME_STAMP
		}
		// @brief Assignment operator for setting the SharedPointer to nullptr.
		SHADE_INLINE SharedPointer& operator =(std::nullptr_t)
		{
			m_Pointer = nullptr;
#ifdef SHADE_USE_POINTER_TIME_STAMP
			m_TimeStamp = nullptr;
#endif	// !SHADE_USE_POINTER_TIME_STAMP
			return *this;
		}
		// @brief Convert the SharedPointer instance into a boolean value indicating whether it is not nullptr.
		SHADE_INLINE operator bool() { return (m_Pointer != nullptr); }
		// @brief Convert the SharedPointer instance into a boolean value indicating whether it is not nullptr.
		SHADE_INLINE operator bool() const { return (m_Pointer != nullptr); }
		// @brief Check if this SharedPointer is equal to another SharedPointer by comparing their pointers.
		// @param other The other SharedPointer to compare with.
		// @return true if the pointers are equal, false otherwise.
		SHADE_INLINE bool operator== (const SharedPointer<T>& other) { return (m_Pointer == other.m_Pointer); }
		// @brief Check if this SharedPointer is not equal to another SharedPointer by comparing their pointers.
		// @param other The other SharedPointer to compare with.
		// @return true if the pointers are not equal, false otherwise.
		SHADE_INLINE bool operator!= (const SharedPointer<T>& other) { return (m_Pointer != other.m_Pointer); }
		// @brief Const version of the equality operator for comparing with another SharedPointer.
		// @param other The other SharedPointer to compare with.
		// @return true if the pointers are equal, false otherwise.
		SHADE_INLINE bool operator== (const SharedPointer<T>& other) const { return (m_Pointer == other.m_Pointer); }
		// @brief Const version of the inequality operator for comparing with another SharedPointer.
		// @param other The other SharedPointer to compare with.
		// @return true if the pointers are not equal, false otherwise.
		SHADE_INLINE bool operator!= (const SharedPointer<T>& other) const { return (m_Pointer != other.m_Pointer); }
		// @brief Equality operator for comparing with another SharedPointer of a related type.
		// @tparam U Type that is related to T.
		template<typename U>
		SHADE_INLINE bool operator== (const SharedPointer<U>& other) { return (m_Pointer.get() == other.m_Pointer.get()); }
		// @brief Inequality operator for comparing with another SharedPointer of a related type.
		// @tparam U Type that is related to T.
		template<typename U>
		SHADE_INLINE bool operator!= (const SharedPointer<U>& other) { return (m_Pointer.get() != other.m_Pointer.get()); }
		// @brief Const versions of equality and inequality operators.
		// @tparam U Type that is related to T.
		template<typename U>
		SHADE_INLINE bool operator== (const SharedPointer<U>& other) const { return (m_Pointer.get() == other.m_Pointer.get()); }
		template<typename U>
		SHADE_INLINE bool operator!= (const SharedPointer<U>& other) const { return (m_Pointer.get() != other.m_Pointer.get()); }
		// @brief Convert the SharedPointer instance into a std::size_t combined hash,
		// where the hash is derived from the instance's pointer and creation timestamp.
		SHADE_INLINE operator std::size_t() { return std::hash<SharedPointer<T>>{}(*this); }
		// @brief Convert the SharedPointer instance into a std::size_t combined hash,
		// where the hash is derived from the instance's pointer and creation timestamp.
		SHADE_INLINE operator std::size_t() const { return std::hash<SharedPointer<T>>{}(*this); }
		// @brief Arrow operator for dereferencing the pointer.
		SHADE_INLINE T* operator->() { return m_Pointer.get(); }
		// @brief Const version of the arrow operator.
		SHADE_INLINE const T* operator->() const { return m_Pointer.get(); }
		// @brief Dereference operator for accessing the pointed object.
		SHADE_INLINE T& operator*() { return *m_Pointer.get(); }
		// @brief Const version of the dereference operator.
		SHADE_INLINE const T& operator*() const { return *m_Pointer.get(); }
		// @brief Get the count of references to the shared pointer.
		SHADE_INLINE std::size_t GetCount() const { return m_Pointer.use_count(); }
		// @brief Providing direct link.
		SHADE_INLINE T& Get() { return  *m_Pointer.get(); }
		// @brief Providing direct link.
		SHADE_INLINE const T& Get() const { return  *m_Pointer.get(); }
		// @brief Providing direct pointer.
		SHADE_INLINE T* Raw() { return  m_Pointer.get(); }
		// @brief Providing direct pointer.
		SHADE_INLINE const T* Raw() const { return  m_Pointer.get(); }
	private:
		

#ifdef SHADE_USE_POINTER_TIME_STAMP
		// @brief Private constructor for SharedPointer<T>::Create().
		SharedPointer(T* ptr, const std::shared_ptr<std::size_t>& timeStamp)
			: m_Pointer(ptr), m_TimeStamp(timeStamp) {}
#else
		// @brief Private constructor for SharedPointer<T>::Create().
		SharedPointer(T* ptr)
			: m_Pointer(ptr) {}
#endif	// !SHADE_USE_POINTER_TIME_STAMP
	private:
		std::shared_ptr<T> m_Pointer;               // The shared pointer to the managed object.
#ifdef SHADE_USE_POINTER_TIME_STAMP
		std::shared_ptr<std::size_t> m_TimeStamp;   // A shared timestamp to track object lifetime.
#endif	// !SHADE_USE_POINTER_TIME_STAMP
	private:
		template<class U>
		friend class SharedPointer;

		// For converting from SharedPointer to Asset
		template<class T>
		friend class Asset;

		friend struct std::hash<SharedPointer<T>>;
	};

	// Base implementation of smart pointer.
	// For unique memeory. Instance will be deleted automatically so you don't need to carry about it!
	template<typename T>
	class UniquePointer
	{
	public:
		UniquePointer() :m_pInstance(nullptr) {}
		UniquePointer(T* instance) : m_pInstance(instance) {}
		~UniquePointer()
		{
			if (m_pInstance)
				delete m_pInstance;
		}
		// Move constructor
		UniquePointer(const UniquePointer<T>&& other) noexcept
		{
			m_pInstance = other.m_pInstance;
			other.Reset();
		}
		// Move(cast) constructor
		template<typename T2>
		UniquePointer(UniquePointer<T2>&& other) noexcept
		{
			m_pInstance = (T*)other.m_pInstance;
			other.m_pInstance = nullptr;
		}
		// Move operator 
		UniquePointer& operator=(UniquePointer<T>&& other) noexcept
		{
			if (this != &other)
			{
				m_pInstance = other.m_pInstance;
				other.m_pInstance = nullptr;
			}
			return *this;
		}
		// Move(cast) operator
		template<typename T2>
		UniquePointer& operator=(UniquePointer<T2>&& other) noexcept
		{
			m_pInstance = other.m_pInstance;
			other.m_pInstance = nullptr;
			return *this;
		}

		// Copy operator.
		UniquePointer& operator=(const UniquePointer<T>& other) = delete;
		// Copy operator.
		UniquePointer(const UniquePointer<T>& other) = delete;

		operator bool() { return (m_pInstance != nullptr); }
		operator const bool() const { return (m_pInstance != nullptr); }

		T* operator->() { return m_pInstance; }
		const T* operator->() const { return m_pInstance; }

		T& operator*() { return *m_pInstance; }
		const T& operator*() const { return *m_pInstance; }

		// Providing direct link.
		T& Get() { return  *m_pInstance; }
		// Providing direct link.
		const T& Get() const { return  &m_pInstance; }
		// Providing direct pointer.
		T* Raw() { return  m_pInstance; }
		// Providing direct pointer.
		const T* Raw() const { return  m_pInstance; }
		// Create instance.
		template<typename ...Args>
		static UniquePointer<T> Create(Args&&... args)
		{
			return std::move(UniquePointer<T>(new T(std::forward<Args>(args)...)));
		}
		// TODO: not sure about it 
		static UniquePointer<T> Create(T other)
		{
			return std::move(UniquePointer<T>(new T(std::move(other))));
		}
		// Unhold instance.
		void Reset()
		{
			if (m_pInstance)
			{
				delete m_pInstance; m_pInstance = nullptr;
			}
		}
	private:
		T* m_pInstance;
		template<class T2>
		friend class UniquePointer;
	};
}

// TODO:
template<typename T>
struct std::hash<shade::SharedPointer<T>>
{
	std::size_t operator()(const shade::SharedPointer<T>& instance) const noexcept
	{
#ifdef SHADE_USE_POINTER_TIME_STAMP
		std::size_t seed = hash<std::shared_ptr<T>>()(instance.m_Pointer);
		shade::HashCombine(seed, instance.m_TimeStamp);
#else
		std::size_t seed = hash<std::shared_ptr<T>>()(instance.m_Pointer);
#endif // !SHADE_USE_POINTER_TIME_STAMP
		return seed;
	}
};