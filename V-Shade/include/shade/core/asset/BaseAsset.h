#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/serializing/Serializer.h>
#include <shade/core/memory/Memory.h>
#include <shade/utils/Logger.h>

namespace shade
{
	/*template<typename T>
	class Asset;*/

	class SHADE_API AssetMeta
	{
	public:
		enum Category : std::uint32_t
		{
			None = 0,
			Primary,	// Represents asset which depends on raw assets.
			Secondary,	// Represents raw asset which exist on disk.
			Blueprint,
			PrimaryReference,
			SecondaryReference,
			ASSET_CATEGORY_MAX_ENUM = 4,
		};
		static std::string GetCategoryAsString(Category category);
		static Category GetCategoryFromString(const std::string& category);

		enum Type : std::uint32_t
		{
			Asset,
			Model,
			Mesh,
			Material,
			Texture,
			Animation,
			Skeleton,
			CollisionShapes,
			Sound,
			AnimationGraph,
			// Insert new here
			Other,
			ASSET_TYPE_MAX_ENUM
		};

		// Here we can keep texture's types and soo on
		enum SubType : std::uint32_t
		{
			ImageDiffuse // and so on
		};

		static std::string GetTypeAsString(Type type);
		static Type GetTypeFromString(const std::string& type);
	};

	class SHADE_API AssetData
	{
	public:
		AssetData();
		AssetData(const std::string& id, AssetMeta::Category category = AssetMeta::Category::None, AssetMeta::Type type = AssetMeta::Type::Asset);
		virtual ~AssetData() = default;

		template<typename T>
		T GetAttribute(const std::string& name) const;
		template<typename T>
		void SetAttribute(const std::string& name, const T& attribute);
		template<typename T>
		void SetAttribute(const std::string& name, const T* attribute);

		void SetCategory(AssetMeta::Category category);
		AssetMeta::Category GetCategory() const;

		void SetType(AssetMeta::Type type);
		AssetMeta::Type GetType() const;

		void SetId(const std::string& id);
		const std::string& GetId() const;

		void SetReference(SharedPointer<AssetData>& data);
		SharedPointer<AssetData>& GetReference();

		void AddDependency(SharedPointer<AssetData>& dependency);
		const std::vector<SharedPointer<AssetData>>& GetDependencies() const;
		std::vector<SharedPointer<AssetData>>& GetDependencies();

		const std::unordered_map<std::string, std::string>& GetAttributes() const;
		std::unordered_map<std::string, std::string>& GetAttributes();

	private:
		void Serialize(std::ostream& stream) const;
		void Deserialize(std::istream& stream);
	private:
		// MAKE DEFAULT CONSTRUCTOR !
		AssetMeta::Category m_Category;
		AssetMeta::Type m_Type;
		std::string m_Id;
		std::string m_SecondaryReferenceId = "NULL"; // Used Only during deserialize !
		std::unordered_map<std::string, std::string> m_Attributes;
		std::vector<SharedPointer<AssetData>> m_Dependencies;
		SharedPointer<AssetData> m_SecondaryReference; // In case that primary asset linked to some secondary asset.

