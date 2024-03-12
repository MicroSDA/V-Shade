#include "shade_pch.h"
#include "AssetManager.h"

std::array<shade::AssetManager::AssetMap, shade::AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM>  shade::AssetManager::m_sAssets;
std::array<shade::AssetManager::TaskQueue, shade::AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM>  shade::AssetManager::m_sTaskQueue;
std::array<shade::AssetManager::AssetsDataList, shade::AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM> shade::AssetManager::m_sAssetsDataList;
std::array<shade::AssetManager::AssetsDataRelink, shade::AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM>  shade::AssetManager::m_sAssetDataRelink;
std::array<std::recursive_mutex, shade::AssetMeta::Category::ASSET_CATEGORY_MAX_ENUM> shade::AssetManager::m_sMutexs;
shade::thread::ThreadPool shade::AssetManager::m_sThreadPool;

// TODO: Try to use m_SecondaryReferenceId != '\0' insetad of m_SecondaryReferenceId != "NULL"

void shade::AssetManager::Delivery(AssetMeta::Category category)
{
	std::unique_lock<std::recursive_mutex> lock(m_sMutexs[category], std::defer_lock);

	if (lock.try_lock())
	{
		if (m_sTaskQueue[category].empty())
			return;

		TaskQueue::iterator search = m_sTaskQueue[category].begin();
		while (search != m_sTaskQueue[category].end())
		{
			auto& task = search->second;
			if (task.Result->_Is_ready())
			{
				try
				{
					auto ressult = search->second.Result->get();
					auto asset = ressult.first;
					auto exception = ressult.second;

					if (exception != nullptr)
						std::rethrow_exception(exception);

					m_sAssets[category].emplace(task.Id, asset);

					for (auto& delivery : task.DeliveryCallbacks)
						delivery(asset);

					asset->InitializeAsset();

					search = m_sTaskQueue[category].erase(search);
				}
				catch (std::exception& exception)
				{
					SHADE_CORE_ERROR("Asset load exception: {0}", exception.what());
					search = m_sTaskQueue[category].erase(search);
				}
			}
			else
			{
				search++;
			}
		}
	}
}

void shade::AssetManager::Initialize(const std::string& filePath)
{
	File file(filePath, File::In, "@s_m_asset", File::VERSION(0, 0, 1));
	if (file.IsOpen())
	{
		Initialize(file.GetStream());
		file.CloseFile();
	}

	//if (std::filesystem::exists(folderPath))
	//{
	//	for (const auto& entry : std::filesystem::directory_iterator(folderPath))
	//	{
	//		// Get file path and inverse '\' to '/'.
	//		const std::string filePath = entry.path().generic_string();

	//		if (entry.path().extension().string() == std::string(".bin"))
	//		{
	//			const std::string filename = entry.path().filename().replace_extension().string();

	//			std::ifstream file(filePath, std::ios::binary);
	//			if (!file.is_open())
	//				SHADE_CORE_WARNING("Failed to initialize AssetManager, wrong path = {0}", filePath)
	//			else
	//			{
	//				Initialize(file);
	//				file.close();
	//			}
	//				
	//		}
	//	}
	//}
	//else
	//	SHADE_CORE_WARNING("Failed to initialize AssetManager, wrong path = {0}", folderPath);
}

void shade::AssetManager::AddNewAssetData(const SharedPointer<AssetData>& data)
{
	m_sAssetsDataList[data->GetCategory()].insert({data->GetId(), data });
	for (auto& [id, asset] : m_sAssetsDataList[AssetMeta::Category::Primary])
		LinkAssetDataRecursivly(id, asset);
}

