#ifndef LU_LEX_H
#define LU_LEX_H

#include "source.h"
#include "token.h"
#include "internal/constexpr.h"
#include "diag.h"
#include <stdexcept>

namespace lu
{

namespace diags
{
    extern diag LEX_INVALID_TOKEN;
    extern diag LEX_EXPECTED_RQUOTE;
    extern diag LEX_TOKEN_INFO;
}

struct lex_except : public diag_except
{
    lex_except(const diag& d) : diag_except(d) {}
};    

// scan next token and update the source location to pos. of next token.
// TODO we'll worry about imports and sub sources later.
token lex(source_reference*, source_location* loc, diag_logger* log);
}

#endif // LU_LEX_H
