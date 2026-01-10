#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "hpp/tools/log_stream.hpp"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string>
#include <string_view>


namespace Tools::Log
{
    static bool s_isSetup = false;
    static const char *s_pAPPLICATION_NAME = nullptr;

    static std::filesystem::path s_applicationPathRoot;
    static std::filesystem::path s_logPath;

    static bool s_useTimestamp = false;
    static bool s_fileClosed = true;
    static Level s_logLevel = Level::Info;
    static std::ofstream s_file;

    static std::string s_fileName;
    static std::string s_timestampStr;
    static std::string s_msgHeaderStr;

    static std::tm utc_tm_now()
    {
        const auto now = std::chrono::system_clock::now();
        const std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::tm out{};
#if defined(_WIN32)
        gmtime_s(&out, &time);
#else
        gmtime_r(&time, &out);
#endif
        return out;
    }

    static std::string utc_timestamp_string()
    {
        std::tm tm = utc_tm_now();
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    static std::string utc_filename_stamp()
    {
        std::tm tm = utc_tm_now();
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Yy%mm%dd%Hh%Mm%SsUTC");
        return oss.str();
    }

    static void make_gdignore(const std::filesystem::path &dir)
    {
        std::filesystem::create_directories(dir);
        std::ofstream(dir / ".gdignore", std::ios::out | std::ios::trunc).close();
    }

    static void create_log_file()
    {
        s_logPath = s_applicationPathRoot / "logs";
        std::filesystem::create_directories(s_logPath);
        make_gdignore(s_logPath);

        s_file.open(s_logPath / s_fileName, std::ios::out);
        s_fileClosed = false;
    }

    static void build_file_name()
    {
        s_timestampStr = utc_filename_stamp();

        s_fileName = std::string(s_pAPPLICATION_NAME) + "_" + s_timestampStr;
        std::replace(s_fileName.begin(), s_fileName.end(), ' ', '_');
        s_fileName.append(".log");
    }

    static void log_line(Level lvl, std::string_view msg)
    {
        const char *level_str = "Unknown";
        switch (lvl)
        {
            case Level::Info:
                level_str = "Info ";
                break;
            case Level::Error:
                level_str = "Error";
                break;
            case Level::Warn:
                level_str = "Warn ";
                break;
            case Level::Debug:
                level_str = "Debug";
                break;
        }

        if (s_useTimestamp)
            s_timestampStr = "[" + utc_timestamp_string() + "]";
        else
            s_timestampStr.clear();

        if (!s_timestampStr.empty())
            s_msgHeaderStr = s_timestampStr + "[" + level_str + "] ";
        else
            s_msgHeaderStr = std::string("[") + level_str + "] ";

        const std::string line = s_msgHeaderStr + std::string(msg);

        switch (lvl)
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

        // Always write to godot but only to file when actually ready for that
        if (lvl > s_logLevel)
            return;

        if (!s_isSetup || s_fileClosed)
            return;

        s_file << line << '\n';
        s_file.flush();
    }

    void info(std::string_view msg) { log_line(Level::Info, msg); }
    void warn(std::string_view msg) { log_line(Level::Warn, msg); }
    void error(std::string_view msg) { log_line(Level::Error, msg); }
    void debug(std::string_view msg) { log_line(Level::Debug, msg); }

    Line::~Line() noexcept
    {
        if (!active)
            return;
        const std::string s = oss.str();
        if (!s.empty())
            log_line(level, s);
    }

    Line info() { return Line(Level::Info); }
    Line warn() { return Line(Level::Warn); }
    Line error() { return Line(Level::Error); }
    Line debug() { return Line(Level::Debug); }

    static void log_compile_version()
    {
        Tools::Log::debug() << "C++ version: "
                            << std::string("C++ __cplusplus=") + std::to_string((long long)__cplusplus)
#if defined(_MSVC_LANG)
                                       + " _MSVC_LANG=" + std::to_string((long long)_MSVC_LANG)
#endif
                ;
    }

    void begin(const char *application_name,
               std::string_view application_path_root_utf8,
               bool use_timestamp,
               Level logging_level)
    {
        if (s_isSetup)
            return;
        if (!application_name)
            return;

        s_pAPPLICATION_NAME = application_name;
        s_useTimestamp = use_timestamp;
        s_logLevel = logging_level;

        s_applicationPathRoot = std::filesystem::u8path(application_path_root_utf8);

        build_file_name();
        s_isSetup = true;
        create_log_file();

        if (s_useTimestamp)
            info("Timestamps are enabled recorded as YYYY-MM-DD HH:MM:SS in UTC time.");

        log_compile_version();
    }

    void end()
    {
        if (!s_isSetup || s_fileClosed)
            return;
        s_file.close();
        s_fileClosed = true;
        s_isSetup = false;
    }
} //namespace Tools::Log
