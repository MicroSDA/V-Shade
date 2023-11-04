#include "shade_pch.h"
#include "Layer.h"

std::unordered_map<std::type_index, shade::SharedPointer<shade::Layer>> shade::Layer::m_sLayers;

void shade::Layer::OnLayersUpdate(SharedPointer<Scene>& scene, const FrameTimer& deltaTime)
{
	for (auto& layer : m_sLayers)
		if(layer.second->IsUpdate())
			layer.second->OnUpdate(scene, deltaTime);
}

void shade::Layer::OnLayersRender(SharedPointer<Scene>& scene, const FrameTimer& deltaTime)
{
	for (auto& layer : m_sLayers)
		if (layer.second->IsActive() && layer.second->IsRender())
		{
			layer.second->OnRenderBegin();
			layer.second->OnRender(scene, deltaTime);
			layer.second->OnRenderEnd();
		}
			
}

void shade::Layer::OnLayersEvent(SharedPointer<Scene>& scene, const Event& event, const FrameTimer& deltaTime)
{
	for (auto& layer : m_sLayers)
		if (layer.second->IsActive())
			layer.second->OnEvent(scene, event, deltaTime);
}

void shade::Layer::RemoveAllLayers()
{
	for (auto& layer : m_sLayers)
		layer.second->OnDestroy();

	m_sLayers.clear();
}

void shade::Layer::SetActive(const bool& isActive)
{
	m_IsActive = isActive;
}

void shade::Layer::SetRender(const bool& isRender)
{
	m_IsRender = isRender;
}

void shade::Layer::SetUpdate(const bool& isUpdate)
{
	m_IsUpdate = isUpdate;
}

const bool& shade::Layer::IsActive() const
{
	return m_IsActive;
}

const bool& shade::Layer::IsRender() const
{
	return m_IsRender;
}

const bool& shade::Layer::IsUpdate() const
{
	return m_IsUpdate;
}