void shade::AssetManager::Initialize(std::istream& stream)
{
	for (auto& category : m_sAssetDataRelink)
		category.clear();

	while (stream.peek() != EOF)
	{
		SharedPointer<AssetData> asset = SharedPointer<AssetData>::Create();
		Serializer::Deserialize(stream, asset);
		ReadAssetDataRecursively(asset);
	}


	/*SharedPointer<AssetData> materialSeconday = SharedPointer<AssetData>::Create();
	materialSeconday->SetId("Cube.Material_2");
	materialSeconday->SetType(AssetMeta::Type::Material);
	materialSeconday->SetCategory(AssetMeta::Category::Secondary);
	materialSeconday->SetAttribute("Path", "./resources/assets/cube/Cube.Material.s_mat");

	SharedPointer<AssetData> materialPirmary = SharedPointer<AssetData>::Create();
	materialPirmary->SetId("Cube.Material_2");
	materialPirmary->SetType(AssetMeta::Type::Material);
	materialPirmary->SetCategory(AssetMeta::Category::Primary);
	materialPirmary->SetReference(materialSeconday);

	materialPirmary->AddDependency(m_sAssetsDataList[AssetMeta::Category::Secondary].find("Cube.Diffuse")->second);
	materialPirmary->AddDependency(m_sAssetsDataList[AssetMeta::Category::Secondary].find("Cube.Specular")->second);
	materialPirmary->AddDependency(m_sAssetsDataList[AssetMeta::Category::Secondary].find("Cube.Normal")->second);

	m_sAssetsDataList[AssetMeta::Category::Primary].insert({ "Cube.Material_2", materialPirmary });
	m_sAssetsDataList[AssetMeta::Category::Secondary].insert({ "Cube.Material_2", materialSeconday });*/

	// We need to replace all PrimaryReference and SecondaryReference encounters and set actul links 
	for (auto& [id, asset] : m_sAssetsDataList[AssetMeta::Category::Primary])
		LinkAssetDataRecursivly(id, asset);
}

void shade::AssetManager::ReleaseMe(const std::string& id, AssetMeta::Category category)
{
	auto asset = m_sAssets[category].find(id);
	if (asset != m_sAssets[category].end())
	{
		m_sAssets[category].erase(asset);
	}
}

void shade::AssetManager::ReadAssetDataRecursively(SharedPointer<AssetData>& data)
{
	try
	{
		const std::string id = data->GetId();
		const AssetMeta::Category category = data->GetCategory();
		const AssetMeta::Category searchCategory = (data->GetCategory() == AssetMeta::Category::PrimaryReference) ? 
			AssetMeta::Category::Primary : (data->GetCategory() == AssetMeta::Category::SecondaryReference) ? 
			AssetMeta::Category::Secondary : data->GetCategory();

		AssetsDataList::iterator search = m_sAssetsDataList[searchCategory].find(id);

		if (category != AssetMeta::Category::PrimaryReference && category != AssetMeta::Category::SecondaryReference)
		{
			if (search == m_sAssetsDataList[searchCategory].end())
			{
				m_sAssetsDataList[searchCategory].insert({ id, data });
				if (category == AssetMeta::Category::Primary)
				{
					if (data->m_SecondaryReferenceId != "NULL")
					{
						// Asset can reference only to secondary
						auto ref = m_sAssetsDataList[AssetMeta::Category::Secondary].find(data->m_SecondaryReferenceId);
						if (ref != m_sAssetsDataList[AssetMeta::Category::Secondary].end())
						{
							data->SetReference(ref->second);
						}
					}
				}
			}
			else
			{
				SHADE_CORE_WARNING("Asset id already exists '{0}'. Skipped...", id)
			}	
		}
		
		for (auto& deppendency : data->GetDependencies())
		{
			ReadAssetDataRecursively(deppendency);
		}
	}
	catch (std::exception& exception)
	{
		SHADE_CORE_WARNING(exception.what());
	}
}

void shade::AssetManager::LinkAssetDataRecursivly(const std::string& id, SharedPointer<AssetData>& data)
{
	// TODO: Has to be tested more to be sure that it works correct.
	const AssetMeta::Category assetCategory = (data->GetCategory() == AssetMeta::Category::PrimaryReference) ?
		AssetMeta::Category::Primary : (data->GetCategory() == AssetMeta::Category::SecondaryReference) ?
		AssetMeta::Category::Secondary : data->GetCategory();

	AssetsDataRelink::iterator relinkSearch = m_sAssetDataRelink[assetCategory].find(id);

	if (relinkSearch == m_sAssetDataRelink[assetCategory].end())
	{
		for (auto& dependecy : data->GetDependencies())
		{
			const AssetMeta::Category depsCategory = (dependecy->GetCategory() == AssetMeta::Category::PrimaryReference) ?
				AssetMeta::Category::Primary : (dependecy->GetCategory() == AssetMeta::Category::SecondaryReference) ?
				AssetMeta::Category::Secondary : dependecy->GetCategory();

			const std::string depsId = dependecy->GetId();
			AssetsDataList::iterator depsSearch = m_sAssetsDataList[depsCategory].find(depsId);

			if (depsSearch != m_sAssetsDataList[depsCategory].end())
				dependecy = depsSearch->second;

			LinkAssetDataRecursivly(depsId, dependecy);
		}
		m_sAssetDataRelink[assetCategory].insert(id);
	}
}

