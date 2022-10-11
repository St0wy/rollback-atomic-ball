#include <spdlog/spdlog.h>

#include <utils/log.hpp>

namespace core
{


void LogDebug(const std::string_view msg)
{
    spdlog::info(msg);
}

void LogWarning(const std::string_view msg)
{
    spdlog::warn(msg);
}

void LogError(const std::string_view msg)
{
    spdlog::error(msg);
}
}
