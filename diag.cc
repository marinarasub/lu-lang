#include "diag.h"

#include <iostream> // TODO custosm stream? nah
#include "string.h"
#include "print.h"

namespace lu
{
// TODO move to color cout class
#if defined(_WIN32) || defined(_WIN64)

#   ifdef _MSC_VER
        __pragma( warning(disable : 5105) ) // C5105: macro expansion producing 'defined' has undefined behavior: enabled with /Zc:preprocessor, in windows.h
#   endif // _MSC_VER

#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <windows.h>

#   ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#       define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#   endif // ENABLE_VIRTUAL_TERMINAL_PROCESSING

#   define LU_ENABLE_ANSI() \
        do { \
            HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE); \
            DWORD dw; \
            GetConsoleMode(hout, &dw); \
            SetConsoleMode(hout, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING); \
        } while(0)

#else // defined(_WIN32) || defined(_WIN64)

// else these do nothing
#   define LU_ENABLE_ANSI()

#endif // defined(_WIN32) || defined(_WIN64)

#ifdef LU_ENABLE_ANSI
constexpr const string_view ANSI_STR_YELLOW = "\x1B[33m";
constexpr const string_view ANSI_STR_GREEN = "\x1B[32m";
constexpr const string_view ANSI_STR_RED = "\x1B[31m";
constexpr const string_view ANSI_STR_BLUE = "\x1B[34m";
constexpr const string_view ANSI_STR_BOLD = "\x1B[1m";
constexpr const string_view ANSI_STR_ITALICS = "\x1B[3m";
constexpr const string_view ANSI_STR_UNDERLINE = "\x1B[4m";
constexpr const string_view ANSI_STR_BLINK = "\x1B[5m";
constexpr const string_view ANSI_STR_DEFAULT_COLOR = "\x1B[39m";
constexpr const string_view ANSI_STR_RESET = "\x1B[0m";
#else
constexpr const string_view ANSI_STR_YELLOW = "";
constexpr const string_view ANSI_STR_GREEN = "";
constexpr const string_view ANSI_STR_RED = "";
constexpr const string_view ANSI_STR_BLUE = "";
constexpr const string_view ANSI_STR_BOLD = "";
constexpr const string_view ANSI_STR_ITALICS = "";
constexpr const string_view ANSI_STR_UNDERLINE = "";
constexpr const string_view ANSI_STR_BLINK = "";
constexpr const string_view ANSI_STR_DEFAULT_COLOR = "";
constexpr const string_view ANSI_STR_RESET = "";
#endif // LU_ENABLE_ANSI

enum ansi_text_color
{
    ANSI_DEFAULT_COLOR = 0,
    ANSI_RED = 1,
    ANSI_GREEN = 2,
    ANSI_YELLOW = 3,
    ANSI_BLUE = 4,
    ANSI_NO_COLOR_CHOICE
};

// flags can be | (bitwise or'd)
// reset auto inserted after printline or print
enum ansi_text_format
{
    ANSI_RESET = 0,
    ANSI_NO_FORMAT = 0x1,
    ANSI_BOLD = 0x2,
    ANSI_ITALICS = 0x4,
    ANSI_UNDERLINE = 0x8,
    ANSI_BLINK = 0x10,
    ANSI_TRIPLE_EMPHASIS = ANSI_BOLD | ANSI_ITALICS | ANSI_UNDERLINE,
};

string_view ansi_text_clr(ansi_text_color clr, bool enable)
{
    if (!enable) return "";

    switch (clr)
    {
    case ansi_text_color::ANSI_RED:
        return (ANSI_STR_RED);
    case ansi_text_color::ANSI_YELLOW:
        return (ANSI_STR_YELLOW);
    case ansi_text_color::ANSI_GREEN:
        return (ANSI_STR_GREEN);
    case ansi_text_color::ANSI_BLUE:
        return (ANSI_STR_BLUE);
    case ansi_text_color::ANSI_DEFAULT_COLOR:
        return (ANSI_STR_DEFAULT_COLOR);
    default: // default do nothing
        return "";
    }
}

string ansi_text_fmt(ansi_text_format flags, bool enable)
{
    if (!enable || (flags & ansi_text_format::ANSI_NO_FORMAT)) return "";
    if (flags == ANSI_RESET) return to_string(ANSI_STR_RESET);

    // build format string
    string fmt;
    if (flags & ansi_text_format::ANSI_BOLD) fmt.append(ANSI_STR_BOLD);
    if (flags & ansi_text_format::ANSI_ITALICS) fmt.append(ANSI_STR_ITALICS);
    if (flags & ansi_text_format::ANSI_UNDERLINE) fmt.append(ANSI_STR_UNDERLINE);
    if (flags & ansi_text_format::ANSI_BLINK) fmt.append(ANSI_STR_BLINK);
    return fmt;
}

string_view ansi_clear(bool enable)
{
    if (!enable) return "";
    return ANSI_STR_RESET;
}

string diag_logger::diag_level_string(diag_level lvl)
{
    // if (lvl >= diag::FATAL_LEVEL)
    // {
    //     return to_string(ansi_text_clr(ANSI_RED, _ansi)).append("fatal").append(ansi_clear(_ansi));
    // }
    if (lvl >= diag::ERROR_LEVEL)
    {
        return to_string(ansi_text_clr(ANSI_RED, _ansi)).append("error").append(ansi_clear(_ansi));
    }
    else if (lvl >= diag::WARN_LEVEL)
    {
        return to_string(ansi_text_clr(ANSI_YELLOW, _ansi)).append("warning").append(ansi_clear(_ansi));
    }
    else if (lvl >= diag::INFO_LEVEL)
    {
        return to_string(ansi_text_clr(ANSI_BLUE, _ansi)).append("info").append(ansi_clear(_ansi));
    }
    else // if (lvl >= DEBUG_LEVEL)
    {
        return to_string(ansi_text_clr(ANSI_DEFAULT_COLOR, _ansi)).append("debug").append(ansi_clear(_ansi));
    }
}

void diag_logger::print(const diag_context& d)
{
    std::ostream& os = d.dg.level >= diag::ERROR_LEVEL ? std::cerr : std::clog;
    string s = to_string(d.srcref.name()).append("(").append(to_string(d.loc.line)).append(", ").append(to_string(d.loc.col)).append("): ")
                .append(diag_level_string(d.dg.level)).append(" (#").append(to_string(d.dg.code)).append("): ").append(d.msg).append('\n');
                //.append(string(4, ' ')).append(d.srcref.text()).append("\n"); // TODO hgihlight and caret (if option on)
    os << s;
}

void diag_logger::flush()
{
    for (size_t i = _pending.size(); i > 0; --i)
    {
        print(pop());
    }
}

diag_context diag_logger::pop()
{
    diag_context d = _pending.front();
    _pending.pop_front();
    return d;
}

bool diag_logger::fatal(const diag& d) const
{
    return d.level > fatal_level;
}

diag diag_logger::push(const diag_context& d)
{
    if (d.dg.level >= min_level)
    {
        _pending.push_back(d);
    }
    if (fatal(d.dg)) // fatal always throws
    {
        throw diag_except(d.dg);
    }
    return d.dg;
}
}