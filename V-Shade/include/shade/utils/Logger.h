#pragma once

//#define SPDLOG_USE_STD_FORMAT
//#define SPDLOG_COMPILED_LIB

#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>

namespace shade
{
	class SHADE_API Logger
	{
	public:
		static void Initialize();
		inline static std::shared_ptr<spdlog::logger>& CoreLogger() { return m_sCoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& ClientLogger() { return m_sClientLogger; }
		inline static std::ostringstream& GetCoreStream() { return m_sCoreStream; }
		inline static std::ostringstream& GetClientStream() { return m_sClientStream; }
	private:
		static std::shared_ptr<spdlog::logger> m_sCoreLogger;
		static std::shared_ptr<spdlog::logger> m_sClientLogger;
		static std::ostringstream m_sCoreStream;
		static std::ostringstream m_sClientStream;
	};

#define SHADE_CORE_ERROR(...)     {::shade::Logger::CoreLogger()->error(__VA_ARGS__);  abort(); }
#define SHADE_CORE_WARNING(...)   ::shade::Logger::CoreLogger()->warn(__VA_ARGS__);
#define SHADE_CORE_INFO(...)      ::shade::Logger::CoreLogger()->info(__VA_ARGS__);
#define SHADE_CORE_TRACE(...)     ::shade::Logger::CoreLogger()->trace(__VA_ARGS__);
#define SHADE_CORE_DEBUG(...)     ::shade::Logger::CoreLogger()->debug(__VA_ARGS__);

#define SHADE_ERROR(...)		  ::shade::Logger::ClientLogger()->error(__VA_ARGS__);
#define SHADE_WARNING(...)		  ::shade::Logger::ClientLogger()->warn(__VA_ARGS__);
#define SHADE_INFO(...)			  ::shade::Logger::ClientLogger()->info(__VA_ARGS__);
#define SHADE_TRACE(...)		  ::shade::Logger::ClientLogger()->trace(__VA_ARGS__);
}

