#ifndef LU_ANALYZE_PRINTER_H
#define LU_ANALYZE_PRINTER_H

#include "../analyze.h"
#include "../string.h"
#include "../print.h"
#include "type_printer.h"

namespace lu
{
namespace internal
{
struct analyze_expr_printer
{
    analyze_expr_printer(const analyze_expr_tree* p_aet) : _indent(0), p_aet(p_aet) {}

    string operator()(const analyze_expr_tree& et)
    {
        string s;
        for (size_t i = 0; i < et.size(); ++i)
        {
            s.append(print(et[i])).append(";\n");
        }
        return s;
    }

    string operator()(const analyze_expr& e)
    {
        return print(e);
    }

    string print(const analyze_expr& e)
    {
        //string s;
        //s.append(to_string("[type=").append(to_string(e.eval_type()))).append(" sid=").append(symbol_id_string(e.sid())).append("]: ");
        switch (e.kind())
        {
        case expr::BLANK:
            return "<blank>";
        case expr::TRUE_LITERAL: /* fallthrough */
        case expr::FALSE_LITERAL: /* fallthrough */
        case expr::STRING_LITERAL: /* fallthrough */
        case expr::DECIMAL_LITERAL: /* fallthrough */
        case expr::INTEGER_LITERAL:
            return print_literal(e);
        case expr::INTRINSIC:
            return print_intrinsic(e);
        case expr::VARIABLE:
            return print_variable(e);
        case expr::TYPED_VARIABLE:
            return print_variable(e);
        // statments are one of these 3:
        case expr::LABEL:
            return string::join("lab", e.text());
        case expr::BRANCH:
            return string::join("br", e.text(), (e.empty() ? "" : string(" ").append(print(e[0]))));
        case expr::RETURN:
            return string::join("ret ", e.text(), (e.empty() ? "" : string(" ").append(print(e[0]))));
        case expr::TUPLE:
        {
            return print_tuple(e);
        }
        case expr::BLOCK:
        {
            return print_block(e);
        }
        case expr::CALL:
        {
            return print_call(e);
        }
        case expr::PARAM:
            return string::join(e.text(), ": ", print(e[0]));
        case expr::DEFAULT_PARAM:
            return string::join(print(e[0]), " dflt ", print(e[1]));
        case expr::ASSIGN:
            // TODO assert size is 2? on constsruction
            return string::join("(", print(e[0]), " <- ", print(e[1]), ")");
        case expr::FUNCTION:
            return print_function(e);
        case expr::NAMED_TYPE:
            return string::join(e.text());
        case expr::FUNCTION_TYPE:
            return print_function_type_expr(e);
        case expr::TUPLE_TYPE:
            return print_tuple_type_expr(e);
        default:
            return "???";
        }
    }
private:

    string print_analyze_expr_type(const analyze_expr& e)
    {
        type_id tid = e.eval_type();
        return type_printer().print(p_aet->context().types(), tid);
    }

    string print_literal(const analyze_expr& e)
    {
        return string::join(escape(e.text()), " [", print_analyze_expr_type(e), "]");   
    }

    string print_function(const analyze_expr& e)
    {
        return string::join("(", print(e[0]), " -> ", print(e[1]), string::join("[", print_analyze_expr_type(e), "]"), ")");
    }

    string print_tuple_type_expr(const analyze_expr& e)
    {
        string s;
        s.append("(");
        if (e.arity() > 0)
        {
            s.append("tt ");
            size_t i = 0;
            for (; i < e.arity() - 1; ++i)
            {
                s.append(print(e[i]));
                s.append(", ");
            }
            s.append(print(e[i])); // last arg w/o trialing comma
            
        }
        s.append(")");
        return s;
    }

    string print_function_type_expr(const analyze_expr& e)
    {
        // assert arity is 2?
        string s;
        s.append("(");
        s.append(print(e[0]));
        s.append(" -> ");
        s.append(print(e[1]));
        s.append(")");
        return s;
    }
    
    string print_intrinsic(const analyze_expr& e)
    {
        return string::join(e.text(), " [", print_analyze_expr_type(e), "]");
    }

    string print_call(const analyze_expr& e)
    {
        assert(e.arity() == 2); //TODO maybe assert on construction
        // assert type is blk
        string s;
        s.append("(call ");
        s.append(print(e[0]));
        s.append(" ");
        s.append(print(e[1]));
        s.append(")");
        return s;
    }

    string print_tuple(const analyze_expr& e)
    {
        string s;
        s.append("(");
        if (e.arity() > 0)
        {
            s.append("tup ");
            size_t i = 0;
            for (; i < e.arity() - 1; ++i)
            {
                s.append(print(e[i]));
                s.append(", ");
            }
            s.append(print(e[i])); // last arg w/o trialing comma
            
        }
        s.append(" ");
        s.append(string::join("[", print_analyze_expr_type(e), "]"));
        s.append(")");
        return s;
    }

    string print_variable(const analyze_expr& e)
    {
        string s;
        s.append("(var ").append(e.text());
        if (e.arity() > 0) // has type
        {
            s.append(": ").append(print(e[0]));
        }
        s.append(" ");
        s.append(string::join("[", print_analyze_expr_type(e), "]"));
        s.append(")");
        return s;
    }

    string print_block(const analyze_expr& e)
    {
        if (e.empty()) return to_string("{}");
        // assert type is blk
        string s;
        s.append("{");
        ++_indent;
        for (size_t i = 0; i < e.arity(); ++i)
        {
            s.append(newline());
            s.append(print(e[i]));
        }
        --_indent;
        s.append(newline());
        s.append("}");
        return s;
    }

    string newline()
    {
        return string("\n").append(string(_indent * 4, ' '));
    }

    size_t _indent;
    const analyze_expr_tree* p_aet;
};

}
}

#endif // LU_ANALYZE_PRINTER_H
