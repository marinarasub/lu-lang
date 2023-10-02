#ifndef LU_VALUE_PRINTER_H
#define LU_VALUE_PRINTER_H

#include "../type.h"
#include "../value.h"
#include "../string.h"
#include "../print.h"

namespace lu
{
namespace internal
{
struct intermediate_value_printer
{
    string print(const symbol_table& syms, const type_registry& types, const intermediate_value& ival)
    {
        switch (ival.tid().tclass)
        {
        case VOID:
            return string("void");
        case UNDEFINED:
            return string("undefined");
        case LITERAL:
            return escape(ival.lit.text);
        case BUILTIN:
            return print_builtin(types.find_type(ival.tid()).bin, ival.bin);
        case FUNCTION:
        {
            return string::join("(*) ", to_string(ival.func.faddr));
        }
        case INTRINSIC:
        {
            return string::join("$", syms.find_intrinsic(ival.intr.iid).name);
        }
        case POINTER:
            throw internal_except_todo();
        case TUPLE:
        {
            string s = "(";
            size_t n = types.find_type(ival.tid()).tup.arity();
            if (n > 0)
            {
                for (size_t i = 0; i < n - 1; ++i)
                {
                    s.append(string::join(print(syms, types, ival.tup.vals[i]), ", "));
                }
                s.append(print(syms, types, ival.tup.vals[n - 1]));
            }
            s.append(")");
            return s;
        }
        case STRUCT:
            throw internal_except_todo();
        case UNION:
            throw internal_except_todo();
            //return to_string(t.un);
        case INTERSECT:
            throw internal_except_todo();
        default:
            throw internal_except_unhandled_switch(to_string(ival.tid().tclass));
        }
    }

private:
    string print_builtin(builtin_type bt, builtin_value bin)
    {
        switch (bt)
        {
        case builtin_type::ASCII:
            return escape(string_view(&bin.ascii, 1));
        case builtin_type::BOOL:
            return bin.b ? "true" : "false";
        case builtin_type::INT32:
            return to_string(bin.i32);
        case builtin_type::INT64:
            return to_string(bin.i64);
        case builtin_type::UINT32:
            return to_string(bin.u32);
        case builtin_type::UINT64:
            return to_string(bin.u64);
        default:
            throw internal_except_unhandled_switch(to_string(bt));
        }
    }

};
}
}

#endif // LU_VALUE_PRINTER_H
