#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/event/Event.h>
#include <shade/core/memory/Memory.h>
#include <shade/utils/Logger.h>
#include <shade/core/application/Application.h>

namespace shade
{
	class SHADE_API EventManager
	{
		template<typename T>
		using EventCallbkack = std::function<bool(T&)>;
	public:
		static void Initialize();
		static void ShutDown();
		// Process new income events.
		static void PollEvents();
		// Push new event in queue.
		template<typename TEvent, typename ...Args>
		static void PushEvent(Args&&... args);
		// Dispatch event
		template<typename T>
		static bool Dispatch(EventCallbkack<T> func, Event& event);

	private:
		static std::queue<std::function<void()>> m_sEventsPool;
		static std::mutex m_sEventQueueMutex;
	private:
	};

	template<typename TEvent, typename ...Args>
	inline void EventManager::PushEvent(Args&&... args)
	{
		static_assert(std::is_assignable_v<Event, TEvent>);

		std::shared_ptr<TEvent> event = std::make_shared<TEvent>(std::forward<Args>(args)...);
		
		std::scoped_lock<std::mutex> lock(m_sEventQueueMutex);
		m_sEventsPool.push([event]() { Application::OnEvent(*event); });
	}
	template<typename T>
	inline bool EventManager::Dispatch(EventCallbkack<T> func, Event& event)
	{
		if (event.GetType() == T::GetStaticType())
			func(*(T*)&event);

		return false;
	}
}