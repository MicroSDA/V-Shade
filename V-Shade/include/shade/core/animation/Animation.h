#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/math/Math.h>
#include <glm/glm/gtx/quaternion.hpp>
#include <shade/core/asset/Asset.h>
#include <shade/core/render/RenderAPI.h>
#include <shade/core/animation/Skeleton.h>

namespace shade
{
	class SHADE_API Animation : ASSET_INHERITANCE(Animation)
	{
		ASSET_DEFINITION_HELPER(Animation)

	public:
		enum class State : std::uint8_t
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
			// The position animation keys
			std::vector<AnimationKey<glm::vec3>> PositionKeys;
			// The rotation animation keys
			std::vector<AnimationKey<glm::quat>> RotationKeys; 
			// The scale animation keys
			std::vector<AnimationKey<glm::vec3>> ScaleKeys;
		};

		using AnimationChannels = std::unordered_map<std::string, Channel>;

	public: 
		virtual ~Animation() = default;
		// Add a channel to the animation
		void AddChannel(const std::string& name, const Channel& channel);
		// Interpolate the position of the channel at the given time
		glm::vec3 InterpolatePosition(const Channel& chanel, float time) const;
		// Interpolate the rotation of the channel at the given time
		glm::quat InterpolateRotation(const Channel& chanel, float time) const;
		// Interpolate the scale of the channel at the given time
		glm::vec3 InterpolateScale(const Channel& chanel, float time) const;
		// Get the keyframe index of the position at the given time
		std::size_t GetPositionKeyFrame(const Channel& chanel, float time) const;
		// Get the keyframe index of the rotation at the given time
		std::size_t GetRotationKeyFrame(const Channel& chanel, float time) const;
		// Get the keyframe index of the scale at the given time
		std::size_t GetScaleKeyFrame(const Channel& chanel, float time) const; 
		// Get the time factor for interpolation
		float GetTimeFactor(float currentTime, float nextTime, float time) const;
		// Get the animation channels
		const AnimationChannels& GetAnimationCahnnels() const; 
		const Channel* GetAnimationCahnnel(const std::string& name) const;
		// Get the ticks per second of the animation
		float GetTiksPerSecond() const; 
		// Get the duration of the animation
		float GetDuration() const; 
		// Set the ticks per second of the animation
		void SetTicksPerSecond(float count); 
		// Set the duration of the animation
		void SetDuration(float duration);

	private:
		Animation(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		void Serialize(std::ostream& stream) const;
		void Deserialize(std::istream& stream);
	private:
		friend class serialize::Serializer;
	private:
		AnimationChannels	m_AnimationChannels;
		float				m_TicksPerSecond = 0.f;
		float				m_Duration = 0.f;
	};

	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const std::vector<Animation::AnimationKey<glm::quat>>& key)
	{
		std::uint32_t size = key.size();
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		Serialize<std::uint32_t>(stream, size);

		if (size)
			stream.write(reinterpret_cast<const char*>(key.data()), key.size() * sizeof(Animation::AnimationKey<glm::quat>));
	}
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, std::vector<Animation::AnimationKey<glm::quat>>& key)
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
			stream.read(reinterpret_cast<char*>(key.data()), key.size() * sizeof(Animation::AnimationKey<glm::quat>));
		}
	}
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const std::vector<Animation::AnimationKey<glm::vec3>>& key)
	{
		std::uint32_t size = key.size();
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		Serialize<std::uint32_t>(stream, size);
	
		if (size)
			stream.write(reinterpret_cast<const char*>(key.data()), key.size() * sizeof(Animation::AnimationKey<glm::vec3>));
	}
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, std::vector<Animation::AnimationKey<glm::vec3>>& key)
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
			stream.read(reinterpret_cast<char*>(key.data()), key.size() * sizeof(Animation::AnimationKey<glm::vec3>));
		}
	}
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const Animation::Channel& channel)
	{
		serialize::Serializer::Serialize<std::vector<Animation::AnimationKey<glm::vec3>>>(stream, channel.PositionKeys);
		serialize::Serializer::Serialize<std::vector<Animation::AnimationKey<glm::quat>>>(stream, channel.RotationKeys);
		serialize::Serializer::Serialize<std::vector<Animation::AnimationKey<glm::vec3>>>(stream, channel.ScaleKeys);
	}
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, Animation::Channel& channel)
	{
		serialize::Serializer::Deserialize<std::vector<Animation::AnimationKey<glm::vec3>>>(stream, channel.PositionKeys);
		serialize::Serializer::Deserialize<std::vector<Animation::AnimationKey<glm::quat>>>(stream, channel.RotationKeys);
		serialize::Serializer::Deserialize<std::vector<Animation::AnimationKey<glm::vec3>>>(stream, channel.ScaleKeys);
	}
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const Animation::AnimationChannels& channels)
	{
		std::uint32_t size = static_cast<std::uint32_t>(channels.size());
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		serialize::Serializer::Serialize<std::uint32_t>(stream, size);
	
		if (size)
		{
			for (auto& [str, value] : channels)
			{
				serialize::Serializer::Serialize<std::string>(stream, str);
				serialize::Serializer::Serialize<Animation::Channel>(stream, value);
			}
		}
	}
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, Animation::AnimationChannels& channels)
	{
		std::uint32_t size = 0;
		// Read size first.
		serialize::Serializer::Deserialize<std::uint32_t>(stream, size);
		if (size == UINT32_MAX)
			throw std::out_of_range(std::format("Incorrect array size = {}", size));

		for (std::uint32_t i = 0; i < size; i++)
		{
			std::string key; Animation::Channel value;

			serialize::Serializer::Deserialize<std::string>(stream, key);
			serialize::Serializer::Deserialize<Animation::Channel>(stream, value);

			channels.insert({ key, value });
		}
	}
	/* Serialize Animation.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const Animation& animation)
	{
		return animation.Serialize(stream);
	}
	/* Deserialize Animation.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, Animation& animation)
	{
		return animation.Deserialize(stream);
	}
	/* Serialize SharedPointer<Animation>.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const SharedPointer<Animation>& animation)
	{
		return animation->Serialize(stream);
	}
	/* Serialize Asset<Animation>.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Serialize(std::ostream& stream, const Asset<Animation>& animation)
	{
		return animation->Serialize(stream);
	}
	/* Deserialize SharedPointer<Animation>.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, SharedPointer<Animation>& animation)
	{
		return animation->Deserialize(stream);
	}
	/* Deserialize Asset<Animation>.*/
	template<>
	SHADE_INLINE void serialize::Serializer::Deserialize(std::istream& stream, Asset<Animation>& animation)
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

