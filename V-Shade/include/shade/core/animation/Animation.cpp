#include "shade_pch.h"
#include "Animation.h"

shade::Animation::Animation(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
{
	const std::string filePath = assetData->GetAttribute<std::string>("Path");

	shade::File file(filePath, shade::File::In, "@s_sanim", shade::File::VERSION(0, 0, 1));
	if (!file.IsOpen())
		SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath)
	else
	{
		file.Read(*this);
		file.CloseFile();
	}
}

void shade::Animation::AddChannel(const std::string& name, const Channel& channel)
{
	if (m_AnimationChannels.find(name) != m_AnimationChannels.end())
		SHADE_CORE_ERROR("Animation channel '{0}' already exists in '{1}' animation!");

	m_AnimationChannels.emplace(name, channel);
}

glm::mat4 shade::Animation::InterpolatePosition(const Channel& channel, float time) const
{
	if (channel.PositionKeys.size() == 1)
		return glm::translate(glm::mat4(1.0f), channel.PositionKeys[0].Key);
	
	std::uint32_t firstKeyFrame = GetPositionKeyFrame(channel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(channel.PositionKeys[firstKeyFrame].TimeStamp, channel.PositionKeys[secondKeyFrame].TimeStamp, time);

	return glm::translate(glm::mat4(1.0f), glm::mix(channel.PositionKeys[firstKeyFrame].Key, channel.PositionKeys[secondKeyFrame].Key, timeFactor));
}

glm::mat4 shade::Animation::InterpolateRotation(const Channel& channel, float time) const
{
	if (channel.RotationKeys.size() == 1)
		return glm::toMat4(glm::normalize(channel.RotationKeys[0].Key));

	std::uint32_t firstKeyFrame = GetRotationKeyFrame(channel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(channel.RotationKeys[firstKeyFrame].TimeStamp, channel.RotationKeys[secondKeyFrame].TimeStamp, time);

	return glm::toMat4(glm::normalize(glm::slerp(channel.RotationKeys[firstKeyFrame].Key, channel.RotationKeys[secondKeyFrame].Key, timeFactor)));
}

glm::mat4 shade::Animation::InterpolateScale(const Channel& channel, float time) const
{
	if (channel.ScaleKeys.size() == 1)
		return glm::scale(glm::mat4(1.f), channel.ScaleKeys[0].Key);

	std::uint32_t firstKeyFrame = GetScaleKeyFrame(channel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(channel.ScaleKeys[firstKeyFrame].TimeStamp, channel.ScaleKeys[secondKeyFrame].TimeStamp, time);

	return glm::scale(glm::mat4(1.0f), glm::mix(channel.ScaleKeys[firstKeyFrame].Key, channel.ScaleKeys[secondKeyFrame].Key, timeFactor));
}

std::size_t shade::Animation::GetPositionKeyFrame(const Channel& chanel, float time) const
{
	for (std::size_t index = 0; index < chanel.PositionKeys.size() - 1; ++index)
	{
		if (chanel.PositionKeys[index + 1].TimeStamp >= time) 
			return index;
	}
	return 0u;
}

std::size_t shade::Animation::GetRotationKeyFrame(const Channel& chanel, float time) const
{
	for (std::size_t index = 0; index < chanel.RotationKeys.size() - 1; ++index)
	{
		if (chanel.RotationKeys[index + 1].TimeStamp >= time)
			return index;
	}
	return 0u;
}

std::size_t shade::Animation::GetScaleKeyFrame(const Channel& chanel, float time) const
{
	for (std::size_t index = 0; index < chanel.ScaleKeys.size() - 1; ++index)
	{
		if (chanel.ScaleKeys[index + 1].TimeStamp >= time)
			return index;
	}
	return 0u;
}

float shade::Animation::GetTimeFactor(float currentTime, float nextTime, float time) const
{
	return (time - currentTime) / (nextTime - currentTime);
}

const shade::Animation::AnimationChannels& shade::Animation::GetAnimationCahnnels() const
{
	return m_AnimationChannels;
}

float shade::Animation::GetTiksPerSecond() const // TODO: Rename Tics
{
	return m_TicksPerSecond;
}

float shade::Animation::GetDuration() const
{
	return m_Duration;
}

void shade::Animation::SetTicksPerSecond(float count)
{
	m_TicksPerSecond = count;
}

void shade::Animation::SetDuration(float duration)
{
	m_Duration = duration;
}

std::size_t shade::Animation::Serialize(std::ostream& stream) const
{
	std::size_t size = Serializer::Serialize(stream, m_Duration);
	size += Serializer::Serialize(stream, m_TicksPerSecond);
	size += Serializer::Serialize(stream, m_AnimationChannels);

	return size;
}

std::size_t shade::Animation::Deserialize(std::istream& stream)
{
	std::size_t size = Serializer::Deserialize(stream, m_Duration);
	size += Serializer::Deserialize(stream, m_TicksPerSecond);
	size += Serializer::Deserialize(stream, m_AnimationChannels);
	return size;
}
