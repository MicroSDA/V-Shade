#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/math/Math.h>
#include <glm/glm/gtx/quaternion.hpp>
#include <shade/core/asset/Asset.h>
#include <shade/core/render/RenderAPI.h>

// TODO: add namespace animation
namespace shade
{
	class SHADE_API Animation : public BaseAsset, public Asset<Animation>
	{
	public:
		enum class State
		{
			Stop,
			Play,
			Pause
		};

		template<typename T>
		struct AnimationKey
		{
			T Key;
			float TimeStamp;
		};
		
		struct Channel
		{
			std::uint32_t ID = ~0;
			std::vector<AnimationKey<glm::vec3>>	PositionKeys;
			std::vector<AnimationKey<glm::quat>>	RotationKeys;
			std::vector<AnimationKey<glm::vec3>>	ScaleKeys;
		};

		using AnimationChannels = std::unordered_map<std::string, Channel>;

	public: 
		virtual ~Animation() = default;
		static AssetMeta::Type GetAssetStaticType();
		virtual AssetMeta::Type GetAssetType() const override;

		Animation() = default;
		static SharedPointer<Animation> CreateEXP();

		void AddChannel(const std::string& name, const Channel& channel);

		glm::mat4 InterpolatePosition(const Channel& chanel, float time);
		glm::mat4 InterpolateRotation(const Channel& chanel, float time);
		glm::mat4 InterpolateScale(const Channel& chanel, float time);

		std::size_t GetPositionKeyFrame(const Channel& chanel, float time) const;
		std::size_t GetRotationKeyFrame(const Channel& chanel, float time) const;
		std::size_t GetScaleKeyFrame(const Channel& chanel, float time) const;

		float GetTimeFactor(float currentTime, float nextTime, float time);
		const AnimationChannels& GetAnimationCahnnels() const;	

		float GetTiksPerSecond() const;
		float GetDuration() const;

		void SetTicksPerSecond(float count);
		void SetDuration(float duration);

	private:
		Animation(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		static Animation* Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		std::size_t Serialize(std::ostream& stream) const;
		std::size_t Deserialize(std::istream& stream);
	private:
		friend class Serializer;
		friend class Asset<Animation>;
	private:
		AnimationChannels m_AnimationChannels;
		float m_TicksPerSecond = 0.f;
		float m_Duration = 0.f;
	};

	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const std::vector<Animation::AnimationKey<glm::quat>>& key, std::size_t)
	{
		std::uint32_t size = key.size();
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		Serialize<std::uint32_t>(stream, size);

		if (size)
			return stream.write(reinterpret_cast<const char*>(key.data()), key.size() * sizeof(Animation::AnimationKey<glm::quat>)).tellp();
		else
			return stream.tellp();
	}
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, std::vector<Animation::AnimationKey<glm::quat>>& key, std::size_t)
	{
		std::uint32_t size = 0;
		// Read size first.
		Deserialize<std::uint32_t>(stream, size);
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		// If size is more then 0 need to read data.
		if (size)
		{
			key.resize(size);
			return stream.read(reinterpret_cast<char*>(key.data()), key.size() * sizeof(Animation::AnimationKey<glm::quat>)).tellg();
		}
		else
			return stream.tellg();
	}
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const std::vector<Animation::AnimationKey<glm::vec3>>& key, std::size_t)
	{
		std::uint32_t size = key.size();
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		Serialize<std::uint32_t>(stream, size);
	
		if (size)
			return stream.write(reinterpret_cast<const char*>(key.data()), key.size() * sizeof(Animation::AnimationKey<glm::vec3>)).tellp();
		else
			return stream.tellp();
	}
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, std::vector<Animation::AnimationKey<glm::vec3>>& key, std::size_t)
	{
		std::uint32_t size = 0;
		// Read size first.
		Deserialize<std::uint32_t>(stream, size);
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		// If size is more then 0 need to read data.
		if (size)
		{
			key.resize(size);
			return stream.read(reinterpret_cast<char*>(key.data()), key.size() * sizeof(Animation::AnimationKey<glm::vec3>)).tellg();
		}
		else
			return stream.tellg();
	}
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Animation::Channel& channel, std::size_t)
	{
		std::size_t size = Serialize<std::uint32_t>(stream, channel.ID);
		size += Serialize<std::vector<Animation::AnimationKey<glm::vec3>>>(stream, channel.PositionKeys);
		size += Serialize<std::vector<Animation::AnimationKey<glm::quat>>>(stream, channel.RotationKeys);
		size += Serialize<std::vector<Animation::AnimationKey<glm::vec3>>>(stream, channel.ScaleKeys);
		return size;
	}
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Animation::Channel& channel, std::size_t)
	{
		std::size_t size = Deserialize<std::uint32_t>(stream, channel.ID);
		size += Deserialize<std::vector<Animation::AnimationKey<glm::vec3>>>(stream, channel.PositionKeys);
		size += Deserialize<std::vector<Animation::AnimationKey<glm::quat>>>(stream, channel.RotationKeys);
		size += Deserialize<std::vector<Animation::AnimationKey<glm::vec3>>>(stream, channel.ScaleKeys);
		return size;
	}
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Animation::AnimationChannels& channels, std::size_t)
	{
		std::uint32_t size = static_cast<std::uint32_t>(channels.size());
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		std::size_t totalSize = Serialize<std::uint32_t>(stream, size);
	
		if (size)
		{
			for (auto& [str, value] : channels)
			{
				totalSize += Serialize<std::string>(stream, str);
				totalSize += Serialize<Animation::Channel>(stream, value);
			}
		}

		return totalSize;
	}
	template<>
	inline std::size_t Serializer::Deserialize(std::istream& stream, Animation::AnimationChannels& channels, std::size_t count)
	{
		std::uint32_t size = 0;
		// Read size first.
		std::size_t totalSize = Deserialize<std::uint32_t>(stream, size);
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		for (std::uint32_t i = 0; i < size; i++)
		{
			std::string key; Animation::Channel value;

			totalSize += Deserialize<std::string>(stream, key);
			totalSize += Deserialize<Animation::Channel>(stream, value);

			channels.insert({ key, value });
		}
		return totalSize;
	}
	/* Serialize Animation.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Animation& animation, std::size_t)
	{
		return animation.Serialize(stream);
	}
	/* Deserialize Animation.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Animation& animation, std::size_t)
	{
		return animation.Deserialize(stream);
	}
	/* Serialize Asset<Animation>.*/
	template<>
	inline std::size_t shade::Serializer::Serialize(std::ostream& stream, const Asset<Animation>& animation, std::size_t)
	{
		return animation->Serialize(stream);
	}
	/* Deserialize Asset<Animation>.*/
	template<>
	inline std::size_t shade::Serializer::Deserialize(std::istream& stream, Asset<Animation>& animation, std::size_t)
	{
		return animation->Deserialize(stream);
	}


#ifndef BONE_TRANSFORM_DATA_SIZE
	#define BONE_TRANSFORM_DATA_SIZE (sizeof(glm::mat4) * RenderAPI::MAX_BONES_PER_INSTANCE)
#endif // !BONE_TRANSFORM_DATA_SIZE

#ifndef BONE_TRANSFORMS_DATA_SIZE
	#define BONE_TRANSFORMS_DATA_SIZE(count) (BONE_TRANSFORM_DATA_SIZE * static_cast<std::uint32_t>(count))
#endif // !BONE_TRANSFORMS_DATA_SIZE
}

