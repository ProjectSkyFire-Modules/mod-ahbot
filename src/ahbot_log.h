/*
 * Logging helpers that keep the upstream {} format strings while using SkyFire's
 * printf-style SF_LOG macros.
 */

#ifndef AHBOT_LOG_H
#define AHBOT_LOG_H

#include "Log.h"
#include <format>
#include <utility>

template<typename... Args>
inline void AHBotLogInfo(std::format_string<Args...> fmt, Args&&... args)
{
    SF_LOG_INFO("module", "%s", std::format(fmt, std::forward<Args>(args)...).c_str());
}

template<typename... Args>
inline void AHBotLogError(std::format_string<Args...> fmt, Args&&... args)
{
    SF_LOG_ERROR("module", "%s", std::format(fmt, std::forward<Args>(args)...).c_str());
}

#endif