		friend class serialize::Serializer;
		friend class AssetManager;
	};
	/* Get attribute as string.*/
	template<>
	SHADE_INLINE std::string AssetData::GetAttribute(const std::string& name) const
	{
		auto search = m_Attributes.find(name);
		if (search != m_Attributes.end())
			return search->second;
		else
			return std::string(); // throw std::exception(std::invalid_argument(std::format("Attribute = '{}' has not been found!", name)));

	}
	/* Set attribute as string.*/
	template<>
	SHADE_INLINE void AssetData::SetAttribute(const std::string& name, const std::string& attribute)
	{
		m_Attributes[name] = attribute;
	}
	/* Set attribute as string.*/
	template<>
	SHADE_INLINE void AssetData::SetAttribute(const std::string& name, const char* attribute)
	{
		m_Attributes[name] = attribute;
	}
	/* Get attribute as std::uint32_t.*/
	template<>
	SHADE_INLINE std::uint32_t AssetData::GetAttribute(const std::string& name) const
	{
		auto search = m_Attributes.find(name);
		if (search != m_Attributes.end())
			return std::stoul(search->second);
		else
			throw std::exception(std::invalid_argument(std::format("Attribute = '{}' has not been found!", name)));
	}
	/* Set attribute as std::uint32_t.*/
	template<>
	SHADE_INLINE void AssetData::SetAttribute(const std::string& name, const std::uint32_t& attribute)
	{
		m_Attributes[name] = std::to_string(attribute);
	}
	/* Get attribute as std::uint64_t.*/
	template<>
	SHADE_INLINE std::uint64_t AssetData::GetAttribute(const std::string& name) const
	{
		auto search = m_Attributes.find(name);
		if (search != m_Attributes.end())
			return std::stoull(search->second);
		else
			throw std::exception(std::invalid_argument(std::format("Attribute = '{}' has not been found!", name)));
	}
	/* Set attribute as std::uint64_t.*/
	template<>
	SHADE_INLINE void AssetData::SetAttribute(const std::string& name, const std::uint64_t& attribute)
	{
		m_Attributes[name] = std::to_string(attribute);
	}
	/* Get attribute as float.*/
	template<>
	SHADE_INLINE float AssetData::GetAttribute(const std::string& name) const
	{
		auto search = m_Attributes.find(name);
		if (search != m_Attributes.end())
			return std::stof(search->second);
		else
			throw std::exception(std::invalid_argument(std::format("Attribute = '{}' has not been found!", name)));
	}
	/* Set attribute as float.*/
	template<>
	SHADE_INLINE void AssetData::SetAttribute(const std::string& name, const float& attribute)
	{
		m_Attributes[name] = std::to_string(attribute);
	}
	/* Get attribute as char.*/
	template<>
	SHADE_INLINE char AssetData::GetAttribute(const std::string& name) const
	{
		auto search = m_Attributes.find(name);
		if (search != m_Attributes.end())
			return search->second[0];
		else
			throw std::exception(std::invalid_argument(std::format("Attribute = '{}' has not been found!", name)));
	}
	/* Set attribute as char.*/
	template<>
	SHADE_INLINE void AssetData::SetAttribute(const std::string& name, const char& attribute)
	{
		m_Attributes[name] = attribute;
	}
	/* Get attribute as double.*/
	template<>
	SHADE_INLINE double AssetData::GetAttribute(const std::string& name) const
	{
		auto search = m_Attributes.find(name);
		if (search != m_Attributes.end())
			return std::stod(search->second);
		else
			throw std::exception(std::invalid_argument(std::format("Attribute = '{}' has not been found!", name)));
	}
	/* Set attribute as double.*/
	template<>
	SHADE_INLINE void AssetData::SetAttribute(const std::string& name, const double& attribute)
	{
		m_Attributes[name] = std::to_string(attribute);
	}
	/* Get attribute as std::uint16_t.*/
	template<>
	SHADE_INLINE std::uint16_t AssetData::GetAttribute(const std::string& name) const
	{
		// TIP: No speciasl checking if value outside of std::uint16_t range !
		auto search = m_Attributes.find(name);
		if (search != m_Attributes.end())
			return static_cast<std::uint16_t>(std::stoi(search->second));
		else
			throw std::exception(std::invalid_argument(std::format("Attribute = '{}' has not been found!", name)));
	}
	/* Set attribute as std::uint16_t.*/
	template<>
	SHADE_INLINE void AssetData::SetAttribute(const std::string& name, const std::uint16_t& attribute)
	{
		m_Attributes[name] = std::to_string(attribute);
	}
	/* Get attribute as std::uint8_t.*/
	template<>
	SHADE_INLINE std::uint8_t AssetData::GetAttribute(const std::string& name) const
	{
		// TIP: No speciasl checking if value outside of std::uint8_t range !
		auto search = m_Attributes.find(name);
		if (search != m_Attributes.end())
			return static_cast<std::uint8_t>(std::stoi(search->second));
		else
			throw std::exception(std::invalid_argument(std::format("Attribute = '{}' has not been found!", name)));
	}
	/* Set attribute as std::uint8_t.*/
	template<>
	SHADE_INLINE void AssetData::SetAttribute(const std::string& name, const std::uint8_t& attribute)
	{
		m_Attributes[name] = std::to_string(attribute);
	}

	class SHADE_API BaseAsset
	{
	public:
		// Represent lifetime behavior for Asset manager
		enum class LifeTime
		{
			KeepAlive, // Keep in memory one instance even if there are no active contributors.
			DontKeepAlive // Remove from memory as soon as posible when there are no active contributors.
		};
		enum class InstantiationBehaviour
		{
			Synchronous,
			Aynchronous
		};
	public:
		BaseAsset(SharedPointer<AssetData> assetData = SharedPointer<AssetData>(), LifeTime lifeTime = LifeTime::DontKeepAlive, InstantiationBehaviour behaviour = InstantiationBehaviour::Aynchronous);
		virtual ~BaseAsset();
		LifeTime GetLifeTime();

		void SetAssetData(const SharedPointer<AssetData>& data);
		const SharedPointer<AssetData>& GetAssetData() const;
		SharedPointer<AssetData>& GetAssetData();
		// Define static method GetAssetStaticType which returns AssetMeta::Type::Undefined
		static AssetMeta::Type GetAssetStaticType();
		// Define virtual method GetAssetType which returns the result of calling GetAssetStaticType
		// This method is virtual, which means that it can be overridden by a subclass
		// This implementation simply calls the static method with the same name
		virtual AssetMeta::Type GetAssetType() const;

