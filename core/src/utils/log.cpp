#include <spdlog/spdlog.h>

#include <utils/log.hpp>

namespace core
{
void LogDebug(const std::string_view msg)
{
	spdlog::debug(msg);
}

Log::Log()
{
	_consoleSink->set_level(spdlog::level::debug);
	_fileSink->set_level(spdlog::level::trace);

	_logger.set_level(spdlog::level::debug);
	_logger.warn("this should appear in both console and file");
	_logger.info("this message should not appear in the console, only in the file");
}

void Log::Debug(const std::string_view msg)
{
	_logger.debug(msg);
}

void Log::Info(const std::string_view msg)
{
	_logger.info(msg);
}

void Log::Warning(const std::string_view msg)
{
	_logger.warn(msg);
}

void Log::Error(const std::string_view msg)
{
	_logger.error(msg);
}

void LogInfo(const std::string_view msg)
{
	//spdlog::info(msg);
	Log::Info(msg);
}

void LogWarning(const std::string_view msg)
{
	Log::Warning(msg);
	//spdlog::warn(msg);
}

void LogError(const std::string_view msg)
{
	Log::Error(msg);
	//spdlog::error(msg);
}
}
