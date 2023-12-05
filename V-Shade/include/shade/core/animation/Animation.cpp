#include "shade_pch.h"
#include "Animation.h"

glm::mat4 shade::Animation::InterpolatePosition(const std::string& chanelName, float time)
{
	const auto& chanel = m_AnimationChanels[chanelName];
	if (chanel.PositionKeys.size() == 1)
		return glm::translate(glm::mat4(1.0f), chanel.PositionKeys[0].Translation);
	
	std::uint32_t firstKeyFrame = GetPositionKeyFrame(chanel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(chanel.PositionKeys[firstKeyFrame].TimeStamp, chanel.PositionKeys[secondKeyFrame].TimeStamp, time);

	return glm::translate(glm::mat4(1.0f), glm::mix(chanel.PositionKeys[firstKeyFrame].Translation, chanel.PositionKeys[secondKeyFrame].Translation, timeFactor));
}

glm::mat4 shade::Animation::InterpolateRotation(const std::string& chanelName, float time)
{
	const auto& chanel = m_AnimationChanels[chanelName];
	if (chanel.RotationKeys.size() == 1)
		return glm::toMat4(glm::normalize(chanel.RotationKeys[0].Orientation));

	std::uint32_t firstKeyFrame = GetRotationKeyFrame(chanel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(chanel.RotationKeys[firstKeyFrame].TimeStamp, chanel.RotationKeys[secondKeyFrame].TimeStamp, time);

	return glm::toMat4(glm::normalize(glm::slerp(chanel.RotationKeys[firstKeyFrame].Orientation, chanel.RotationKeys[secondKeyFrame].Orientation, timeFactor)));
}

glm::mat4 shade::Animation::InterpolateScale(const std::string& chanelName, float time)
{
	const auto& chanel = m_AnimationChanels[chanelName];
	if (chanel.ScaleKeys.size() == 1)
		return glm::scale(glm::mat4(1.f), chanel.ScaleKeys[0].Scale);

	std::uint32_t firstKeyFrame = GetScaleKeyFrame(chanel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(chanel.ScaleKeys[firstKeyFrame].TimeStamp, chanel.ScaleKeys[secondKeyFrame].TimeStamp, time);

	return glm::scale(glm::mat4(1.0f), glm::mix(chanel.ScaleKeys[firstKeyFrame].Scale, chanel.ScaleKeys[secondKeyFrame].Scale, timeFactor));
}

std::size_t shade::Animation::GetPositionKeyFrame(const Chanel& chanel, float time) const
{
	for (std::size_t index = 0; index < chanel.PositionKeys.size() - 1; ++index)
	{
		if (chanel.PositionKeys[index + 1].TimeStamp >= time)
			return index;
	}
	return 0;
}

std::size_t shade::Animation::GetRotationKeyFrame(const Chanel& chanel, float time) const
{
	for (std::size_t index = 0; index < chanel.RotationKeys.size() - 1; ++index)
	{
		if (chanel.RotationKeys[index + 1].TimeStamp >= time)
			return index;
	}
	return 0;
}

std::size_t shade::Animation::GetScaleKeyFrame(const Chanel& chanel, float time) const
{
	for (std::size_t index = 0; index < chanel.ScaleKeys.size() - 1; ++index)
	{
		if (chanel.ScaleKeys[index + 1].TimeStamp >= time)
			return index;
	}
	return 0;
}

float shade::Animation::GetTimeFactor(float currentTime, float nextTime, float time)
{
	return (time - currentTime) / (nextTime - currentTime);
}