	private:
		LifeTime m_LifeTime;
		InstantiationBehaviour m_Behaviour;
		SharedPointer<AssetData> m_AssetData;
		bool m_HasBeenInitialized = false;
		void InitializeAsset();
		void LifeTimeManagment();

		template<typename T>
		friend class Asset;

		friend class AssetManager;
	};


#define ASSET_DEFINITION_CREATE_0(atype) SHADE_INLINE static atype* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) { return new atype(assetData, lifeTime, behaviour); }
#define ASSET_DEFINITION_CREATE_1(atype, type, name) SHADE_INLINE static atype* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour, type name) { return new atype(assetData, lifeTime, behaviour, name); }
#define ASSET_DEFINITION_CREATE_2(atype, type1, name1, type2, name2) SHADE_INLINE static atype* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour, type1 name1, type2, name2) { return new atype(assetData, lifeTime, behaviour, name1, name2); }
#define ASSET_DEFINITION_CREATE_3(atype, type1, name1, type2, name2, type3, name3) SHADE_INLINE static atype* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour, type1 name1, type2, name2, atype3, name3) { return new type(assetData, lifeTime, behaviour, name1, name2, name3); }

#define ASSET_INHERITANCE(atype) public shade::BaseAsset //,public shade::Asset<type>
#define ASSET_STATIC_TYPE_HELPER(atype) \
		public: \
			static inline AssetMeta::Type GetAssetStaticType() { return AssetMeta::Type::##atype; } \
			virtual inline AssetMeta::Type GetAssetType() const override { return GetAssetStaticType(); } \

#define ASSET_DEFINITION_HELPER(atype) \
		public: \
			ASSET_STATIC_TYPE_HELPER(atype) \
			static inline SharedPointer<##atype> CreateEXP() { return SharedPointer<##atype>::Create(); }\
		private:  \
			atype() = default; \
			ASSET_DEFINITION_CREATE_0(atype) \
			friend class SharedPointer<atype>;\
			friend class Asset<atype>;

#define ASSET_DEFINITION_HELPER_1(atype, type1, name1) \
		public: \
			ASSET_STATIC_TYPE_HELPER(atype) \
			static inline SharedPointer<##atype> CreateEXP() { return SharedPointer<##atype>::Create(); }\
		private:  \
			atype() = default; \
			ASSET_DEFINITION_CREATE_1(atype, type1, name1)  \
			friend class SharedPointer<atype>;\
			friend class Asset<atype>;

#define ASSET_DEFINITION_HELPER_2(atype, type1, name1, type2, name2) \
		public: \
			ASSET_STATIC_TYPE_HELPER(atype) \
			static inline SharedPointer<##atype> CreateEXP() { return SharedPointer<##atype>::Create(); }\
		private:  \
			atype() = default; \
			ASSET_DEFINITION_CREATE_2(atype, type1, name1, type2, name2)  \
			friend class SharedPointer<atype>;\
			friend class Asset<atype>;

#define ASSET_DEFINITION_HELPER_3(atype, type1, name1, type2, name2, type3, name3) \
		public: \
			ASSET_STATIC_TYPE_HELPER(atype, atype) \
			static inline SharedPointer<##atype> CreateEXP() { return SharedPointer<##atype>::Create(); }\
		private:  \
			atype() = default; \
			ASSET_DEFINITION_CREATE_3(atype, type1, name1, type2, name2)  \
			friend class SharedPointer<atype>;\
			friend class Asset<atype>;

	
	/* Serialize AssetData.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const SharedPointer<AssetData>& assetData)
	{
		assetData->Serialize(stream);
	}
	/* Deserialize AssetData.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, SharedPointer<AssetData>& assetData)
	{
		assetData->Deserialize(stream);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* Serialize std::vector<AssetData>.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const std::vector<SharedPointer<AssetData>>& array)
	{
		std::uint32_t size = static_cast<std::uint32_t>(array.size());
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		// Write size first.
		stream.write(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
		// If size is more then 0 need to write data.
		if (size)
		{
			for (const auto& asset : array)
			{
				Serializer::Serialize(stream, asset);
			}
		}
	}
	/* Deserialize std::vector<AssetData>.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, std::vector<SharedPointer<AssetData>>& array)
	{
		std::uint32_t size = 0;
		// Read size first.
		stream.read(reinterpret_cast<char*>(&size), sizeof(std::uint32_t));
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		// If size is more then 0 need to read data.
		if (size)
		{
			array.resize(size);
			for (auto& asset : array)
			{
				asset = SharedPointer<AssetData>::Create();
				Serializer::Deserialize(stream, asset);
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
