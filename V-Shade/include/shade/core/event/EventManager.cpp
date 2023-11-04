#include "shade_pch.h"
#include "EventManager.h"


std::queue<std::function<void()>> shade::EventManager::m_sEventsPool;
std::mutex shade::EventManager::m_sEventQueueMutex;

void shade::EventManager::Initialize()
{

}

void shade::EventManager::ShutDown()
{

}

void shade::EventManager::PollEvents()
{
	std::scoped_lock<std::mutex> lock(m_sEventQueueMutex);
	// Process custom event queue
	while (m_sEventsPool.size() > 0)
	{
		// Execute event callback.
		m_sEventsPool.front()();
		m_sEventsPool.pop();
	}
}
