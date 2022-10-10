#pragma once

#include <string_view>

class Log final
{
public:
    Log() = delete;

    static void log(std::string_view msg);
    static void error(std::string_view msg) { log(msg); }
    static void warning(std::string_view msg) { log(msg); }
    static void info(std::string_view msg) { log(msg); }
};
