#ifndef LU_INTERMEDIATE_H
#define LU_INTERMEDIATE_H

#include "type.h"
#include "value.h"
#include "expr.h"
#include "string.h"
#include "adt/array.h"
#include "adt/vector.h"

#include "symbol.h"
#include "scope.h" // for symbol table
#include "analyze.h"
#include "value.h"
#include "utility.h"
#include "internal/constexpr.h"
#include "intermediate_common.h"

namespace lu
{
    // using var_id = size_t;
    // using intr_id = size_t;
// post parse and post resolver represenataiton (after reducing all named references ids - types are kept and correspoinding type info in regsitery)
// pending a shorter better name.
// only executable code should exist since all static stuff should be done, so no function declarations, static exprs etc., all literals are already converted to actual type etc.
// struct decl
// {
//     symbol_id sid;
//     type_id tid;
// };



struct intermediate;

namespace diags
{
    extern diag INTERMEDIATE_INFO;
}
// struct intermediate_block
// {
// private:
//     array<symbol> _locals;
//     //symbol _ret; // return value
//     //decl* _temps; // temporaries. TODO use stack since not fixed size?

//     array<intermediate> _subs;
//     // size_t _idx; // current instruction (pc)

//     //intermediate_block* _parent; // for returning

// };

// struct intermediate_load_constant
// {
//     intermediate_value val;
// };

struct intermediate_load_symbol
{
    intermediate_load_symbol(symbol_id sid) : sid(sid) {}

    symbol_id sid;
};

// temp from last load -> sid
struct intermediate_store_symbol
{
    intermediate_store_symbol(symbol_id sid, unique<intermediate>&& eval) : sid(sid), eval(move(eval)) {}
    intermediate_store_symbol(intermediate_store_symbol&&);
    intermediate_store_symbol(const intermediate_store_symbol&);

    symbol_id sid;
    unique<intermediate> eval;
};

struct intermediate_load_constant
{
    intermediate_load_constant(intermediate_value&& val) : val(move(val)) {}

    intermediate_value val;
};

struct intermediate_intrinsic
{
    intermediate_intrinsic(intrinsic_code icode, intrinsic_id iid, symbol_id dest, symbol_id op) : icode(icode), iid(iid), dest(dest), op(op) {}

    intrinsic_code icode;
    intrinsic_id iid;
    symbol_id dest;
    symbol_id op;
};

struct intermediate_block
{
    array<symbol> locals;
    array<intermediate> subs;
};

struct intermediate_tuple
{
    intermediate_tuple(array<intermediate>&& subs) : subs(subs) {}

    array<intermediate> subs;
};

struct intermediate_call
{
    intermediate_call(intermediate_addr faddr, array<intermediate>&& args) : body(faddr), args(move(args)) {}

    intermediate_addr body; // should go to a "block"
    array<intermediate> args;
};

struct intermediate_return
{
    intermediate_addr ra;
};

struct intermediate_branch
{
    struct offset
    {
        LU_CONSTEXPR static bool NEGATIVE = true;
        LU_CONSTEXPR static bool POSITIVE = false;

        offset() : sign(POSITIVE), abs_offset(0) {}
        offset(bool sign, size_t offset) : sign(sign), abs_offset(offset) {}

        bool sign;
        size_t abs_offset;
    };

    intermediate_branch() : base(), condition(nullptr), offset() {}
    intermediate_branch(intermediate_branch&&);
    intermediate_branch(const intermediate_branch&);

    intermediate_addr base;
    unique<intermediate> condition; 
    offset offset;
    // sign? negative offset : positive offset
    
};

struct intermediate
{
    enum intermediate_op
    {
        ILLEGAL,
        LOAD_CONSTANT, // load a constant/immediate value
        LOAD_SYMBOL, // load a variable from var_id
        STORE_SYMBOL, // var_id <- variable or return register
        INTRINSIC, // a non control-flow pritimve functoin  
        BLOCK,
        TUPLE,
        CALL, // jump to intermediate, assign parameters with called intermeidates
        RETURN,
        BRANCH, // uncoditional can just be br true
        HALT,
    };

