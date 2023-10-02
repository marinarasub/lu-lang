#ifndef LU_INTERPRETER_H
#define LU_INTERPRETER_H

#include "intermediate.h"
#include "symbol.h"
#include "value.h"
#include "diag.h"

#include "adt/vector.h"

namespace lu
{

namespace diags
{
    extern diag INTERPRET_ILLEGAL;
}

enum class interpret_result
{
    INTERPRET_OK,
    INTERPRET_FAIL,
};

LU_CONSTEXPR bool ok(interpret_result ir)
{
    return ir == interpret_result::INTERPRET_OK;
}

struct interpret_except : public diag_except
{
    interpret_except(const diag& d) : diag_except(d) {}
};

// TODO maybe just use vaue table value.h
struct intermediate_interpreter_state
{
    intermediate_interpreter_state() {}

    void probe(symbol_id);
    intermediate_value& operator[](symbol_id);
    const intermediate_value& operator[](symbol_id) const;

    // TODO stdout, sstdint etc.
private:
    // symbol id -> value
    vector<intermediate_value> _vals;
};

// start interpreting from givne intrusction/address
interpret_result interpret(const intermediate_program*, intermediate_interpreter_state*, intermediate_addr, diag_logger*);

}

#endif // LU_INTERPRETER_H
