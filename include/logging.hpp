#pragma once
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "paper2_scotland2/shared/logger.hpp"

#define INFO(str, ...) Paper::Logger::fmtLogTag<Paper::LogLevel::INF>(str, "VoiceChat" __VA_OPT__(, __VA_ARGS__))
#define ERROR(str, ...) Paper::Logger::fmtLogTag<Paper::LogLevel::ERR>(str, "VoiceChat" __VA_OPT__(, __VA_ARGS__))
#define WARNING(str, ...) Paper::Logger::fmtLogTag<Paper::LogLevel::WRN>(str, "VoiceChat" __VA_OPT__(, __VA_ARGS__))
