#ifndef LU_DIAG_H
#define LU_DIAG_H

#include "string.h"
#include "token.h"
#include "utility.h"
#include "adt/list.h"
#include "except.h"
#include <cstddef>
//#include <limits>

namespace lu
{
    using diag_level = size_t;
    using diag_code = size_t;

    struct diag
    {
        constexpr static diag_level MAX_LEVEL = std::numeric_limits<diag_level>::max();
        constexpr static diag_level ERROR_LEVEL = 300;
        constexpr static diag_level WARN_LEVEL = 200;
        constexpr static diag_level INFO_LEVEL = 100;
        constexpr static diag_level DEBUG_LEVEL = 0;

        diag() : level(0), code(0) {}
        diag(diag_level level, diag_code code) : level(level), code(code) {}

        //bool fatal() const { return level >= FATAL_LEVEL; }

        diag_level level;
        diag_code code;
    };

    string to_string(const diag&);

    struct diag_context
    {
        diag_context(const diag& diag, const source_reference& src, const source_location& loc, string&& msg) : dg(diag), srcref(src), loc(loc), msg(move(msg)) {}

        diag dg;
        source_reference srcref;
        source_location loc;
        string msg;
    };

    void print(const diag_context&);

    struct diag_logger
    {
        diag_level min_level;
        diag_level fatal_level;
        
        // TODO ostream too
        diag_logger(diag_level lvl = diag::DEBUG_LEVEL, diag_level flvl = diag::MAX_LEVEL, bool ansi = true) : min_level(lvl), fatal_level(flvl), _ansi(ansi) {}

        void print(const diag_context&);
        void flush();
        // push from back, pop from font (queue)
        diag push(const diag_context&);
        diag_context pop();

        bool fatal(const diag&) const;
    private:
        string diag_level_string(diag_level);

        list<diag_context> _pending;
        bool _ansi;
    };

    struct diag_except : public lu_except
    {
        diag_except(const diag& d) : lu_except("see diagnostic"), dg(d) {}

        diag dg;
    };
}

#endif // LU_DIAG_H
