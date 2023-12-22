#pragma once
#include <shade/core/asset/Asset.h>

namespace shade
{
	//class SHADE_API AnimationGraph : public BaseAsset, public Asset<AnimationGraph>
	//{
	//public:

	//	class SHADE_API Node
	//	{
	//	public:

	//	};

	//	// Destructor
	//	virtual ~AnimationGraph() = default;
	//	// Static method to get the static type of AnimationGraph asset
	//	static AssetMeta::Type GetAssetStaticType();
	//	// Virtual method to get the type of the AnimationGraph asset
	//	virtual AssetMeta::Type GetAssetType() const override;
	//	// Default constructor
	//	AnimationGraph() = default;
	//	// Static method to create a shared pointer to a AnimationGraph object
	//	static SharedPointer<AnimationGraph> CreateEXP();
	//public:


	//private:
	//	// Create a skeleton object with the given asset data, lifetime, and instantiation behaviour
	//	AnimationGraph(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
	//	// Create a new skeleton object with the given asset data, lifetime, and instantiation behaviour
	//	static AnimationGraph* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
	//	// Serialize the animationGraph object and write the serialized data to the given output stream
	//	std::size_t Serialize(std::ostream& stream) const;
	//	// Deserialize the animationGraph object from the given input stream and return the number of bytes read
	//	std::size_t Deserialize(std::istream& stream);
	//private:
	//	friend class Serializer;
	//	friend class Asset<AnimationGraph>;
	//private:
	//	std::unordered_map<std::string, SharedPointer<Node>> m_Nodes;
	//};
}