    template <typename... ArgsT> static intermediate emplace_load_constant(ArgsT&&... args) { return create_load_constant(intermediate_load_constant(forward<ArgsT>(args)...)); }
    template <typename... ArgsT> static intermediate emplace_load_symbol(ArgsT&&... args) { return create_load_symbol(intermediate_load_symbol(forward<ArgsT>(args)...)); }
    template <typename... ArgsT> static intermediate emplace_store_symbol(ArgsT&&... args) { return create_store_symbol(intermediate_store_symbol(forward<ArgsT>(args)...)); }
    template <typename... ArgsT> static intermediate emplace_intrinsic(ArgsT&&... args) { return create_intrinsic(intermediate_intrinsic(forward<ArgsT>(args)...)); }
    template <typename... ArgsT> static intermediate emplace_call(ArgsT&&... args) { return create_call(intermediate_call(forward<ArgsT>(args)...)); }
    template <typename... ArgsT> static intermediate emplace_tuple(ArgsT&&... args) { return create_tuple(intermediate_tuple(forward<ArgsT>(args)...)); }
    
    static intermediate create_load_constant(intermediate_load_constant&&);
    static intermediate create_load_symbol(intermediate_load_symbol);
    static intermediate create_store_symbol(intermediate_store_symbol&&);//...
    static intermediate create_intrinsic(intermediate_intrinsic);
    static intermediate create_call(intermediate_call&&);
    static intermediate create_tuple(intermediate_tuple&&);
    static intermediate create_halt();

    intermediate();

    intermediate(intermediate&&);
    intermediate(const intermediate&);
    ~intermediate();

    intermediate& operator=(intermediate&&);
    intermediate& operator=(const intermediate&);
    bool operator<(const intermediate&) const;

    const source_reference& srcref() const { return _srcref; }
    const source_location& loc() const { return _loc; }
    intermediate_op op() const { return _inst; }

    union
    {
        intermediate_load_constant imm;
        intermediate_load_symbol load;
        intermediate_store_symbol store;
        intermediate_intrinsic intr;
        intermediate_block blk;
        intermediate_call call;
        intermediate_tuple tup;
        intermediate_return ret;
        intermediate_branch br;
    };

private:
    source_reference _srcref;
    source_location _loc;

    intermediate_op _inst;

    
    //intermediate_block* _blk; intermediate* _args; size_t _nargs; // for call

    intermediate& create(intermediate&& other);
    intermediate& create(const intermediate& other);
    intermediate& assign(intermediate&& other);
    intermediate& assign(const intermediate& other);
    void destroy();

    // function exec(intr)
    //   if type is constant
    //      assert(FALSE);
    //  if type is variable
    //      assert(FALSE); // should be enforeced on creation that variable must not be standalone
    //  if type is assign
    //      if rhs is constant or varaible, perform assignment, else assign exec(rhs)
    //      >> return (get_decl(_var) = exec(rhs))
    //  if type is call
    //      _blk.set(args...)
    //      return exec(blk)
    //  if type is return
    //      if rhs is constant or variable, set ret to that
    //      else assign exec(rhs)
    //      scope.destroy()
    // 

};

const char* intermediate_op_cstr(intermediate::intermediate_op op);

string to_string(const intermediate&);

struct transform_except : public diag_except
{
    transform_except(const diag& d) : diag_except(d) {}
};

// struct value_table
// {
//     map<symbol_id, intermediate_value> vals;
// };

struct intermediate_context
{
    intermediate_context() {}
    intermediate_context(analyze_context&&);

    symbol_table& symbols() { return _syms; }
    type_registry& types() { return _types; }
    intermediate_value_table static_values() { return _svals; }
    const symbol_table& symbols() const { return _syms; }
    const type_registry& types() const { return _types; }
    const intermediate_value_table static_values() const { return _svals; }

private:
    type_registry _types;
    // types and
    // symbol_id -> value map
    symbol_table _syms;
    intermediate_value_table _svals;
    //value_table values;
};

enum class intermediate_transform_result
{
    INTERMEDIATE_TRANSFORM_OK,
    INTERMEDIATE_TRANSFORM_FAIL,
};

LU_CONSTEXPR bool ok(intermediate_transform_result itr)
{
    return itr == intermediate_transform_result::INTERMEDIATE_TRANSFORM_OK;
}

struct intermediate_program
{
    intermediate_program() {}
    
    void set_context(analyze_context&&);
    intermediate& operator[](intermediate_addr);
    const intermediate& operator[](intermediate_addr) const;

    intermediate_addr push(intermediate&&);
    intermediate_addr write(intermediate_addr, intermediate&&); // overwrite if needed - only use for hard-coded addresses (probably unnecessary)

    size_t size() const;

    intermediate_context& context() { return _ctxt; }
    const intermediate_context& context() const { return _ctxt; }

private:
    vector<intermediate> _insts;
    // context...
    intermediate_context _ctxt;
};

// intermed. is evaluatable:
//void evaluate(intermediate_block*, context*);

intermediate_transform_result intermediate_transform(analyze_expr_tree*, intermediate_program*, diag_logger*);

}

#endif // LU_INTERMEDIATE_H
