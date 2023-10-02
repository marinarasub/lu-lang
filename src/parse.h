#ifndef LU_PARSE_H
#define LU_PARSE_H

#include "expr.h"
#include "source.h"
#include "string.h"
#include "adt/array.h"
#include "adt/vector.h"
#include "diag.h"
#include <stdexcept>

namespace lu
{

namespace diags
{
    extern diag PARSE_EXPECTED_PRIMARY;
    extern diag PARSE_EXPECTED_RPAREN;
    extern diag PARSE_EXPECTED_RBRACE;
    extern diag PARSE_EXPECTED_EOE;
    extern diag PARSE_EXPECTED_TYPE;
    extern diag PARSE_EXPECTED_IDENTIFIER;
    extern diag PARSE_EXPECTED_TARGET;
    extern diag PARSE_EXPR_INFO;
}

struct parse_expr : public expr
{
    // enum expr_type
    // {
    //     BLANK,
    //     COMMENT,
    //     //LITERAL, // str is literal text
    //     TRUE_LITERAL,
    //     FALSE_LITERAL,
    //     STRING_LITERAL,
    //     DECIMAL_LITERAL,
    //     INTEGER_LITERAL,

    //     VARIABLE, // (a var's name) str is varname, expr[0] is type if size > 0
    //     TYPED_VARIABLE,
    //     //UNARY,
    //     //GROUP, // group only exists as parse time.
    //     //BINARY,
    //     //TERNARY,
    //     BLOCK, // expr[0..] are any expr
    //     TUPLE, // expr[0..] are any expr
    //     CALL, // str is funcname, expr[0..n] are any expr
    //     ASSIGN, // expr[0] is target, expr[1] is 0
    //     FUNCTION, // expr[0] is body, expr[1] is return type, expr[2..] are params

    //     // 
    //     PARAM, // or tuple/struct member, str is param name, expr[1] is type, expr[2] is default-init
    //     DEFAULT_PARAM,
    //     // types
    //     NAMED_TYPE, // (a type's name) str is name of type
    //     FUNCTION_TYPE, // expr[0] is return type, expr[1..] are arg types
    //     TUPLE_TYPE, // expr[i] is either named type, 
    //     // pseudo-expr, but treat as expr for convinience
    //     LABEL, // nah label is expr with 0 or 1 sub.
    //     BRANCH,
    //     RETURN,
    // };

    parse_expr() {}
    parse_expr(expr::expr_kind, const source_reference&, const source_location&, string_view text);
    parse_expr(expr::expr_kind, const source_reference&, const source_location&, string_view text, array<parse_expr>&&);
    parse_expr(parse_expr&&);
    parse_expr(const parse_expr&);

    ~parse_expr();
    
    void clear();

    parse_expr& operator=(parse_expr&&);
    parse_expr& operator=(const parse_expr&);

    string_view text() const { return _text; }
    // expr_type type() const { return _type; }
    size_t arity() const { return _subs.size(); }
    bool empty() const { return _subs.size() == 0; }

    parse_expr& operator[](size_t idx) { return _subs[idx]; }
    const parse_expr& operator[](size_t idx) const { return _subs[idx]; }

private:
    void validate() const;

    //expr_type _type;
    // TODO reduce to 64 bytes, maybe only allow up to max int32 lines?
    // metadata
    //source_reference _src;
    //source_location _loc;
    
    //string _label;
    string _text;
    array<parse_expr> _subs;
    // TODO expr id?

    // {
    //     assert(idx < _n);
    //     return _first + idx;
    // }
};
constexpr size_t SSS = sizeof(parse_expr);

// TODO do something liek is() in token types
bool assignable(const parse_expr& e);


struct parse_expr_tree
{
    parse_expr_tree() {}

    parse_expr& operator[](size_t idx) { return _top_exprs[idx]; }
    const parse_expr& operator[](size_t idx) const { return _top_exprs[idx]; }

    size_t size() const;

    parse_expr& push_top_expr(parse_expr&&);
private:
    vector<parse_expr> _top_exprs; // which ones to execute from (top level exprs)
};

string to_string(const parse_expr_tree&);


struct parse_except : public diag_except
{
    parse_except(const diag& d) : diag_except(d) {}
}; 
    // TODO make members private. this is easier for now.
// struct parsed_program
// {
//     enum parse_result
//     {
//         PARSE_OK,
//         PARSE_FAIL,
//     };

//     parsed_program() : ok(PARSE_OK) {}

//     vector<token> parse_tokens; // normal parse channel 
//     vector<token> whitespace_tokens; // whitespace channel
//     parse_expr_tree exprs;
//     parse_result ok;
// };

// thrown if following tokens are unparsable. usually is fatal but if sync() is ok and user allows, can continue parsing (although likely will cascase so not recommended)
// struct parse_except : public std::runtime_error
// {
//     parse_except() : std::runtime_error("parse_except") {}

// };

enum class parse_result
{
    PARSE_OK,
    PARSE_FAIL,
};

LU_CONSTEXPR bool ok(parse_result pr)
{
    return pr == parse_result::PARSE_OK;
}

parse_result parse(const source*, parse_expr_tree*, diag_logger* log);
}

#endif // LU_PARSE_H
