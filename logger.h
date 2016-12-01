#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <mutex>

class Logger
{
public:
    ~Logger();
    static void log(const std::string &text);

};


#endif // LOGGER_H
