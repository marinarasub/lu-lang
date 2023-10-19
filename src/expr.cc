#include "expr.h"

#include "utility.h"
#include "string.h"
#include <cassert>

namespace lu
{
expr::~expr()
{

}

expr::expr(expr&& other) : _kind(other._kind), _srcref(move(other._srcref)), _loc(move(other._loc)) {}

expr::expr(const expr& other) : _kind(other._kind), _srcref(other._srcref), _loc(other._loc) {}

expr& expr::operator=(expr&& other)
{
    _kind = (other._kind);
    _loc = move(other._loc);
    _srcref = move(other._srcref);
    return *this;
}

expr& expr::operator=(const expr& other)
{
    _kind = (other._kind);
    _loc = (other._loc);
    _srcref = (other._srcref);
    return *this;
}

bool expr::is(expr::expr_kind kind) const
{
    return _kind == kind;
}

expr::expr_kind token_to_expr_literal_kind(token::token_kind kind)
{
    switch (kind)
    {
    case token::TRUE_LITERAL:
        return expr::TRUE_LITERAL;
    case token::FALSE_LITERAL:
        return expr::FALSE_LITERAL;
    case token::STRING_LITERAL:
        return expr::STRING_LITERAL;
    case token::DECIMAL_LITERAL:
        return expr::DECIMAL_LITERAL;
    case token::INTEGER_LITERAL:
        return expr::INTEGER_LITERAL;
    default:
        assert(kind == token::TRUE_LITERAL); // should always be false, shouldn't have reached here.
    }
    return expr::FALSE_LITERAL; // doesn't matter since should never reach this point.
}

}