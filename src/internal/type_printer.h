#ifndef LU_TYPE_PRINTER_H
#define LU_TYPE_PRINTER_H

#include "../string.h"
#include "../type.h"
#include "../except.h"

namespace lu
{
namespace internal
{

struct type_printer
{
    type_printer()
    {}

    string print(const type_registry& types, const type& t)
    {
        return print_type(types, t);
    }

    string print(const type_registry& types, type_id tid)
    {
        return print_type_from_type_id(types, tid);
    }

private:
    string print_type(const type_registry& types, const type& t)
    {
        switch (t.tclass)
        {
        case VOID:
            return string("void");
        case UNDEFINED:
            return string("undefined");
        case LITERAL:
            return to_string(t.lit);
        case BUILTIN:
            return to_string(t.bin);
        case FUNCTION:
        {
            string s = "(";
            size_t nparam = t.fun.param_count();
            if (nparam > 0)
            {
                for (size_t i = 0; i < nparam - 1; ++i)
                {
                    s.append(string::join(print_type_from_type_id(types, t.fun.params[i].tid), ", ")); // TODO param name
                }
                s.append(print_type_from_type_id(types, t.fun.params[nparam - 1].tid));
            }
            s.append(")");
            return string::join(s, " -> ", print_type_from_type_id(types, t.fun.ret));
        }
        case INTRINSIC:
        {
            string s;
            // TODO pram count always 2? should it ignore undef fields (not used)
            if (t.intr.config == intrinsic_type::NONE)
            {
                // pass, s is ""
            }
            else if (t.intr.config == intrinsic_type::DEST_ONLY)
            {
                s.append(
                    print_type_from_type_id(types, t.intr.params[intrinsic_type::DEST_PARAM])
                );
            }
            else if (t.intr.config == intrinsic_type::OP_ONLY)
            {
                s.append(
                    print_type_from_type_id(types, t.intr.params[intrinsic_type::OP_PARAM])
                );
            }
            else if (t.intr.config == intrinsic_type::BOTH)
            {
                s.append(
                    print_type_from_type_id(types, t.intr.params[intrinsic_type::DEST_PARAM])
                ).append(
                    ", "
                ).append(
                    print_type_from_type_id(types, t.intr.params[intrinsic_type::OP_PARAM])
                );
            }
            return string::join("$(", s , ")");
        }
        case POINTER:
            throw internal_except_todo();
        case TUPLE:
        {
            string s = "(";
            size_t n = t.tup.arity();
            if (n > 0)
            {
                for (size_t i = 0; i < n - 1; ++i)
                {
                    const tuple_type::member& mem = t.tup[i];
                    string field = mem.name.empty() ? "" : string::join(mem.name, ": ");
                    s.append(string::join(field,  print_type_from_type_id(types, mem.tid), ", "));
                }
                const tuple_type::member& mem = t.tup[n - 1];
                string field = mem.name.empty() ? "" : string::join(mem.name, ": ");
                s.append(string::join(field,  print_type_from_type_id(types, mem.tid)));
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
            throw internal_except_unhandled_switch(to_string(t.tclass));
        }
    }

    string print_type_from_type_id(const type_registry& types, type_id tid)
    {   
        const type& t = types.find_type(tid);
        return print_type(types, t);
    }

};


}
}
#endif // LU_TYPE_PRINTER_H
