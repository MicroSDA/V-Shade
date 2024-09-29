#include "shade_pch.h"
#include "Animation.h"
#include <shade/core/serializing/File.h>

shade::Animation::Animation(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
{
	const std::string filePath = assetData->GetAttribute<std::string>("Path");

	if (file::File file = file::FileManager::LoadFile(filePath, "@s_anim"))
	{
		file.Read(*this);
	}
	else
	{
		SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath);
	}
}

void shade::Animation::AddChannel(const std::string& name, const Channel& channel)
{
	if (m_AnimationChannels.find(name) != m_AnimationChannels.end())
		SHADE_CORE_ERROR("Animation channel '{0}' already exists in '{1}' animation!");

	m_AnimationChannels.emplace(name, channel);
}

glm::vec3 shade::Animation::InterpolatePosition(const Channel& channel, float time) const
{
	if (channel.PositionKeys.size() == 1)
		return channel.PositionKeys[0].Key;
	
	std::uint32_t firstKeyFrame = GetPositionKeyFrame(channel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(channel.PositionKeys[firstKeyFrame].TimeStamp, channel.PositionKeys[secondKeyFrame].TimeStamp, time);

	return glm::mix(channel.PositionKeys[firstKeyFrame].Key, channel.PositionKeys[secondKeyFrame].Key, timeFactor);
}

glm::quat shade::Animation::InterpolateRotation(const Channel& channel, float time) const
{
	if (channel.RotationKeys.size() == 1)
		return glm::toMat4(glm::normalize(channel.RotationKeys[0].Key));

	std::uint32_t firstKeyFrame = GetRotationKeyFrame(channel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(channel.RotationKeys[firstKeyFrame].TimeStamp, channel.RotationKeys[secondKeyFrame].TimeStamp, time);

	return glm::normalize(glm::slerp(channel.RotationKeys[firstKeyFrame].Key, channel.RotationKeys[secondKeyFrame].Key, timeFactor));
}

glm::vec3 shade::Animation::InterpolateScale(const Channel& channel, float time) const
{
	if (channel.ScaleKeys.size() == 1)
		return channel.ScaleKeys[0].Key;

	std::uint32_t firstKeyFrame = GetScaleKeyFrame(channel, time), secondKeyFrame = firstKeyFrame++;

	float timeFactor = GetTimeFactor(channel.ScaleKeys[firstKeyFrame].TimeStamp, channel.ScaleKeys[secondKeyFrame].TimeStamp, time);

	return glm::mix(channel.ScaleKeys[firstKeyFrame].Key, channel.ScaleKeys[secondKeyFrame].Key, timeFactor);
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

const shade::Animation::Channel* shade::Animation::GetAnimationCahnnel(const std::string& name) const
{
	const auto it = m_AnimationChannels.find(name);
	if (it != m_AnimationChannels.end())
	{
		return &it->second;
	}
	
	return nullptr;
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

float shade::Animation::GetFps() const
{
	float fps = 0;
	for (auto [name, channel] : m_AnimationChannels)
		fps = std::max(fps, std::max(float(channel.PositionKeys.size()), std::max(float(channel.RotationKeys.size()), float(channel.ScaleKeys.size()))));
	return fps * m_TicksPerSecond / m_Duration;
}

void shade::Animation::Serialize(std::ostream& stream) const
{
	serialize::Serializer::Serialize(stream, m_Duration);
	serialize::Serializer::Serialize(stream, m_TicksPerSecond);
	serialize::Serializer::Serialize(stream, m_AnimationChannels);
}

void shade::Animation::Deserialize(std::istream& stream)
{
	serialize::Serializer::Deserialize(stream, m_Duration);
	serialize::Serializer::Deserialize(stream, m_TicksPerSecond);
	serialize::Serializer::Deserialize(stream, m_AnimationChannels);
}
