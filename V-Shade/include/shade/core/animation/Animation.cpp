#include "shade_pch.h"
#include "Animation.h"

void shade::Animation::AddChannel(const std::string& name, const Channel& channel)
{
	m_AnimationChannels.emplace(name, channel);
}

glm::mat4 shade::Animation::InterpolatePosition(const Channel& channel, float time)
{
	if (channel.PositionKeys.size() == 1)
		return glm::translate(glm::mat4(1.0f), channel.PositionKeys[0].Translation);
	
	std::uint32_t firstKeyFrame = GetPositionKeyFrame(channel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(channel.PositionKeys[firstKeyFrame].TimeStamp, channel.PositionKeys[secondKeyFrame].TimeStamp, time);

	return glm::translate(glm::mat4(1.0f), glm::mix(channel.PositionKeys[firstKeyFrame].Translation, channel.PositionKeys[secondKeyFrame].Translation, timeFactor));
}

glm::mat4 shade::Animation::InterpolateRotation(const Channel& channel, float time)
{
	if (channel.RotationKeys.size() == 1)
		return glm::toMat4(glm::normalize(channel.RotationKeys[0].Orientation));

	std::uint32_t firstKeyFrame = GetRotationKeyFrame(channel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(channel.RotationKeys[firstKeyFrame].TimeStamp, channel.RotationKeys[secondKeyFrame].TimeStamp, time);

	return glm::toMat4(glm::normalize(glm::slerp(channel.RotationKeys[firstKeyFrame].Orientation, channel.RotationKeys[secondKeyFrame].Orientation, timeFactor)));
}

glm::mat4 shade::Animation::InterpolateScale(const Channel& channel, float time)
{
	if (channel.ScaleKeys.size() == 1)
		return glm::scale(glm::mat4(1.f), channel.ScaleKeys[0].Scale);

	std::uint32_t firstKeyFrame = GetScaleKeyFrame(channel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(channel.ScaleKeys[firstKeyFrame].TimeStamp, channel.ScaleKeys[secondKeyFrame].TimeStamp, time);

	return glm::scale(glm::mat4(1.0f), glm::mix(channel.ScaleKeys[firstKeyFrame].Scale, channel.ScaleKeys[secondKeyFrame].Scale, timeFactor));
}

std::size_t shade::Animation::GetPositionKeyFrame(const Channel& chanel, float time) const
{
	for (std::size_t index = 0; index < chanel.PositionKeys.size() - 1; ++index)
	{
		if (chanel.PositionKeys[index + 1].TimeStamp > time)
			return index;
	}
	return 0;
}

std::size_t shade::Animation::GetRotationKeyFrame(const Channel& chanel, float time) const
{
	for (std::size_t index = 0; index < chanel.RotationKeys.size() - 1; ++index)
	{
		if (chanel.RotationKeys[index + 1].TimeStamp > time)
			return index;
	}
	return 0;
}

std::size_t shade::Animation::GetScaleKeyFrame(const Channel& chanel, float time) const
{
	for (std::size_t index = 0; index < chanel.ScaleKeys.size() - 1; ++index)
	{
		if (chanel.ScaleKeys[index + 1].TimeStamp > time)
			return index;
	}
	return 0;
}

float shade::Animation::GetTimeFactor(float currentTime, float nextTime, float time)
{
	return (time - currentTime) / (nextTime - currentTime);
}

const shade::Animation::AnimationChannels& shade::Animation::GetAnimationCahnnels() const
{
	return m_AnimationChannels;
}

std::uint32_t shade::Animation::GetTiksPerSecond() const
{
	return m_TicksPerSecond;
}

float shade::Animation::GetDureation() const
{
	return m_Duration;
}

void shade::Animation::SetTicksPerSecond(std::uint32_t count)
{
	m_TicksPerSecond = count;
}

void shade::Animation::SetDuration(float duration)
{
	m_Duration = duration;
}
