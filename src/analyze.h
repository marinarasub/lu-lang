#ifndef LU_ANALYZE_H
#define LU_ANALYZE_H

#include "expr.h"
#include "parse.h"
#include "diag.h"
#include "type.h"
#include "scope.h"
#include "symbol.h"
#include "flag.h"
#include "adt/array.h"
#include "internal/constexpr.h"
#include "value.h"
#include <cstdint>

namespace lu
{

namespace diags
{
    extern diag ANALYZE_NOT_CONVERTIBLE;
    extern diag ANALYZE_EXPR_INFO;
    extern diag ANALYZE_CALL_ARITY_MISMATCH;
}

enum class eval_flag : uint32_t
{
    STATIC = (1 << 1),
};

struct analyze_except : public diag_except
{
    analyze_except(const diag& d) : diag_except(d) {}
};

struct analyze_context
{
    symbol_table& symbols() { return _syms; }
    type_registry& types() { return _types; }
    intermediate_value_table& static_values() { return _svals; }
    const symbol_table& symbols() const { return _syms; }
    const type_registry& types() const { return _types; }
    const intermediate_value_table& static_values() const { return _svals; }

private:
    symbol_table _syms;
    type_registry _types;
    intermediate_value_table _svals;
    // TODO type conversions


};

struct analyze_expr : public expr
{
    analyze_expr();
    analyze_expr(expr::expr_kind);
    ~analyze_expr();
    
    static analyze_expr create_vanilla(const parse_expr&, type_id, array<analyze_expr>&& subs = {});
    static analyze_expr create_hassymbol(const parse_expr&, type_id, symbol_id, array<analyze_expr>&& subs = {});
    static analyze_expr create_hasintrinsic(const parse_expr&, type_id, intrinsic_id); // TODO convert constructors to staticfunctions to make them more clear
    static analyze_expr create_hastype(const parse_expr&, type_id, type_id); // for type expr, last type_id is the type_id being expressed, other two will be TYPEID which is type of a type expr itself.
    // TODO ubs soncstructor

    symbol_id sid() const;
    intrinsic_id iid() const;
    type_id tid() const;

    void set_eval_type(type_id tid) { _etid = tid; }

    type_id base_type() const { return _btid; }
    type_id eval_type() const { return _etid; }
    string_view text() const { return _text; }

    size_t arity() const { return _subs.size(); }
    bool empty() const { return _subs.size() == 0; }

    analyze_expr& operator[](size_t idx) { return _subs[idx]; }
    const analyze_expr& operator[](size_t idx) const { return _subs[idx]; }

    // TODO flags setter
    
private:
    analyze_expr(const parse_expr&);

    void destroy();

    //parse_expr e;
    string _text; // same as parse_expr
    type_id _btid;
    type_id _etid; // what type will expr eval to?
    union
    {
        type_id _tid;
        symbol_id _sid; // which symbol does this node refer to? (or none
        intrinsic_id _iid;
    };
    
    flags<eval_flag> _flags;
    
    array<analyze_expr> _subs;
};

struct analyze_expr_tree
{
    analyze_expr_tree();

    analyze_expr& operator[](size_t idx) { return _top_exprs[idx]; }
    const analyze_expr& operator[](size_t idx) const { return _top_exprs[idx]; }

    size_t size() const;

    analyze_expr& push_top_expr(analyze_expr&&);
    analyze_context& context() { return _ctxt; }
    const analyze_context& context() const { return _ctxt; }
private:
    vector<analyze_expr> _top_exprs;
    analyze_context _ctxt;
};
    
enum class analyze_result
{
    ANALYZE_OK,
    ANALYZE_FAIL,
};

LU_CONSTEXPR bool ok(analyze_result ar)
{
    return ar == analyze_result::ANALYZE_OK;
}

analyze_result analyze(const parse_expr_tree*, analyze_expr_tree*, diag_logger*);

// string to_string(const analyze_expr& e);
// string to_string(const analyze_expr_tree& et);

}


#endif // LU_ANALYZE_H
