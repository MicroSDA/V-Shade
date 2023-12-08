#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/math/Math.h>
#include <glm/glm/gtx/quaternion.hpp>
// TODO: add namespace animation
namespace shade
{
	class SHADE_API Animation
	{
	public:

		struct PositionKey
		{
			glm::vec3 Translation;
			float TimeStamp;
		};

		struct RotationKey
		{
			glm::quat Orientation;
			float TimeStamp;
		};

		struct ScaleKey
		{
			glm::vec3 Scale;
			float TimeStamp;
		};

		struct Channel
		{
			std::uint32_t ID = ~0;
			std::vector<PositionKey>	PositionKeys;
			std::vector<RotationKey>	RotationKeys;
			std::vector<ScaleKey>		ScaleKeys;
		};

		using AnimationChannels = std::unordered_map<std::string, Channel>;

	public: 
		Animation() = default;
		virtual ~Animation() = default;

		void AddChannel(const std::string& name, const Channel& channel);

		glm::mat4 InterpolatePosition(const Channel& chanel, float time);
		glm::mat4 InterpolateRotation(const Channel& chanel, float time);
		glm::mat4 InterpolateScale(const Channel& chanel, float time);

		std::size_t GetPositionKeyFrame(const Channel& chanel, float time) const;
		std::size_t GetRotationKeyFrame(const Channel& chanel, float time) const;
		std::size_t GetScaleKeyFrame(const Channel& chanel, float time) const;

		float GetTimeFactor(float currentTime, float nextTime, float time);
		const AnimationChannels& GetAnimationCahnnels() const;	

		std::uint32_t GetTiksPerSecond() const;
		float GetDureation() const;

		void SetTicksPerSecond(std::uint32_t count);
		void SetDuration(float duration);

	private:
		AnimationChannels m_AnimationChannels;
		std::uint32_t m_TicksPerSecond = 0;
		float m_Duration;
	};
}
