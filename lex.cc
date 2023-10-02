#include "lex.h"

#include "string.h"
#include "token.h"
#include "adt/map.h"

// TODO replace with encoding
#include <cctype>
#include <cassert>

// TODO DEBUG include
#include <iostream>

namespace lu
{

namespace diags
{
    diag LEX_INVALID_TOKEN = diag(diag::ERROR_LEVEL, 1000);
    diag LEX_EXPECTED_RQUOTE = diag(diag::ERROR_LEVEL, 1001);
    diag LEX_TOKEN_INFO = diag(diag::DEBUG_LEVEL, 1900);
}

namespace internal
{
    struct match
    {
        size_t size;
        token::token_kind type;

        operator bool() const
        {
            return size != 0;
        }
    };

    struct keyword_map
    {
        void insert(string_view keyword, token::token_kind type)
        {
            _kws.insert(keyword, type);
        }

        match match_longest(const string_view& sv) const
        {
            for (const auto& kv : _kws)
            {
                if (sv.subview(0, kv.key.size()) == kv.key)
                {
                    return { kv.key.size(), kv.value };
                }
            }
            return { 0, token::STOP }; // type doens't matter since no match
        }

    private:
        using map_type = flat_map<string_view, token::token_kind>;
        map_type _kws;
    };

    struct lexer
    {
        lexer(source_reference* p_srcref, source_location* p_loc, const keyword_map* p_kws, diag_logger* p_log) : p_srcref(p_srcref), p_loc(p_loc), p_kws(p_kws), p_log(p_log) {}

        source_reference* p_srcref;
        source_location* p_loc;
        const keyword_map* p_kws;
        diag_logger* p_log;

        string_view until_whitespace(size_t ahead = 0)
        {
            size_t start_pos = p_srcref->pos() + ahead;
            size_t end_pos = start_pos;
            while (end_pos < p_srcref->src().size() && !std::isspace((p_srcref->src())[end_pos]))
            {
                end_pos++;
            }
            return p_srcref->src().read(start_pos, end_pos - start_pos);
        }
        
        string_view until_eol(size_t ahead = 0)
        {
            size_t start_pos = p_srcref->pos() + ahead;
            size_t end_pos = start_pos;
            while (end_pos < p_srcref->src().size() && ((p_srcref->src())[end_pos] != '\n'))
            {
                end_pos++;
            }
            return p_srcref->src().read(start_pos, end_pos - start_pos);
        }

        diag_context make_invalid_token()
        {
            return diag_context(
                diags::LEX_INVALID_TOKEN,
                p_srcref->subref(0, until_eol().size()), // TODO surrounding context
                *p_loc,
                move(string("invalid token '").append(until_whitespace()).append("'")) // did you mean?
            );
        }

        diag_context make_expected_rquote()
        {
            return diag_context(
                diags::LEX_EXPECTED_RQUOTE,
                p_srcref->subref(0, until_eol().size()), // TODO surrounding context
                *p_loc,
                move(string("expected '\"' to match ").append(until_whitespace()))
            );
        }

        diag_context make_lex_token_info(const token& t)
        {
            return diag_context(diags::LEX_TOKEN_INFO, t.srcref(), t.loc(), move(string("lexed token: ").append(to_string(t))));
        }

        token produce(token::token_kind type, size_t len)
        {
            token t = token(type, p_srcref->subref(0, len), *p_loc);
            p_loc->advance(len);
            p_srcref->advance(len);
            return t;
        }

        token newline()
        {
            token t = token(token::NEWLINE, p_srcref->subref(0, 1), *p_loc);
            p_loc->newline();
            p_srcref->advance(1);
            return t;
        }

        token stop()
        {
            return token(token::STOP, p_srcref->subref(0, 0), *p_loc);
        }

        bool isend(size_t ahead = 0) const
        {
            return p_srcref->pos() + ahead >= p_srcref->src().size();
        }

        string::CharT ahead(size_t n) const
        {
            assert(!isend(n));

            return (p_srcref->src())[p_srcref->pos() + n];
        }

        string_view rest() const
        {
            assert(!isend(0));

            return p_srcref->src().read(p_srcref->pos());
        }

        match match_comment()
        {
            //assert(!pos.empty());
            //assert(pos.current() == '#');
            if (isend(0) || ahead(0) != '#') return { 0, token::COMMENT };

            size_t peek = 1;
            while (!isend(peek))
            {
                // newline ends comment
                if (ahead(peek) == '\n')
                {
                    break;
                }
                ++peek;
            }
            return { peek, token::COMMENT };
        }

        match match_whitespace()
        {
            // assume source.current() starts comment with space
            //assert(!pos.empty());
            //assert(std::isspace(pos.current()));

            size_t peek = 0;
            while (!isend(peek))
            {
                // any non space or newline ends 
                if (ahead(peek) == '\n' || !std::isspace(ahead(peek)))
                {
                    break;
                }
                ++peek;
            }
            return { peek, token::WHITESPACE };
        }

        match match_intrinsic()
        {
            if (isend(0) || ahead(0) != '$') return { 0, token::INTRINSIC };
            if (isend(1) || !(std::isalnum(ahead(1)) || ahead(1) == '_')) return { 0, token::INTRINSIC };

            size_t peek = 2;
            while (!isend(peek))
            {
                // match ([0-9a-Z]|_)*
                if (!(std::isalnum(ahead(peek)) || ahead(peek) == '_'))
                {
                    break;
                }
                ++peek;
            }
            return { peek, token::INTRINSIC };
        }

