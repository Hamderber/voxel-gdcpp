#pragma once
#include <filesystem>
#include <sstream>
#include <string_view>

namespace Tools
{
namespace Log
{
enum class Level : unsigned char
{
    Info = 0,
    Error = 1,
    Warn = 2,
    Debug = 3
};

void info(const std::string_view &MSG);
void warn(const std::string_view &MSG);
void error(const std::string_view &MSG);
void debug(const std::string_view &MSG);

struct Line
{
    Level level;
    std::ostringstream oss;
    bool active = true;

    explicit Line(Level lvl) : level(lvl) {}

    Line(const Line &) = delete;
    Line &operator=(const Line &) = delete;

    Line(Line &&other) noexcept
            : level(other.level),
              oss(std::move(other.oss)),
              active(other.active)
    {
        other.active = false;
    }

    ~Line() noexcept;

    template <class T>
    Line &operator<<(const T &v)
    {
        oss << v;
        return *this;
    }

    Line &operator<<(std::ostream &(*manip)(std::ostream &))
    {
        oss << manip;
        return *this;
    }
};

Line info();
Line warn();
Line error();
Line debug();

void begin(const char *pAPPLICATION_NAME_STR, const std::filesystem::path APPLICATION_PATH_ROOT, const bool USE_TIMESTAMP,
        const Level LOGGING_LEVEL);
void end(void);
} //namespace Log
} //namespace Tools
