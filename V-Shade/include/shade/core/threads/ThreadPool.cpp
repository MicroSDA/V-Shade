#include "shade_pch.h"
#include "ThreadPool.h"

shade::thread::ThreadPool::ThreadPool(std::size_t threadsCount)
{
	if (!threadsCount)
		throw std::invalid_argument("Invalid thread count: 0 or less.");
	else
		Start(threadsCount);
}

shade::thread::ThreadPool::~ThreadPool()
{
	Quit();
}

void shade::thread::ThreadPool::Start(std::size_t threadsCount)
{
	for (std::size_t i = 0; i < threadsCount; ++i)
	{
		m_Threads.emplace_back([=] {
			while (true)
			{
				//Sync
				std::unique_lock<std::mutex> lock{ m_Mutex };

				m_Event.wait(lock, [=] { return m_Quit || !m_Tasks.empty(); });

				if (m_Quit && m_Tasks.empty())
					break;

				auto task = std::move(m_Tasks.front());
				m_Tasks.pop();
				lock.unlock();
				//!Sync

				//Async
				task();
				//!Async
			}
		});
	}
}

void shade::thread::ThreadPool::Quit() noexcept
{
	{
		std::unique_lock<std::mutex> lock{ m_Mutex };
		m_Quit = true;
	}

	m_Event.notify_all();

	for (auto& thread : m_Threads)
		thread.join();
}
