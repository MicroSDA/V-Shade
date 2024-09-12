#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/threads/ThreadPool.h>
#include <shade/utils/Logger.h>
#include <shade/core/asset/Asset.h>
#include <shade/core/serializing/Serializer.h>
#include <shade/core/serializing/File.h>

namespace shade
{
	class SHADE_API AssetManager
	{
	#ifndef SHADE_ASSET_META_FILE_PATH
		#define SHADE_ASSET_META_FILE_PATH "./resources/ASSET_META.bin"
	#endif // !SHADE_META_FILE_PATH

	public:
		// Contain all assets which were loaded.
		using AssetMap = std::unordered_map<std::string, Asset<BaseAsset>>;
		// Result of loading thread.
		using TaskResult = std::shared_ptr<std::future<std::pair<Asset<BaseAsset>, std::exception_ptr>>>;
		// Callback for delivering asset to reciver.
		using DeliveryCallback = std::function<void(Asset<BaseAsset>&)>;
		// Contains assets meta data.
		using AssetsDataList = std::unordered_map<std::string, SharedPointer<AssetData>>;
		using AssetsDataRelink = std::unordered_set<std::string>;
	public:
		// Initialize Assets data from file in folder.
		static void Initialize(const std::string& filePath = SHADE_ASSET_META_FILE_PATH);
		// Initialize Assets data from stream.
		static void Initialize(std::istream& stream);
		// Save Assets data to file.
		static void Save(const std::string& filePath = SHADE_ASSET_META_FILE_PATH);
		// Save Assets data to stream.
		static void Save(std::ostream& stream);
		// TIP: Not SharedPointer ?
		static void AddNewAssetData(const SharedPointer<AssetData>& data);

		// Use for loading new asset into memory by specific asset id.
		// If you want to kepp an asset in memory even if all contributors was deleted, 
		// use BaseAsset::Lifetime::KeepAlive or BaseAsset::Lifetime::DontKeepAlive if you don't want to keep it in memeory.
		// All assets that were held with BaseAsset::Lifetime::DontKeepAlive flag will be removed automatically ! 
		template<typename T, BaseAsset::InstantiationBehaviour behaviour = BaseAsset::InstantiationBehaviour::Aynchronous , typename ...Args /*, std::enable_if_t<std::is_base_of_v<Asset<T>, T>, bool> = true*/>
		static auto GetAsset(const std::string& id, AssetMeta::Category category, BaseAsset::LifeTime lifeTime, DeliveryCallback callback, Args&& ...args);

		// Function usues for delivery assets which where loaded.
		static void DeliveryAssets();

		static AssetsDataList& GetAssetDataList(AssetMeta::Category category);
		static SharedPointer<AssetData> GetAssetData(AssetMeta::Category category, const std::string& id);

		static void ShutDown();
	private:
		struct Task
		{
			enum class TaskStatus
			{
				NotReady,
				Ready
			};
			Task() = default;
			Task(const std::string& id, TaskResult& result, DeliveryCallback deliveryCallback) :
				Id(id), Result(std::move(result)), DeliveryCallbacks(1, deliveryCallback) {}

			std::string Id;
			TaskResult Result;
			std::vector<DeliveryCallback> DeliveryCallbacks;
			TaskStatus Status = TaskStatus::NotReady;
		};
		using TaskQueue = std::unordered_map<std::string, AssetManager::Task>;
	private:
		static std::array<AssetMap, AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM>  m_sAssets;
		static std::array<TaskQueue, AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM>  m_sTaskQueue;
		static std::array<AssetsDataList, AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM> m_sAssetsDataList;
		static std::array<AssetsDataRelink, AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM> m_sAssetDataRelink;
		static std::array<std::recursive_mutex, AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM> m_sMutexs;
		static thread::ThreadPool m_sThreadPool;
	private:
		static void ReleaseMe(const std::string& id, AssetMeta::Category category);
		friend class BaseAsset;
	private:
		static void ReadAssetDataRecursively(SharedPointer<AssetData>& data);
		static void LinkAssetDataRecursivly(const std::string& id, SharedPointer<AssetData>& data);
		static void Delivery(AssetMeta::Category category);
		template<typename T, BaseAsset::InstantiationBehaviour behaviour, typename ...Args>
		static auto LoadNew(const SharedPointer<AssetData>& assetData, AssetMeta::Category category, BaseAsset::LifeTime lifeTime, DeliveryCallback callback, Args&& ...args);
	};

