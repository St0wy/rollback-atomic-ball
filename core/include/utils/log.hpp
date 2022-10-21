#pragma once

#include <string_view>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace core
{

class Log
{
public:
	inline static const spdlog::filename_t LOG_FILE_PATH = "logs/multisink.txt";

	Log();

	static void Debug(std::string_view msg);
	static void Info(std::string_view msg);
	static void Warning(std::string_view msg);
	static void Error(std::string_view msg);

private:
	inline static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> _consoleSink =
		std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

	inline static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> _fileSink =
		std::make_shared<spdlog::sinks::basic_file_sink_mt>(LOG_FILE_PATH, true);

	inline static spdlog::logger _logger{ "multi_sink", {_consoleSink, _fileSink} };
};

/**
 * \brief LogDebug is a function that prints the msg to the console
 * \param msg is the text to be printed
 */
void LogInfo(std::string_view msg);

/**
 * \brief LogWarning is a function that prints a yellow message to the console
 * \param msg is the text to be printed
 */
void LogWarning(std::string_view msg);

/**
 * \brief LogError is a function that prints a red message to the console
 * \param msg is the text to be printed
 */
void LogError(std::string_view msg);
}
