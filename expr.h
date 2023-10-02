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
        VARIABLE, // (a var's name) str is varname, expr[0] is type if size > 0
        TYPED_VARIABLE,
        //UNARY,
        //GROUP, // group only exists as parse time.
        //BINARY,
        //TERNARY,
        BLOCK, // expr[0..] are any expr
        TUPLE, // expr[0..] are any expr
        CALL, // str is funcname, expr[0..n] are any expr
        ASSIGN, // expr[0] is target, expr[1] is 0
        FUNCTION, // expr[0] is body, expr[1] is return type, expr[2..] are params

        // 
        PARAM, // or tuple/struct member, str is param name, expr[1] is type, expr[2] is default-init
        DEFAULT_PARAM,
        // types
        NAMED_TYPE, // (a type's name) str is name of type
        FUNCTION_TYPE, // expr[0] is return type, expr[1..] are arg types
        TUPLE_TYPE, // expr[i] is either named type, 
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

}

#endif // LU_EXPR_H
