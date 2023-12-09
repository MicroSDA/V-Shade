#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/math/Math.h>
#include <glm/glm/gtx/quaternion.hpp>
#include <shade/core/asset/Asset.h>
// TODO: add namespace animation
namespace shade
{
	class SHADE_API Animation : public BaseAsset, public Asset<Animation>
	{
	public:

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
		float GetDureation() const;

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
}
