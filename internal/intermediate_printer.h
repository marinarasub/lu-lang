#ifndef LU_INTERMEDIATE_PRINTER_H
#define LU_INTERMEDIATE_PRINTER_H

#include "../intermediate.h"
#include "../string.h"
#include "../print.h"
#include "value_printer.h"
#include "type_printer.h"

namespace lu
{
namespace internal
{
struct intermediate_printer
{
    intermediate_printer(const intermediate_program* ip) : p_ip(ip) {}

    string print(intermediate_addr iaddr, const intermediate& i)
    {
        string s = string::join(hex(iaddr), " ",  intermediate_op_cstr(i.op()), " ", print(i));
        return s;
    }

    string print(const intermediate& i)
    {
        switch (i.op())
        {
        case intermediate::ILLEGAL:
            return "";
        case intermediate::LOAD_CONSTANT:
            return print_constant(i.imm);
        case intermediate::LOAD_SYMBOL:
            return print_load(i.load);
        case intermediate::STORE_SYMBOL:
            return print_store(i.store);
        case intermediate::INTRINSIC:
            return print_intrinsic(i.intr);
        case intermediate::BLOCK:
            return "TODO";
        case intermediate::TUPLE:
            return "TODO";
        case intermediate::CALL:
            return "TODO";
        case intermediate::RETURN:
            return "TODO";
        case intermediate::BRANCH:
            return "TODO";
        default:
            throw internal_except_unhandled_switch(intermediate_op_cstr(i.op()));
        }
    }

private:
    string print_constant(const intermediate_load_constant& imm)
    {
        return intermediate_value_printer().print(p_ip->context().symbols(), p_ip->context().types(), imm.val);
    }

    string print_intrinsic(const intermediate_intrinsic& intr)
    {
        auto config = p_ip->context().symbols().find_intrinsic(intr.iid).itype.config;
        switch (config)
        {
        case intrinsic_type::NONE:
        {
            return string::join(
                intrinsic_code_cstr(intr.icode),
                "()"
            );
        }
        case intrinsic_type::DEST_ONLY:
        {
            return string::join(
                intrinsic_code_cstr(intr.icode),
                " (",
                print_symbol(p_ip->context().symbols()[intr.dest]),
                ")"
            );
        }
        case intrinsic_type::OP_ONLY:
        {
            return string::join(
                intrinsic_code_cstr(intr.icode),
                " (",
                print_symbol(p_ip->context().symbols()[intr.op]),
                ")"
            );
        }
        case intrinsic_type::BOTH:
        {
            return string::join(
                intrinsic_code_cstr(intr.icode),
                " (",
                print_symbol(p_ip->context().symbols()[intr.dest]),
                ", ",
                print_symbol(p_ip->context().symbols()[intr.op]),
                ")"
            );
        }
        default:
            throw internal_except_unhandled_switch(to_string(config));
        }
    }

    string print_store(const intermediate_store_symbol& store)
    {
        const symbol& sym = p_ip->context().symbols()[store.sid];
        return string::join(print_symbol(sym), " <- ", print(*store.eval));
    }

    string print_load(const intermediate_load_symbol& load)
    {
        const symbol& sym = p_ip->context().symbols()[load.sid];
        return print_symbol(sym);
    }

    string print_symbol(const symbol& sym)
    {
        return string::join("#", to_string(sym.sid), " ", sym.name, " [", type_printer().print(p_ip->context().types(), sym.tid) , "]");//, hex(sym.flags.));
    }

    const intermediate_program* p_ip;
};
}
}

#endif // LU_INTERMEDIATE_PRINTER_H
