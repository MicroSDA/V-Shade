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

		struct Chanel
		{
			std::vector<PositionKey>	PositionKeys;
			std::vector<RotationKey>	RotationKeys;
			std::vector<ScaleKey>		ScaleKeys;
		};

	public: 
		Animation() = default;
		virtual ~Animation() = default;

		glm::mat4 InterpolatePosition(const std::string& chanelName, float time);
		glm::mat4 InterpolateRotation(const std::string& chanelName, float time);
		glm::mat4 InterpolateScale(const std::string& chanelName, float time);


		std::size_t GetPositionKeyFrame(const Chanel& chanel, float time) const;
		std::size_t GetRotationKeyFrame(const Chanel& chanel, float time) const;
		std::size_t GetScaleKeyFrame(const Chanel& chanel, float time) const;

		float GetTimeFactor(float currentTime, float nextTime, float time);

	public:
		// TODO: Temporary public 
		std::unordered_map<std::string, Chanel> m_AnimationChanels;
		std::uint32_t m_TicksPerSecond = 0;
		float m_Duration;
	};
}
