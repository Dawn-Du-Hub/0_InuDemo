#include "inuspdlog.h"
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h" 
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <chrono>
#include <sstream>
#include <iostream>
#include <fstream>
#include <ctime>
#include<iomanip>
namespace Inuchip
{
    using namespace std;
    std::shared_ptr<spdlog::logger> my_logger;
    void spdlog_init()
    {
        auto tp = std::chrono::system_clock::now(); 
        std::time_t cur_time = std::chrono::system_clock::to_time_t(tp); 
    
        std::stringstream  ss_time;
        ss_time << std::put_time(std::localtime(&cur_time), "%Y-%m-%d-%H-%M-%S");

        std::cout << ss_time.str() << std::endl;
        std::string filename = "logs/inuexample-log-" + ss_time.str() + ".txt";

        my_logger = spdlog::basic_logger_mt("file_logger", filename, true);

        my_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        my_logger->flush_on(spdlog::level::warn);
        my_logger->set_level(spdlog::level::info);

        my_logger->info("INU example start !");
        my_logger->flush();
    }
}