	template<typename T, shade::BaseAsset::InstantiationBehaviour behaviour, typename ...Args/*, std::enable_if_t<std::is_base_of_v<Asset<T>, T>, bool>*/>
	inline auto AssetManager::GetAsset(const std::string& id, AssetMeta::Category category, BaseAsset::LifeTime lifeTime, DeliveryCallback callback, Args&& ...args)
	{
		// Get the asset metadata for the given category and id
		auto assetData = GetAssetData(category, id);
		// Checking if assetData is not null and its type id matches with static type id of T
		if (assetData)
		{
			if (assetData->GetType() == T::GetAssetStaticType())
			{
				{
					// Lock the mutex corresponding to the asset's category
					std::lock_guard<std::recursive_mutex> lock(m_sMutexs[category]);

					// Check if the asset is already loaded
					AssetMap::const_iterator search = m_sAssets[category].find(id);
					if (search != m_sAssets[category].end())
					{
						// If asset already exists, create a new task to deliver it
						if constexpr (behaviour == BaseAsset::InstantiationBehaviour::Synchronous)
						{
							callback(m_sAssets[category].at(id));
						}
						else
						{
							// If asset already exists, create a new task to deliver it
							std::promise<std::pair<Asset<BaseAsset>, std::exception_ptr>>  promise;
							TaskResult result = std::make_shared<std::future<std::pair<Asset<BaseAsset>, std::exception_ptr>>>(promise.get_future());

							promise.set_value({ (*search).second, nullptr });
							m_sTaskQueue[category].emplace(id, std::move(Task(id, result, callback)));
						}
					}
					else
					{
						// If asset doesn't exist, check if there is already a task to load it
						TaskQueue::iterator task = m_sTaskQueue[category].find(id);
						if (task != m_sTaskQueue[category].end())
						{
							if constexpr (behaviour == BaseAsset::InstantiationBehaviour::Synchronous)
							{
								// WARNING: In case task was created as async and exeption was occured where it has to be handled ?
								auto asset = task->second.Result->get().first;
								callback(asset);
							}
							else
								task->second.DeliveryCallbacks.emplace_back(callback);
						}
						else
						{
							// If is a new asset, create a new task to load it
							LoadNew<T, behaviour>(assetData, category, lifeTime, callback, std::forward<Args>(args)...);
						}
					}
				}
			}
			else
			{
				SHADE_CORE_WARNING("Asset<{0}> type = '{1}' is not match with asset data type = '{2}' !", typeid(T).name(), AssetMeta::GetTypeAsString(T::GetAssetStaticType()), AssetMeta::GetTypeAsString(assetData->GetType()));
			}
		}
		else
		{
			SHADE_CORE_WARNING("Asset id = '{0}' doesn't exist in asset data list.", id);
		}
	}
	// Template function for loading an asset of type T from a category of assets through its id
	template<typename T, BaseAsset::InstantiationBehaviour behaviour, typename ...Args>
	inline auto AssetManager::LoadNew(const SharedPointer<AssetData>& assetData, AssetMeta::Category category, BaseAsset::LifeTime lifeTime, DeliveryCallback callback, Args&&...args)
	{
		if constexpr (behaviour == BaseAsset::InstantiationBehaviour::Synchronous)
		{
			// If is a new asset, create a result
			try
			{
				auto asset = Asset<BaseAsset>(Asset<T>::Create(assetData, lifeTime, behaviour, std::forward<Args>(std::decay_t<Args>(args))...));
				m_sAssets[category].emplace(assetData->GetId(), asset);
				callback(asset);

				asset->InitializeAsset();
			}
			catch (std::exception& exception)
			{
				SHADE_CORE_ERROR("Asset load exception: {0}", exception.what());
			}
		}
		else
		{
			TaskResult result = std::make_shared<std::future<std::pair<Asset<BaseAsset>, std::exception_ptr>>>(
				m_sThreadPool.Emplace([=]()
					{
						// Try to create a new asset of type T with the given asset data and life time
						try
						{
							auto asset = Asset<BaseAsset>(Asset<T>::Create(assetData, lifeTime, behaviour, std::forward<Args>(std::decay_t<Args>(args))...));
							return std::make_pair(Asset<BaseAsset>(asset), std::current_exception());
						}
						// If there is any exception during the asset creation, return an empty Asset<BaseAsset> and the current exception
						catch (...)
						{
							return std::make_pair(Asset<BaseAsset>(), std::current_exception());
						}
					}));

			m_sTaskQueue[category].emplace(assetData->GetId(), std::move(Task(assetData->GetId(), result, callback)));
		}
	}
}
