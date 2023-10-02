#ifndef LU_TOKEN_H
#define LU_TOKEN_H

#include <cstddef>
#include <algorithm>

#include "source.h"
#include "internal/constexpr.h"

namespace lu
{
// struct source_location
// {
//     LU_CONSTEXPR source_location() : pos(0), line(0), col(0) {}
//     LU_CONSTEXPR source_location(size_t pos, size_t line, size_t col) : pos(pos), line(line), col(col) {}

//     LU_CONSTEXPR source_location newline() const { return source_location(pos + 1, line + 1, 0); }
//     LU_CONSTEXPR source_location advance(size_t n) const {return source_location(pos + n, line, col + n); }

//     size_t pos;
//     size_t line;
//     size_t col;
// };

struct source_reference
{
    LU_CONSTEXPR source_reference() : _p_src(nullptr), _pos(0), _len(0) {}
    LU_CONSTEXPR source_reference(const source* p_src, size_t pos, size_t len) : _p_src(p_src), _pos(pos), _len(len) {}

    string_view name() const { return (_p_src == nullptr) ? "" : _p_src->name(); }
    string_view text() const { return (_p_src == nullptr) ? "" : _p_src->read(_pos, _len); }
    const encoding& enc() const { return _p_src->enc(); }
    const source& src() const { return *_p_src; }
    size_t pos() const { return _pos; }
    
    source_reference& advance(size_t ahead);

    LU_CONSTEXPR_IF_CXX14 source_reference subref(size_t offset, size_t len) const
    {
        return source_reference(_p_src, _pos + offset, std::min(len, _len - offset));
    }

private:
    const source* _p_src;
    size_t _pos;
    size_t _len;
};

// LOGICAL POS, LINE, COL, ie if file is included, pos will still count up
struct source_location
{
    LU_CONSTEXPR source_location() : pos(0), line(1), col(1) {}
    LU_CONSTEXPR source_location(size_t pos, size_t line, size_t col) : pos(pos), line(line), col(col) {}

    source_location& newline();
    source_location& advance(size_t n);

    size_t pos;
    size_t line;
    size_t col;
};

struct token
{
    enum token_kind
    {
        ILLEGAL,

        WHITESPACE, // \s - \n
        COMMENT, // "//"
        // compiler
        DIRECTIVE, // #
        LABEL, // @...
        // grammar/separators
        LEFT_PARENTHESIS,
        RIGHT_PARENTHESIS,
        LEFT_BRACKET,
        RIGHT_BRACKET,
        LEFT_BRACE,
        RIGHT_BRACE,
        NEWLINE,
        COLON,
        SEMICOLON,
        COMMA,
        DOT,
        DOUBLE_DOT, // .., for ranges
        ELIPSIS,// ..., for variadic
        FORWARD_ARROW,// ->
        BACKWARD_ARROW,// <-
        // fundementals
        INTRINSIC, // $...
        // op
        MINUS,
        PLUS,
        EQUAL,
        BANG,
        BANG_EQUAL,
        COLON_EQUAL,
        EQUAL_EQUAL,
        LESS,
        GREATER,
        LESS_EQUAL,
        GREATER_EQUAL,
        // literals
        _label_LITERAL_FIRST,
        STRING_LITERAL = _label_LITERAL_FIRST,
        INTEGER_LITERAL,
        DECIMAL_LITERAL,
        TRUE_LITERAL,
        FALSE_LITERAL,
        NULL_LITERAL,
        _label_LITERAL_LAST = NULL_LITERAL,
        // keywords
        //TYPE_KEYWORD,
        RETURN_KEYWORD,
        BRANCH_KEYWORD,
        // general/user
        IDENTIFIER,
        // markers
        STOP, // eof, but EOF is macro in cstdlib};
    };

    LU_CONSTEXPR token() : _type(ILLEGAL) {}
    LU_CONSTEXPR token(token_kind type, const source_reference& src, const source_location& loc) : _type(type), _sr(src), _loc(loc) {}
    LU_CONSTEXPR token(const token& other) : _type(other._type), _sr(other._sr), _loc(other._loc) {}
    token& operator=(const token& other);

    // bool isliteral() const;
    // bool isendofexpr() const;
    // bool iswhitespace() const;
    // bool iseol() const;
    // bool isstop() const;
    bool is(token_kind) const;
    template <typename ...RestT>
    bool is(token_kind type, RestT... rest) const { return is(type) || is(rest...); }

    token_kind kind() const { return _type; }
    string_view text() const { return _sr.text(); }
    const source_reference& srcref() const { return _sr; }
    const source_location& loc() const { return _loc; }
private:
    token_kind _type;
    // change to source_info with loc pos, end pos, and also name of src.
    source_reference _sr;
    source_location _loc;
};

string_view token_type_str(token::token_kind);

string to_string(source_location);

string to_string(const token& t);

constexpr size_t TTT = sizeof(token);
}

#endif // LU_TOKEN_H