        match match_label()
        {
            if (isend(0) || ahead(0) != '@') return { 0, token::LABEL };
            if (isend(1) || !(std::isalnum(ahead(1)) || ahead(1) == '_')) return { 0, token::LABEL };

            size_t peek = 2;
            while (!isend(peek))
            {
                // match ([0-9a-Z]|_)*
                if (!(std::isalnum(ahead(peek)) || ahead(peek) == '_'))
                {
                    break;
                }
                ++peek;
            }
            return { peek, token::LABEL };
        }

        match match_string_literal()
        {
            if (isend(0) || ahead(0) != '"') return { 0, token::STRING_LITERAL };

            size_t peek = 1;
            while (!isend(peek))
            {
                // TODO idk what i was doing here
                // match ".*" (except double quote (implicitly end), newline - ignore)
                if (ahead(peek) == '\n')
                {
                    ++peek;
                    continue;
                }
                if (ahead(peek) == '"')
                {
                    return { ++peek, token::STRING_LITERAL };
                }
                ++peek;
            }
            p_loc->advance(peek);
            p_srcref->advance(peek);
            throw lex_except(p_log->push(make_expected_rquote()));
        }

        match match_integer_literal()
        {
            // TODO hex, bin etc.
            size_t peek = 0;
            while (!isend(peek))
            {
                // match [0-9]*
                if (!std::isdigit(ahead(peek)))
                {
                    break;
                }
                ++peek;
            }
            return { peek, token::INTEGER_LITERAL };
        }

        match match_numeric_literal()
        {
            // TODO sci notation
            // match [0-9]*(.[0-9]+)
            size_t peek = 0;
            while (!isend(peek))
            {
                // match [0-9]*
                if (!std::isdigit(ahead(peek)))
                {
                    break;
                }
                ++peek;
            }

            // match .[0=9], if not return integer
            if (!(!isend(peek + 1) && ahead(peek) == '.' && std::isdigit(ahead(peek + 1))))
            {
                return { peek, token::INTEGER_LITERAL };
            }
            peek += 2;

            while (!isend(peek))
            {
                // match [0-9]*
                if (!std::isdigit(ahead(peek)))
                {
                    break;
                }
                ++peek;
            }
            return { peek, token::DECIMAL_LITERAL };
        }

        // ([a-Z]|_)([0-9]|[a-Z]|_)*
        match match_identifier()
        {
            if (isend(0) || !(std::isalpha(ahead(0)) || ahead(0) == '_')) return { 0, token::IDENTIFIER };

            size_t peek = 1;
            while (!isend(peek))
            {
                if (!(std::isalnum(ahead(peek)) || ahead(peek) == '_'))
                {
                    break;
                }
                ++peek;
            }
            return { peek, token::IDENTIFIER };
        }

        token scan_next()
        {
            if (isend()) return stop();

            string::CharT c = ahead(0);

            // swtich matches:
            switch (c)
            {
            case '\n':
                return newline();
            case '(':
                return produce(token::LEFT_PARENTHESIS, 1);
            case ')':
                return produce(token::RIGHT_PARENTHESIS, 1);
            case '{':
                return produce(token::LEFT_BRACE, (1));
            case '}':
                return produce(token::RIGHT_BRACE, (1));
            case ',':
                return produce(token::COMMA, (1));
            case ';':
                return produce(token::SEMICOLON, (1));
            case '=':
                return produce(token::EQUAL, (1));
            case '.':
            {
                if (!isend(1) && ahead(1) == '.')
                {
                    size_t i = 1;
                    while (!isend(i) && ahead(i) == '.') ++i;
                    return produce(token::ELIPSIS, (i));
                }
                return produce(token::DOT, (1));
            }
            case ':':
            {
                if (!isend(1) && ahead(1) == '=')
                {
                    return produce(token::COLON_EQUAL, (2));
                }
                else
                {
                    return produce(token::COLON, (1));
                }
            }
            case '<':
            {
                if (!isend(1) && ahead(1) == '-')
                {
                    return produce(token::BACKWARD_ARROW, (2));
                }
                else
                {
                    return produce(token::LESS, (1));
                }
            }
            case '-':
            {
                if (!isend(1) && ahead(1) == '>')
                {
                    return produce(token::FORWARD_ARROW, (2));
                }
                else
                {
                    return produce(token::MINUS, (1));
                }
            }
            case '+':
            {
                return produce(token::PLUS, (1));
            }
            default:
                break;
            }

            //  non-switch matches:
            internal::match match;
            if ((match = match_comment()));
            else if ((match = match_whitespace()));
            else if ((match = match_intrinsic()));
            else if ((match = match_label()));
            else if ((match = match_string_literal()));
            else if ((match = match_numeric_literal()));
            else if ((match = match_identifier())) {}

            // keyword will override if same length or longer as another match
            internal::match matchkw = p_kws->match_longest(rest());
            if (matchkw.size >= match.size)
            {
                match = matchkw;
            }

            if (!match)
            {
                // sync to whitespace
                p_loc->advance(until_whitespace().size());
                p_srcref->advance(until_whitespace().size());
                throw lex_except(p_log->push(make_invalid_token()));
            }
            return produce(match.type, (match.size));
        }

        token scan()
        {
            token t = scan_next();
            p_log->push(make_lex_token_info(t));
            return t;
        }

    };
}

internal::keyword_map default_kws()
{
    internal::keyword_map kws;
    kws.insert("true", token::TRUE_LITERAL);
    kws.insert("false", token::FALSE_LITERAL);
    kws.insert("ret", token::RETURN_KEYWORD);
    kws.insert("br", token::BRANCH_KEYWORD);
    return kws;
}

 // TODO
const internal::keyword_map kws = default_kws();

token lex(source_reference* p_srcref, source_location* p_loc, diag_logger* p_log)
{
    internal::lexer lexer(p_srcref, p_loc, &kws, p_log);

    token t = lexer.scan();
    // TODO DEBUG
    return t;
}
}