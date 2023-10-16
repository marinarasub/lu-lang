#ifndef LU_EXPR_H
#define LU_EXPR_H

#include "adt/array.h"
#include "adt/vector.h"
#include "utility.h"
#include "string.h"
#include "source.h"
#include "token.h"
#include "internal/constexpr.h"

#include <cstddef>
//#include <limits>

namespace lu
{

struct expr;
//using expr_id = size_t;

struct expr
{
    LU_CONSTEXPR static size_t ASSIGN_TARGET_IDX = 0;
    LU_CONSTEXPR static size_t ASSIGN_RHS_IDX = 1;
    LU_CONSTEXPR static size_t CALL_CALLEE_IDX = 0;
    LU_CONSTEXPR static size_t CALL_ARGS_IDX = 1;
    LU_CONSTEXPR static size_t TYPED_VARIABLE_TYPE_IDX = 0;
    LU_CONSTEXPR static size_t FUNCTION_PARAMS_IDX = 0;
    LU_CONSTEXPR static size_t FUNCTION_BODY_IDX = 1;

    enum expr_kind
    {
        BLANK,
        //LITERAL, // str is literal text
        _label_LITERAL_FIRST,
        TRUE_LITERAL = _label_LITERAL_FIRST,
        FALSE_LITERAL,
        STRING_LITERAL,
        DECIMAL_LITERAL,
        INTEGER_LITERAL,
        _label_LITERAL_LAST = INTEGER_LITERAL,

        INTRINSIC,
        VARIABLE, // (a var's name) str is varname, expr[0] is tye if size > 0
        TYPED_VARIABLE,
        //UNARY,
        //GROUP, // group only exists as parse time.
        //BINARY,
        //TERNARY,
        BLOCK, // expr[0..] are any expr
        TUPLE, // expr[0..] are any expr
        CALL, // str is funcname, expr[0..n] are any expr
        ASSIGN, // expr[0] is target, expr[1] is 0
        FUNCTION, // expr[0] is body, expr[1] is return tye, expr[2..] are params

        // 
        PARAM, // or tuple/struct member, str is param name, expr[1] is tye, expr[2] is default-init
        DEFAULT_PARAM,
        // tyes
        NAMED_TYPE, // (a tye's name) str is name of tye
        FUNCTION_TYPE, // expr[0] is return tye, expr[1..] are arg tyes
        TUPLE_TYPE, // expr[i] is either named tye, 
        // pseudo-expr, but treat as expr for convinience
        LABEL, // nah label is expr with 0 or 1 sub.
        BRANCH,
        RETURN,
    };

    expr() : _kind(expr_kind::BLANK) {}
    expr(expr_kind kind) : _kind(kind) {}
    expr(expr_kind kind, const source_reference& sr, const source_location& loc) : _kind(kind), _srcref(sr), _loc(loc) {}
    expr(const expr&);
    expr(expr&&);
    ~expr();

    expr& operator=(expr&& other);
    expr& operator=(const expr& other);
    
    expr_kind kind() const { return _kind; }
    const source_reference& srcref() const { return _srcref; }
    const source_location& loc() const { return _loc; }

    bool is(expr_kind) const;
    template <typename ...RestT>
    bool is(expr_kind type, RestT... rest) const { return is(type) || is(rest...); }

private:
    expr_kind _kind;
    // TODO reduce to 64 bytes, maybe only allow up to max int32 lines?
    // metadata
    source_reference _srcref;
    source_location _loc;
};

expr::expr_kind token_to_expr_literal_kind(token::token_kind);

LU_CONSTEXPR bool isliteral(const expr& e)
{
    return expr::_label_LITERAL_FIRST <= e.kind() && e.kind() <= expr::_label_LITERAL_LAST;
}

LU_CONSTEXPR bool isvariable(const expr& e)
{
    return e.kind() == expr::VARIABLE || e.kind() == expr::TYPED_VARIABLE;
}

LU_CONSTEXPR bool istypedvariable(const expr& e)
{
    return e.kind() == expr::TYPED_VARIABLE;
}

LU_CONSTEXPR bool isassign(const expr& e)
{
    return e.kind() == expr::ASSIGN;
}

LU_CONSTEXPR bool iscall(const expr& e)
{
    return e.kind() == expr::CALL;
}

LU_CONSTEXPR bool istuple(const expr& e)
{
    return e.kind() == expr::TUPLE;
}

LU_CONSTEXPR bool istype(const expr& e)
{
    return e.kind() == expr::TUPLE_TYPE || e.kind() == expr::FUNCTION_TYPE || e.kind() == expr::NAMED_TYPE;
}

LU_CONSTEXPR bool isintrinsic(const expr& e)
{
    return e.kind() == expr::INTRINSIC;
}

LU_CONSTEXPR bool isfunction(const expr& e)
{
    return e.kind() == expr::FUNCTION;
}

LU_CONSTEXPR bool isblock(const expr& e)
{
    return e.kind() == expr::BLOCK;
}

LU_CONSTEXPR bool isblank(const expr& e)
{
    return e.kind() == expr::BLANK;
}

}

#endif // LU_EXPR_H