void shade::AssetManager::Save(const std::string& filePath)
{
	File file(filePath, File::Out, "@s_m_asset", File::VERSION(0, 0, 1));
	if (file.IsOpen())
	{
		Save(file.GetStream());
		file.CloseFile();
	}

	//std::ofstream file(filePath, std::ios::binary);
	//if (!file.is_open())
	//	SHADE_CORE_WARNING("Failed to save asset data, wrong path = {0}", filePath)
	//else
	//	Save(file);

	//file.close();
}

void shade::AssetManager::Save(std::ostream& stream)
{
	for (auto& [id, asset] : m_sAssetsDataList[AssetMeta::Category::Secondary])
	{
		// Full serialize for secondary asset !
		Serializer::Serialize(stream, asset);
	}
	for (auto& [id, asset] : m_sAssetsDataList[AssetMeta::Category::Primary])
	{
		Serializer::Serialize(stream, asset->GetId());
		Serializer::Serialize(stream, std::uint32_t(asset->GetCategory()));
		Serializer::Serialize(stream, std::uint32_t(asset->GetType()));
		if (asset->GetReference())
			Serializer::Serialize(stream, asset->GetReference()->GetId());
		else
			Serializer::Serialize(stream, std::string("NULL"));

		Serializer::Serialize(stream, asset->GetAttributes());
		Serializer::Serialize(stream, std::uint32_t(asset->GetDependencies().size()));

		// Partial serialize for dependecy because they will be as separete encounter anyway 
		for (auto& dependency : asset->GetDependencies())
		{
			Serializer::Serialize(stream, dependency->GetId());
			const AssetMeta::Category depsCategory = dependency->GetCategory();
			if(depsCategory == AssetMeta::Category::Primary)
				Serializer::Serialize(stream, std::uint32_t(AssetMeta::Category::PrimaryReference));
			else if(depsCategory == AssetMeta::Category::Secondary)
				Serializer::Serialize(stream, std::uint32_t(AssetMeta::Category::SecondaryReference));
			else
				Serializer::Serialize(stream, std::uint32_t(depsCategory));

				Serializer::Serialize(stream, std::uint32_t(dependency->GetType()));

			if(dependency->GetReference())
				Serializer::Serialize(stream, dependency->GetReference()->GetId());
			else
				Serializer::Serialize(stream, std::string("NULL"));
			// Set dependecy's info as min as posible(Just a short link)
			// No attributes for dependecies
			Serializer::Serialize(stream, std::uint32_t(0));
			// No dependecies for dependency
			Serializer::Serialize(stream, std::uint32_t(0));
		}
	}
	for (auto& [id, asset] : m_sAssetsDataList[AssetMeta::Category::Blueprint])
	{
		Serializer::Serialize(stream, asset);
	}
}

void shade::AssetManager::DeliveryAssets()
{
	{
		Delivery(AssetMeta::Category::Primary);
	}
	{
		Delivery(AssetMeta::Category::Secondary);
	}
	{
		Delivery(AssetMeta::Category::Blueprint);
	}
}

shade::AssetManager::AssetsDataList& shade::AssetManager::GetAssetDataList(AssetMeta::Category category)
{
	return m_sAssetsDataList[category];
}

shade::SharedPointer<shade::AssetData> shade::AssetManager::GetAssetData(shade::AssetMeta::Category category, const std::string& id)
{
	AssetsDataList::iterator search = m_sAssetsDataList[category].find(id);
	if (search != m_sAssetsDataList[category].end())
	{
		return search->second;
	}
	else
	{
		SHADE_CORE_WARNING("Couldn't find '{0}' in AssetDataList !", id);
		return nullptr;
	}
}

void shade::AssetManager::ShutDown()
{
	for (auto& relink : m_sAssetDataRelink)
		relink.clear();
	for (auto& asset : m_sAssets)
		asset.clear();
	for (auto& assetData : m_sAssetsDataList)
		assetData.clear();
}
