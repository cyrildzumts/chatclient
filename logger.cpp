#include "logger.h"
#include <mutex>


Logger::~Logger()
{

}

void Logger::log(const std::string &text)
{
    static std::mutex mux;
    std::lock_guard<std::mutex>lk(mux);
    std::cout << text << std::endl;
    std::cout << std::flush;
}
