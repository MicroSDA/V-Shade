#pragma once
#include <shade/core/asset/BaseAsset.h>
#include <shade/utils/Utils.h>

namespace shade
{
	/* Asset wrapper which implement smart pointer behavior. */
	template<typename T>
	class Asset
	{
	public:
		// Default constructor to create an empty Asset.
		Asset() = default;
		// Asset destructor.
		~Asset() // TIP: Virtual was removed need to test !!
		{ 
			// In case we have only two alive instances, one here and one in AssetManager 
			// we need to run life time managment based on BaseAsset::LifeTime flag
			if(GetCount() == 2) LifeTimeManagment();
		};
	public:
		// @brief Create a new Asset with the given arguments.
		// @tparam Args... Variadic template arguments for constructing the managed object.
		template<typename... Args, typename = std::enable_if<!std::is_same_v<decltype(&T::Create), decltype(&T::Create)>, bool>>
		static Asset<T> Create(Args&& ...args)
		{
#ifdef SHADE_USE_POINTER_TIME_STAMP
			//static_assert(std::is_same<decltype(&T::Create), decltype(&T::Create)>::value, "T::Create is not a member function.");
			return Asset<T>(T::Create(std::forward<Args>(args)...), std::make_shared<std::size_t>(GetCurrentTimeStamp<std::chrono::microseconds>()));
#else
			//static_assert(std::is_same<decltype(&T::Create), decltype(&T::Create)>::value, "T::Create is not a member function.");
			return Asset<T>(T::Create(std::forward<Args>(args)...));
#endif	//!SHADE_USE_POINTER_TIME_STAMP
		}
		// @brief Copy constructor to create a new Asset from an existing one.
		Asset(const Asset<T>& other)
		{
			m_Pointer = other.m_Pointer;
#ifdef SHADE_USE_POINTER_TIME_STAMP
			m_TimeStamp = other.m_TimeStamp;
#endif //!SHADE_USE_POINTER_TIME_STAMP
		}
		// @brief Copy assignment operator to assign the value of an existing Asset.
		Asset& operator=(const Asset<T>& other)
		{
			if (m_Pointer != other.m_Pointer)
			{
				m_Pointer = other.m_Pointer;
#ifdef SHADE_USE_POINTER_TIME_STAMP
				m_TimeStamp = other.m_TimeStamp;
#endif //!SHADE_USE_POINTER_TIME_STAMP
			}
			return *this;
		}
		// @brief Copy constructor for related types (e.g., base or derived classes).
		// @tparam U Type that is related to T.
		template<typename U, std::enable_if_t<std::is_base_of_v<U, T> || std::is_base_of_v<T, U>, bool> = true>
		Asset(const Asset<U>&other)
		{
			m_Pointer = std::static_pointer_cast<T>(other.m_Pointer);
#ifdef SHADE_USE_POINTER_TIME_STAMP
			m_TimeStamp = other.m_TimeStamp;
#endif //!SHADE_USE_POINTER_TIME_STAMP
		}
		// @brief Copy assignment operator for related types.
		// @tparam U Type that is related to T.
		template<typename U, std::enable_if_t<std::is_base_of_v<U, T> || std::is_base_of_v<T, U>, bool> = true>
		Asset & operator=(const Asset<U>&other)
		{
			if (m_Pointer != other.m_Pointer)
			{
				m_Pointer = std::static_pointer_cast<T>(other.m_Pointer);
#ifdef SHADE_USE_POINTER_TIME_STAMP
				m_TimeStamp = other.m_TimeStamp;
#endif //!SHADE_USE_POINTER_TIME_STAMP
			}
			return *this;
		}
		// @brief Constructor for setting the Asset to nullptr.
		Asset(std::nullptr_t)
		{
			m_Pointer = nullptr;
#ifdef SHADE_USE_POINTER_TIME_STAMP
			m_TimeStamp = nullptr;
#endif //!SHADE_USE_POINTER_TIME_STAMP
		}
		// @brief Assignment operator for setting the Asset to nullptr.
		Asset& operator =(std::nullptr_t)
		{
			m_Pointer = nullptr;
#ifdef SHADE_USE_POINTER_TIME_STAMP
			m_TimeStamp = nullptr;
#endif //!SHADE_USE_POINTER_TIME_STAMP
			return *this;
		}
		Asset(const SharedPointer<T>& other)
		{
			m_Pointer = other.m_Pointer;
#ifdef SHADE_USE_POINTER_TIME_STAMP
			m_TimeStamp = other.m_TimeStamp;
#endif
		}
		Asset& operator=(const SharedPointer<T>& other)
		{
			if (m_Pointer != other.m_Pointer)
			{
				m_Pointer = other.m_Pointer;
#ifdef SHADE_USE_POINTER_TIME_STAMP
				m_TimeStamp = other.m_TimeStamp;
#endif //!SHADE_USE_POINTER_TIME_STAMP
			}
			return *this;
		}
		// @brief Convert the Asset instance into a boolean value indicating whether it is not nullptr.
		operator bool() { return (m_Pointer != nullptr); }
		// @brief Convert the Asset instance into a boolean value indicating whether it is not nullptr.
		operator bool() const { return (m_Pointer != nullptr); }
		// @brief Check if this Asset is equal to another Asset by comparing their pointers.
		// @param other The other Asset to compare with.
		// @return true if the pointers are equal, false otherwise.
		bool operator== (const Asset<T>& other) { return (m_Pointer == other.m_Pointer); }
		// @brief Check if this Asset is not equal to another Asset by comparing their pointers.
		// @param other The other Asset to compare with.
		// @return true if the pointers are not equal, false otherwise.
		bool operator!= (const Asset<T>& other) { return (m_Pointer != other.m_Pointer); }
		// @brief Const version of the equality operator for comparing with another Asset.
		// @param other The other Asset to compare with.
		// @return true if the pointers are equal, false otherwise.
		bool operator== (const Asset<T>& other) const { return (m_Pointer == other.m_Pointer); }
		// @brief Const version of the inequality operator for comparing with another Asset.
		// @param other The other Asset to compare with.
		// @return true if the pointers are not equal, false otherwise.
		bool operator!= (const Asset<T>& other) const { return (m_Pointer != other.m_Pointer); }
		// @brief Equality operator for comparing with another Asset of a related type.
		// @tparam U Type that is related to T.
		template<typename U>
		bool operator== (const Asset<U>& other) { return (m_Pointer.get() == other.m_Pointer.get()); }
		// @brief Inequality operator for comparing with another Asset of a related type.
		// @tparam U Type that is related to T.
		template<typename U>
		bool operator!= (const Asset<U>& other) { return (m_Pointer.get() != other.m_Pointer.get()); }
		// @brief Const versions of equality and inequality operators.
		// @tparam U Type that is related to T.
		template<typename U>
		bool operator== (const Asset<U>& other) const { return (m_Pointer.get() == other.m_Pointer.get()); }
		template<typename U>
		bool operator!= (const Asset<U>& other) const { return (m_Pointer.get() != other.m_Pointer.get()); }
		// @brief Convert the Asset instance into a std::size_t combined hash,
		// where the hash is derived from the instance's pointer and creation timestamp.
		operator std::size_t() { return std::hash<Asset<T>>{}(*this); }
		// @brief Convert the Asset instance into a std::size_t combined hash,
		// where the hash is derived from the instance's pointer and creation timestamp.
		operator std::size_t() const { return std::hash<Asset<T>>{}(*this); }
		// @brief Arrow operator for dereferencing the pointer.
		T* operator->() { return m_Pointer.get(); }
		// @brief Const version of the arrow operator.
		const T* operator->() const { return m_Pointer.get(); }
		// @brief Dereference operator for accessing the pointed object.
		T& operator*() { return *m_Pointer.get(); }
		// @brief Const version of the dereference operator.
		const T& operator*() const { return *m_Pointer.get(); }
		// @brief Get the count of references to the shared pointer.
		std::size_t GetCount() const { return m_Pointer.use_count(); }
		// @brief Providing direct link.
		T& Get() { return  *m_Pointer.get(); }
		// @brief Providing direct link.
		const T& Get() const { return  *m_Pointer.get(); }
		// @brief Providing direct pointer.
		T* Raw() { return  m_Pointer.get(); }
		// @brief Providing direct pointer.
		const T* Raw() const { return  m_Pointer.get(); }
	private:
	

#ifdef SHADE_USE_POINTER_TIME_STAMP
		// @brief Private constructor for Asset<T>::Create().
		Asset(T* ptr, const std::shared_ptr<std::size_t>& timeStamp)
			: m_Pointer(ptr), m_TimeStamp(timeStamp) {}
#else 
		// @brief Private constructor for Asset<T>::Create().
		Asset(T* ptr, const std::shared_ptr<std::size_t>& timeStamp)
			: m_Pointer(ptr){}
#endif  //!SHADE_USE_POINTER_TIME_STAMP
	private:
		std::shared_ptr<T> m_Pointer;               // The shared pointer to the managed object.
#ifdef SHADE_USE_POINTER_TIME_STAMP
		std::shared_ptr<std::size_t> m_TimeStamp;   // A shared timestamp to track object lifetime.
#endif //!SHADE_USE_POINTER_TIME_STAMP
	private:
		void LifeTimeManagment() 
		{ 
			// In case we have Asset<Drawable> compiler cannot staticly cast this pointer into BaseAsset* so we have to use dynamic_cast insted
			// See Renderer::SubmitStaticMesh
			BaseAsset* base = dynamic_cast<BaseAsset*>(m_Pointer.get());
			if (base) base->LifeTimeManagment();
		}
	private:
		template<class U>
		friend class Asset;

		friend struct std::hash<Asset<T>>;
	};
}

namespace std
{
	template<typename T>
	struct hash<shade::Asset<T>>
	{
		std::size_t operator()(const shade::Asset<T>& instance) const noexcept
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
}
