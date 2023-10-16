#ifndef LU_PARSE_PRINTER_H
#define LU_PARSE_PRINTER_H

#include "../parse.h"
#include "../string.h"
#include "../print.h"

namespace lu
{
namespace internal
{
struct parse_expr_printer
{
    parse_expr_printer() : _indent(0) {}

    string operator()(const parse_expr_tree& et)
    {
        string s;
        for (size_t i = 0; i < et.size(); ++i)
        {
            s.append(print(et[i])).append(";\n");
        }
        return s;
    }

    string operator()(const parse_expr& e)
    {
        return print(e);
    }

    string print(const parse_expr& e)
    {
        string s;
        switch (e.kind())
        {
        case expr::BLANK:
            return "<blank>";
        case expr::TRUE_LITERAL: /* fallthrough */
        case expr::FALSE_LITERAL: /* fallthrough */
        case expr::DECIMAL_LITERAL: /* fallthrough */
        case expr::INTEGER_LITERAL:
            return s.append(e.text());
        case expr::STRING_LITERAL:
            return s.append(escape(e.text()));
        case expr::INTRINSIC:
            return print_intrinsic(e);
        case expr::VARIABLE:
            return print_variable(e);
        case expr::TYPED_VARIABLE:
            return print_variable(e);
        // statments are one of these 3:
        case expr::LABEL:
            return s.append("lab ").append(e.text());
        case expr::BRANCH:
            return s.append("br ").append(e.text()).append(e.empty() ? "" : string(" ").append(print(e[0])));
        case expr::RETURN:
            return s.append("ret ").append(e.text()).append(e.empty() ? "" : string(" ").append(print(e[0])));
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
            return s.append(e.text()).append(": ").append(print(e[0]));
        case expr::DEFAULT_PARAM:
            return s.append(print(e[0])).append(" dflt ").append(print(e[1]));
        case expr::ASSIGN:
            // TODO assert size is 2? on constsruction
            return s.append("(").append(print(e[0])).append(" <- ").append(print(e[1])).append(")");
        case expr::FUNCTION:
            return s.append("(").append(print(e[0])).append(" -> ").append(print(e[1])).append(")");
        case expr::NAMED_TYPE:
            return s.append(e.text());
        case expr::FUNCTION_TYPE:
            return print_function_type(e);
        case expr::TUPLE_TYPE:
            return print_tuple_type(e);
        default:
            return "???";
        }
    }

private:
    string print_intrinsic(const parse_expr& e)
    {
        return e.text();
    }

    string print_tuple_type(const parse_expr& e)
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

    string print_function_type(const parse_expr& e)
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

    string print_variable(const parse_expr& e)
    {
        string s;
        s.append("(var ").append(e.text());
        if (e.arity() > 0) // has type
        {
            s.append(": ").append(print(e[0]));
        }
        s.append(")");
        return s;
    }

    string print_block(const parse_expr& e)
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

    string print_call(const parse_expr& e)
    {
        // 0 is callee, 1 is paren tuple of args
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

    string print_tuple(const parse_expr& e)
    {
        // assert type is tup
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
        s.append(")");
        return s;
    }

    string newline()
    {
        return string("\n").append(string(_indent * 4, ' '));
    }

    size_t _indent;
};

}
}

// string to_string(const parse_expr& e)
// {
//     // TODO interator for
//     return internal::parse_expr_printer()(e);
// }

// string to_string(const parse_expr_tree& et)
// {
//     // TODO interator for
//     return internal::parse_expr_printer()(et);
// }

#endif // LU_PARSE_PRINTER_H
