#pragma once
#include <shade/config/ShadeAPI.h>

namespace shade
{
	namespace thread
	{
		class SHADE_API ThreadPool
		{
		public:
			ThreadPool(std::size_t threadsCount = std::thread::hardware_concurrency());
			~ThreadPool();

			template<class T>
			auto Emplace(T task) -> std::future<decltype(task())>;
		private:
			void Start(std::size_t threadsCount);
			void Quit() noexcept;
		private:
			std::queue<std::function<void()>>	m_Tasks;
			std::vector<std::thread>			m_Threads;
			std::condition_variable				m_Event;
			std::mutex							m_Mutex;
			bool								m_Quit = false;
		};

		template<class T>
		inline auto ThreadPool::Emplace(T task) -> std::future<decltype(task())>
		{
			// TODO: Add exeption catcher !
			auto wrapper = std::make_shared<std::packaged_task<decltype(task()) ()>>(task);
			{
				std::unique_lock<std::mutex> lock{ m_Mutex };
				m_Tasks.emplace([=] {
					(*wrapper)();
					});
			}

			m_Event.notify_one();
			return wrapper->get_future();
		}

	}
}

