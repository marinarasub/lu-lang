#include "interpreter.h"

#include "internal/debug.h"
#include "except.h"
#include "internal/intermediate_printer.h"
#include "print.h"

namespace lu
{

namespace diags
{
    diag INTERPRET_ILLEGAL = diag(diag::ERROR_LEVEL, 5000);
}

void intermediate_interpreter_state::probe(symbol_id sid)
{
    if (_vals.size() <= sid)
    {
        _vals.resize(sid + 1);
    }
}

intermediate_value& intermediate_interpreter_state::operator[](symbol_id sid)
{
    assert(sid < _vals.size());

    return _vals[sid];
}

const intermediate_value& intermediate_interpreter_state::operator[](symbol_id sid) const
{
    assert(sid < _vals.size());

    return _vals[sid];
}

namespace internal
{
    struct interpreter
    {
        interpreter(const intermediate_program* ip, intermediate_interpreter_state* is, intermediate_addr iaddr, diag_logger* log)
            : p_ip(ip), p_state(is), iaddr(iaddr), p_log(log), printer(ip) {}

        const intermediate_program* p_ip;
        intermediate_interpreter_state* p_state;
        intermediate_addr iaddr;
        diag_logger* p_log;
        intermediate_printer printer;

        diag_context make_intermediate_illegal(intermediate_addr iaddr, const intermediate& i)
        {
            return diag_context(
                diags::INTERPRET_ILLEGAL,
                i.srcref(),
                i.loc(),
                string::join("illegal intermediate: ", printer.print(iaddr, i))
            );
        }

        intermediate_value& get(symbol_id sid)
        {
            return (*p_state)[sid];
        }

        const intermediate_value& get(symbol_id sid) const
        {
            return (*p_state)[sid];
        }

        bool stop() const
        {
            return iaddr >= p_ip->size();
        }

        const intermediate& curr() const
        {
            return (*p_ip)[iaddr];
        }

        void advance()
        {
            ++iaddr;
        }

        void throw_diag(const diag_context& dc)
        {
            p_log->push(dc);
            throw interpret_except(dc.dg);
        }

        void invoke_intrinsic(const intermediate_intrinsic& intr)
        {
            switch (intr.icode)
            {
            case I32PRINT:
                print(to_string(get(intr.op).bin.i32));
                break;
            case I64PRINT:
                print(to_string(get(intr.op).bin.i64));
                break;
            case U32PRINT:
                print(to_string(get(intr.op).bin.u32));
                break;
            case U64PRINT:
                print(to_string(get(intr.op).bin.u64));
                break;
            case I32ADD:
                get(intr.dest).bin.i32 += get(intr.op).bin.i32;
                break;
            case I64ADD:
                get(intr.dest).bin.i64 += get(intr.op).bin.i64;
                break;
            case U32ADD:
                get(intr.dest).bin.u32 += get(intr.op).bin.u32;
                break;
            case U64ADD:
                get(intr.dest).bin.u64 += get(intr.op).bin.u64;
                break;
            case BPRINT:
                print(get(intr.op).bin.b ? "true" : "false");
                break;
            case ASCIIPRINT:
                putascii(get(intr.op).bin.ascii);
                break;
            case LNEG:
                get(intr.dest).bin.b = !get(intr.dest).bin.b;
                break;
            // case LAND:
            //     return "LAND";
            // case LOR:
            //     return "LOR";
            default:
                throw internal_except_todo();
            }
        }

        intermediate_value interpret_intermediate_eval(const intermediate& intm)
        {
            switch (intm.op())
            {
            case intermediate::ILLEGAL:
                throw_diag(make_intermediate_illegal(iaddr, intm));
            case intermediate::LOAD_CONSTANT:
                return intm.imm.val;
            case intermediate::LOAD_SYMBOL:
                return (*p_state)[intm.load.sid];
            case intermediate::STORE_SYMBOL:
                throw internal_except_todo();
            case intermediate::INTRINSIC:
                throw internal_except_todo();
            case intermediate::BLOCK:
                throw internal_except_todo();
            case intermediate::TUPLE:
                throw internal_except_todo();
            case intermediate::CALL:
                throw internal_except_todo();
            case intermediate::RETURN:
                throw internal_except_todo();
            case intermediate::BRANCH:
                throw internal_except_todo();
            default:
                throw internal_except_unhandled_switch(intermediate_op_cstr(intm.op()));
            }
        }

        void interpret_intermediate_top(const intermediate& intm)
        {
            switch (intm.op())
            {
            case intermediate::ILLEGAL:
                throw_diag(make_intermediate_illegal(iaddr, intm));
            case intermediate::LOAD_CONSTANT:
                throw internal_except_todo();
            case intermediate::LOAD_SYMBOL:
                throw internal_except_todo();
            case intermediate::STORE_SYMBOL:
                p_state->probe(intm.store.sid);
                (*p_state)[intm.store.sid] = interpret_intermediate_eval(*intm.store.eval);
                break;
            case intermediate::INTRINSIC:
                invoke_intrinsic(intm.intr);
                break;
            case intermediate::BLOCK:
                throw internal_except_todo();
            case intermediate::TUPLE:
                throw internal_except_todo();
            case intermediate::CALL:
                throw internal_except_todo();
            case intermediate::RETURN:
                throw internal_except_todo();
            case intermediate::BRANCH:
                throw internal_except_todo();
            default:
                throw internal_except_unhandled_switch(intermediate_op_cstr(intm.op()));
            }
        }

        void interpret_next()
        {
            interpret_intermediate_top(curr());
            advance();
        }

        void interpret()
        {
            if (stop())
            {
                return;
            }
            interpret_next();
        }
    };
};

interpret_result interpret(const intermediate_program* ip, intermediate_interpreter_state* is, intermediate_addr iaddr, diag_logger* log)
{
    interpret_result res = interpret_result::INTERPRET_OK;

    internal::interpreter itpr(ip, is, iaddr, log);
    while (!itpr.stop())
    {
        try
        {
            itpr.interpret();
        }
        catch(const interpret_except& e)
        {
            res = interpret_result::INTERPRET_FAIL;
            if (log->fatal(e.dg))
            {
                break;
            }
            itpr.advance();
        }
    }
    return res;
}

}