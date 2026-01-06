#include "hpp/tools/log.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace Tools
{
namespace Log
{
static bool s_isSetup = false;
static const char *s_pAPPLICATION_NAME = nullptr;
static std::filesystem::path s_applicationPathRoot;
static std::filesystem::path s_logPath;
static bool s_useTimestamp = false;
static bool s_fileClosed = true;
static Level s_logLevel;
static std::ofstream s_file;

static std::string s_fileName = "";
static std::string s_timestampStr = "";
static std::string s_msgHeaderStr = "";

static std::tm utc_tm_now()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm out{};
#if defined(_WIN32)
    // Windows
    gmtime_s(&out, &time);
#else
    // POSIX
    gmtime_r(&time, &out);
#endif
    return out;
}

static std::string utc_timestamp_string()
{
    // "YYYY-MM-DD HH:MM:SS"
    std::tm tm = utc_tm_now();
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

static std::string utc_filename_stamp()
{
    // "YYYYyMMmDDdHHhMMmSSsUTC"
    std::tm tm = utc_tm_now();
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Yy%mm%dd%Hh%Mm%SsUTC");
    return oss.str();
}

static void log(const Level LOGGING_LEVEL, const std::string_view &rMSG)
{
    if (!s_isSetup)
        return;

    if (s_fileClosed)
        return;

    if (LOGGING_LEVEL > s_logLevel)
        return;

    const char *pLEVEL_STR = "Unknown";

    switch (LOGGING_LEVEL)
    {
        case Level::Info:
            pLEVEL_STR = "Info ";
            break;
        case Level::Error:
            pLEVEL_STR = "Error";
            break;
        case Level::Warn:
            pLEVEL_STR = "Warn ";
            break;
        case Level::Debug:
            pLEVEL_STR = "Debug";
            break;
    }

    if (s_useTimestamp)
        s_timestampStr = "[" + utc_timestamp_string() + "]";
    else
        s_timestampStr.clear();

    // Header: "[YYYY-MM-DD HH:MM:SS][Level] "  or "[Level] " if no timestamps
    if (!s_timestampStr.empty())
        s_msgHeaderStr = s_timestampStr + "[" + pLEVEL_STR + "] ";
    else
        s_msgHeaderStr = std::string("[") + pLEVEL_STR + "] ";

    const std::string line = s_msgHeaderStr + std::string(rMSG);
    s_file << line << std::endl;

    switch (LOGGING_LEVEL)
    {
        case Level::Error:
            godot::UtilityFunctions::printerr(godot::String(line.c_str()));
            break;
        case Level::Warn:
            godot::UtilityFunctions::push_warning(godot::String(line.c_str()));
            break;
        default:
            godot::UtilityFunctions::print(godot::String(line.c_str()));
            break;
    }
}

void info(const std::string_view &MSG) { log(Level::Info, MSG); }
void error(const std::string_view &MSG) { log(Level::Error, MSG); }
void warn(const std::string_view &MSG) { log(Level::Warn, MSG); }
void debug(const std::string_view &MSG) { log(Level::Debug, MSG); }

Line::~Line() noexcept
{
    if (!active)
        return;

    const std::string s = oss.str();

    if (!s.empty())
        log(level, s);
}

Line info() { return Line(Level::Info); }
Line warn() { return Line(Level::Warn); }
Line error() { return Line(Level::Error); }
Line debug() { return Line(Level::Debug); }

static void make_gdignore(const std::filesystem::path &dir)
{
    std::filesystem::create_directories(dir);
    std::ofstream(dir / ".gdignore", std::ios::out | std::ios::trunc).close();
}

static void create_log_file(void)
{
    s_logPath = s_applicationPathRoot / "logs";
    std::filesystem::create_directories(s_logPath);

    // Makes log folder hidden in godot editor
    make_gdignore(s_logPath);

    s_file.open(s_logPath / s_fileName, std::ios::out);
    s_fileClosed = false;
}

static void build_file_name(void)
{
    // Timestamp in filename, UTC
    s_timestampStr = utc_filename_stamp();

    s_fileName = std::string(s_pAPPLICATION_NAME) + "_" + s_timestampStr;
    std::replace(s_fileName.begin(), s_fileName.end(), ' ', '_');
    s_fileName.append(".log");
}

static void log_compile_version(void)
{
    Tools::Log::debug() << "C++ version: "
                        << std::string("C++ __cplusplus=") +
                    std::to_string((long long)__cplusplus)
#if defined(_MSVC_LANG)
                    + " _MSVC_LANG=" +
                    std::to_string((long long)_MSVC_LANG)
#endif
            ;
}

void begin(const char *pAPPLICATION_NAME,
        const std::filesystem::path APPLICATION_PATH_ROOT,
        const bool USE_TIMESTAMP, const Level LOGGING_LEVEL)
{
    if (s_isSetup)
        return;

    if (!pAPPLICATION_NAME)
        return;

    s_pAPPLICATION_NAME = pAPPLICATION_NAME;
    s_applicationPathRoot = APPLICATION_PATH_ROOT;
    s_useTimestamp = USE_TIMESTAMP;
    s_logLevel = LOGGING_LEVEL;

    build_file_name();

    s_isSetup = true;
    create_log_file();

    if (s_useTimestamp)
        info("Timestamps are enabled recorded as YYYY-MM-DD HH:MM:SS in UTC time.");

    log_compile_version();
}

void end(void)
{
    if (!s_isSetup)
        return;

    if (s_fileClosed)
        return;

    s_file.close();
    s_fileClosed = true;
    s_isSetup = false;
}
} // namespace Log
} // namespace Tools
