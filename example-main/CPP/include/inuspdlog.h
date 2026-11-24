
#pragma once 
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h" 
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace Inuchip{
    extern std::shared_ptr<spdlog::logger> my_logger;
    void spdlog_init();
}