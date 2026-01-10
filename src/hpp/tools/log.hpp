#pragma once

#include <string_view>

namespace Tools::Log
{
    enum class Level : unsigned char
    {
        Info = 0,
        Error = 1,
        Warn = 2,
        Debug = 3
    };

    void info(std::string_view msg);
    void warn(std::string_view msg);
    void error(std::string_view msg);
    void debug(std::string_view msg);

    void begin(const char *pAPP_NAME, std::string_view utf8, bool use_timestamp, Level logging_level);

    void end();
} //namespace Tools::Log
