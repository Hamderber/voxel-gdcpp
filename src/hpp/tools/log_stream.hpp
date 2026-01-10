#pragma once

#include "hpp/tools/log.hpp"
#include <ostream>
#include <sstream>
#include <utility>

namespace Tools::Log
{
    struct Line
    {
        Level level;
        std::ostringstream oss;
        bool active = true;

        explicit Line(Level lvl) : level(lvl) {}

        Line(const Line &) = delete;
        Line &operator=(const Line &) = delete;

        Line(Line &&other) noexcept : level(other.level), oss(std::move(other.oss)), active(other.active)
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
} //namespace Tools::Log
