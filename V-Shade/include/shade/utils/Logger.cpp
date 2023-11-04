#include "shade_pch.h"
#include "Logger.h"

// Static initialization 
std::shared_ptr<spdlog::logger>	shade::Logger::m_sCoreLogger;
std::shared_ptr<spdlog::logger>	shade::Logger::m_sClientLogger;
std::ostringstream						shade::Logger::m_sClientStream;
std::ostringstream						shade::Logger::m_sCoreStream;

void shade::Logger::Initialize()
{
	spdlog::set_pattern("%^[%T]%n:%v%$");

#if 0
	m_sCoreLogger = std::make_shared<spdlog::logger>("CORE", std::make_shared<spdlog::sinks::ostream_sink_mt>(m_sCoreStream));
	m_sCoreLogger->set_level(spdlog::level::trace);

	m_sClientLogger = std::make_shared<spdlog::logger>("APPLICATION", std::make_shared<spdlog::sinks::ostream_sink_mt>(m_sClientStream));
	m_sClientLogger->set_level(spdlog::level::trace);
#else
	m_sCoreLogger = spdlog::stderr_color_mt("CORE");
	m_sCoreLogger->set_level(spdlog::level::trace);

	m_sClientLogger = spdlog::stderr_color_mt("APPLICATION");
	m_sClientLogger->set_level(spdlog::level::trace);
#endif // 0
}
