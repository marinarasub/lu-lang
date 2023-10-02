#include "token.h"

#include "print.h"
#include <cassert>

namespace lu
{
    source_reference& source_reference::advance(size_t ahead)
    {
        assert(ahead <= _len);
        
        _pos += ahead;
        _len -= ahead;
        return *this;
    }

    source_location& source_location::newline()
    {
        //pos = pos + 1; // newline is not seen so should not count towards logical pos
        line = line + 1;
        col = 1;
        return *this;
    }

    source_location& source_location::advance(size_t n)
    {
        pos = pos + n;
        col = col + n;
        return *this;
    }

    token& token::operator=(const token& other)
    {
        this->_type = other._type;
        this->_sr = other._sr;
        this->_loc = other._loc;
        return *this;
    }

    // bool token::isliteral() const
    // {
    //     return _label_LITERAL_FIRST <= _type && _type <= _label_LITERAL_LAST;
    // }

    // bool token::isendofexpr() const
    // {
    //     // newline may not be treated as exnd of expr by parser but counts as newline if asked
    //     return _type == SEMICOLON || _type == NEWLINE || _type == STOP;
    // }

    // bool token::iswhitespace() const
    // {
    //     return _type == WHITESPACE || _type == COMMENT;
    // }

    // bool token::iseol() const
    // {
    //     return _type == NEWLINE;
    // }

    // bool token::isstop() const
    // {
    //     return _type == STOP;
    // }

    bool token::is(token_kind type) const
    {
        return _type == type;
    }

    string_view token_type_str(token::token_kind type)
    {
        switch (type)
        {
        case lu::token::WHITESPACE:
            return "WHITESPACE";
        case lu::token::COMMENT:
            return "COMMENT";
        case lu::token::DIRECTIVE:
            return "DIRECTIVE";
        case lu::token::LEFT_PARENTHESIS:
            return "LEFT_PARENTHESIS";
        case lu::token::RIGHT_PARENTHESIS:
            return "RIGHT_PARENTHESIS";
        case lu::token::LEFT_BRACKET:
            return "LEFT_BRACKET";
        case lu::token::RIGHT_BRACKET:
            return "RIGHT_BRACKET";
        case lu::token::LEFT_BRACE:
            return "LEFT_BRACE";
        case lu::token::RIGHT_BRACE:
            return "RIGHT_BRACE";
        case lu::token::NEWLINE:
            return "NEWLINE";
        case lu::token::SEMICOLON:
            return "SEMICOLON";
        case lu::token::COLON:
            return "COLON";
        case lu::token::COMMA:
            return "COMMA";
        case lu::token::DOT:
            return "DOT";
        case lu::token::FORWARD_ARROW:
            return "FORWARD_ARROW";
        case lu::token::BACKWARD_ARROW:
            return "BACKWARD_ARROW";
        case lu::token::INTRINSIC:
            return "INTRINSIC";
        case lu::token::MINUS:
            return "MINUS";
        case lu::token::PLUS:
            return "PLUS";
        case lu::token::EQUAL:
            return "EQUAL";
        case lu::token::BANG:
            return "BANG";
        case lu::token::BANG_EQUAL:
            return "BANG_EQUAL";
        case lu::token::COLON_EQUAL:
            return "COLON_EQUAL";
        case lu::token::EQUAL_EQUAL:
            return "EQUAL_EQUAL";
        case lu::token::LESS:
            return "LESS";
        case lu::token::GREATER:
            return "GREATER";
        case lu::token::LESS_EQUAL:
            return "LESS_EQUAL";
        case lu::token::GREATER_EQUAL:
            return "GREATER_EQUAL";
        case lu::token::FALSE_LITERAL:
            return "FALSE_LITERAL";
        case lu::token::TRUE_LITERAL:
            return "TRUE_LITERAL";
        case lu::token::STRING_LITERAL:
            return "STRING_LITERAL";
        case lu::token::INTEGER_LITERAL:
            return "INTEGER_LITERAL";
        case lu::token::DECIMAL_LITERAL:
            return "DECIMAL_LITERAL";
        case lu::token::RETURN_KEYWORD:
            return "RETURN_KEYWORD";
        case lu::token::IDENTIFIER:
            return "IDENTIFIER";
        case lu::token::STOP:
            return "STOP";
        default:
            return "???";
        }
    }

    string to_string(source_location loc)
    {
        return to_string("(")
            .append(to_string(loc.line))
            .append(", ")
            .append(to_string(loc.col))
            .append(")");
    }

    string to_string(const token& t)
    {
        return to_string(t.srcref().name())
            .append(to_string(t.loc()))
            .append(": ")
            .append(token_type_str(t.kind()))
            .append(": \"")
            .append(escape(t.text()))
            .append("\"");
    }
}