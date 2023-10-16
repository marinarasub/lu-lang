#include "parse.h"

#include "adt/array.h"
#include "adt/vector.h"
#include "utility.h"
#include "lex.h"
#include "source.h"
#include "string.h"
#include "expr.h"
#include "internal/parse_printer.h"

namespace lu
{
namespace diags
{
    diag PARSE_EXPECTED_PRIMARY = diag(diag::ERROR_LEVEL, 2000);
    diag PARSE_EXPECTED_RPAREN = diag(diag::ERROR_LEVEL, 2001);
    diag PARSE_EXPECTED_RBRACE = diag(diag::ERROR_LEVEL, 2002);
    diag PARSE_EXPECTED_EOE = diag(diag::ERROR_LEVEL, 2003);
    diag PARSE_EXPECTED_TYPE = diag(diag::ERROR_LEVEL, 2004);
    diag PARSE_EXPECTED_IDENTIFIER = diag(diag::ERROR_LEVEL, 2005);
    diag PARSE_EXPECTED_TARGET = diag(diag::ERROR_LEVEL, 2006);
    diag PARSE_EXPR_INFO = diag(diag::DEBUG_LEVEL, 2900);
}

void parse_expr::validate() const
{
    switch (kind())
    {
    case BLANK:
    {
        assert(_subs.size() == 0);
        break;
    }
    case TRUE_LITERAL:
    {
        assert(_subs.size() == 0);
        break;
    }
    case FALSE_LITERAL:
    {
        assert(_subs.size() == 0);
        break;
    }
    case STRING_LITERAL:
    {
        assert(_subs.size() == 0);
        break;
    }
    case DECIMAL_LITERAL:
    {
        assert(_subs.size() == 0);
        break;
    }
    case INTEGER_LITERAL:
    {
        assert(_subs.size() == 0);
        break;
    }
    case VARIABLE:
    {
        assert(_text.size() > 0);
        assert(_subs.size() == 0);
        break;
    }
    case TYPED_VARIABLE:
    {
        assert(_text.size() > 0);
        assert(_subs.size() == 1);
        break;
    }
    case BLOCK:
    {
        break;
    }
    case TUPLE:
    {
        break;
    }
    case CALL:
    {
        assert(_subs.size() > 0);
        break;
    }
    case ASSIGN:
    {
        assert(_subs.size() == 2);
        break;
    }
    case FUNCTION:
    {
        assert(_subs.size() == 2);
        break;
    }
    case PARAM:
    {
        assert(_text.size() > 0);
        assert(_subs.size() == 1);
        break;
    }
    case DEFAULT_PARAM:
    {
        assert(_subs.size() == 2);
        break;
    }
    case NAMED_TYPE:
    {
        assert(_text.size() > 0);
        assert(_subs.size() == 0);
        break;
    }
    case FUNCTION_TYPE:
    {
        assert(_subs.size() == 2);
        break;
    }
    case TUPLE_TYPE:
    {
        break;
    }
    case LABEL:
    {
        assert(_text.size() > 0);
        assert(_subs.size() == 0);
        break;
    }
    case BRANCH:
    {
        assert(_subs.size() == 1); // blank or a return parse_expr
        break;
    }
    case RETURN:
    {
        assert(_subs.size() == 1);
        break;
    }
    default:
        break;
    }
}

// TODO VALIDATE ON CONSTRUCTION THAT SIZE MAKES SENSE FOR TYPE
parse_expr::parse_expr(expr_kind type, const source_reference& sr, const source_location& loc, string_view text) : parse_expr(type, sr, loc, text, array<parse_expr>())
{}

parse_expr::parse_expr(expr_kind type, const source_reference& sr, const source_location& loc, string_view text, array<parse_expr>&& subs) : expr(type, sr, loc), _text(to_string(text)), _subs(move(subs))
{
    validate();
}

parse_expr::parse_expr(parse_expr&& other) : expr(move(other)), _text(move(other._text)), _subs(move(other._subs))
{}

parse_expr::parse_expr(const parse_expr& other) : expr(other), _text(other._text), _subs(other._subs)
{}

parse_expr::~parse_expr() {}

void parse_expr::clear()
{
    _subs.clear();
}

parse_expr& parse_expr::operator=(parse_expr&& other)
{
    expr::operator=(move(other));
    _text = (move(other._text));
    _subs = (move(other._subs));
    return *this;
}

parse_expr& parse_expr::operator=(const parse_expr& other)
{
    expr::operator=(other);
    _text = (other._text);
    _subs = (other._subs);
    return *this;
}

bool tuple_assignable(const parse_expr& e)
{
    for (size_t i = 0; i < e.arity(); ++i)
    {
        if (!assignable(e[i])) return false;
    }   
    return true;
}

bool assignable(const parse_expr& e)
{
    return e.kind() == expr::VARIABLE ||
        e.kind() ==  expr::TYPED_VARIABLE ||
        ((e.kind() == expr::TUPLE) && tuple_assignable(e));
}


size_t parse_expr_tree::size() const
{
    return _top_exprs.size();
}

parse_expr& parse_expr_tree::push_top_expr(parse_expr&& e)
{
    _top_exprs.push_back(move(e));
    return _top_exprs.back();
}


// TODO FOR BOTH parse and lex, sync before throwing to leave location in valid state - parsing the whole file and error handling should be done externally

namespace internal
{
    bool isliteral(const token& t)
    {
        return token::_label_LITERAL_FIRST <= t.kind() && t.kind() <= token::_label_LITERAL_LAST;
    }

    bool isendofexpr(const token& t)
    {
        // newline may not be treated as exnd of expr by parser but counts as newline if asked
        return t.is(token::SEMICOLON, token::NEWLINE, token::STOP);
    }

    bool iswhitespace(const token& t)
    {
        return t.is(token::WHITESPACE, token::COMMENT);
    }

    bool iseol(const token& t)
    {
        return t.is(token::NEWLINE);
    }

    bool isstop(const token& t)
    {
        return t.is(token::STOP);
    }

    struct parser
    {
        parser(source_reference* p_srcref, parse_expr_tree* p_exprs, source_location* p_loc, diag_logger* p_log) : p_srcref(p_srcref), p_exprs(p_exprs), p_loc(p_loc), p_log(p_log)
        {
            // expr_srcref = *p_srcref;
            // expr_loc = *p_loc;
            // get first parse token
            nexttok();
        }

        //const source* p_src;
        source_reference* p_srcref;
        parse_expr_tree* p_exprs;
        source_location* p_loc;
        diag_logger* p_log;
        // cache the ref and loc of start of expr.
        source_reference expr_srcref;
        source_location expr_loc;
        //
        token curr;
        token next; // our 1 lookahead

        diag_context make_expected_primary()
        {
            return diag_context(
                diags::PARSE_EXPECTED_PRIMARY,
                next.srcref(), // TODO surrounding context
                next.loc(),
                move(string("expected primary-expr after '").append(curr.text()).append("'"))
            );
        }

        diag_context make_expected_rparen()
        {
            return diag_context(
                diags::PARSE_EXPECTED_RPAREN,
                next.srcref(), // TODO surrounding context
                next.loc(),
                move(string("expected closing ')' after '").append(curr.text()).append("'"))
            );
        }

        diag_context make_expected_rbrace()
        {
            return diag_context(
                diags::PARSE_EXPECTED_RBRACE,
                next.srcref(), // TODO surrounding context
                next.loc(),
                move(string("expected closing '}' after '").append(curr.text()).append("'"))
            );
        }

        diag_context make_expected_eoe()
        {
            return diag_context(
                diags::PARSE_EXPECTED_EOE,
                next.srcref(), // TODO surrounding context
                next.loc(),
                move(string("expected terminating ';' or newline after '").append(curr.text()).append("'"))
            );
        }

        diag_context make_expected_kind()
        {
            return diag_context(
                diags::PARSE_EXPECTED_TYPE,
                next.srcref(), // TODO surrounding context
                next.loc(),
                move(string("expected a type after '").append(curr.text()).append("'"))
            );
        }

        diag_context make_expected_identifer()
        {
            return diag_context(
                diags::PARSE_EXPECTED_IDENTIFIER,
                next.srcref(), // TODO surrounding context
                next.loc(),
                move(string("expected a identifier before '").append(curr.text()).append("'"))
            );
        }

        diag_context make_expected_target()
        {
            return diag_context(
                diags::PARSE_EXPECTED_TARGET,
                next.srcref(), // TODO surrounding context
                next.loc(),
                move(string("expected an assignable target before '").append(curr.text()).append("'"))
            );
        }

        diag_context make_parse_expr_info(const parse_expr& e)
        {
            return diag_context(
                diags::PARSE_EXPR_INFO,
                e.srcref(),
                e.loc(),
                move(string("parsed expression: ").append(internal::parse_expr_printer()(e)))
            );
        }

        // diag make_expected_params()
        // {
        //     return diag(
        //         EXPECTED_PARAMS_LEVEL,
        //         1008,
        //         next.src(), // TODO surrounding context
        //         next.loc(),
        //         move(string("expected parameter list before '").append(curr.src().text).append("'"))
        //     );
        // }
    
        bool stop()
        {
            return next.is(token::STOP);
        }

        void nexttok()
        {
            next = lex(p_srcref, p_loc, p_log);
            if (iswhitespace(next))
            {
                //result.whitespace_tokens.push_back(next);
                nexttok();
            }
        }
        // we should never parse whitespace get next token until non whitespace.
        void advance()
        {
            curr = next;
            //result.parse_tokens.push_back(curr);
            nexttok();
        }

        void sync()
        {
            while (!stop())
            {
                if (accept(isendofexpr))
                {
                    break;
                }
                advance();
            }
        }

        bool check() { return false; }
        // check is useful for optional grammar, i.e. if (!check(after)) { optional() }; after
        template <typename PredT, typename ...RestT>
        bool check(PredT p, RestT... r)
        {
            return p(next) || check(r...);
        }

        template <typename ...RestT>
        bool check(token::token_kind type, RestT... r)
        {
            return next.is(type) || check(r...);
        }

        // accept is useful determening how to proceed with curr, i.e. the lookahead.
        template <typename ...CheckT>
        bool accept(CheckT... types)
        {
            if (check(types...))
            {
                advance();
                return true;
            }
            return false;
        }

        // TODO diag msg as param. Expect is useful for terminal grammar.
        template <typename ...AcceptT>
        void expect(const diag_context& dg, AcceptT... types)
        {
            if (!accept(types...))
            {
                throw parse_except(p_log->push(dg));
            }
        }

        void ignore_eol()
        {
            while (accept(iseol));
        }

        string literal_text(const token& t)
        {
            string_view src_text = t.text();
            if (t.is(token::STRING_LITERAL))
            {
                assert(src_text.size() >= 2);
                assert(src_text[0] == '\"');
                assert(src_text[src_text.size() - 1] == '\"');

                return unescape(curr.text().subview(1, src_text.size() - 2));
            }
            else
            {
                return src_text;
            }
        }

        // TODO srcref over whole expr, not just current token
        parse_expr produce_blank()
        {
            return parse_expr(expr::BLANK, expr_srcref, expr_loc, "");
        }

        parse_expr produce_term(expr::expr_kind kind, string_view text)
        {
            return parse_expr(kind, expr_srcref, expr_loc, text);
        }

        parse_expr produce_nary(expr::expr_kind kind, string_view text, array<parse_expr>&& subs)
        {
            return parse_expr(kind, expr_srcref, expr_loc, text, move(subs));
        }

        parse_expr& make_top(parse_expr&& e)
        {
            return p_exprs->push_top_expr(move(e));
        }

        // like a varaible, but no name is type deafult isntead of name

        parse_expr primary_kind()
        {
            if (accept(token::IDENTIFIER))
            {
                return produce_term(expr::NAMED_TYPE, curr.text());
            }
            if (accept(token::LEFT_PARENTHESIS))
            {
                return paren_tuple_kind();
            }
            throw parse_except(p_log->push(make_expected_kind()));
        }

        // param is type or typename: type
        parse_expr param()
        {
            parse_expr lhs = kind();
            if (accept(token::COLON))
            {
                // if colon, that was actually the parameter's name, assert was NAMED_TYPE
                if (lhs.kind() != expr::NAMED_TYPE)
                {
                    throw diag_except(p_log->push(make_expected_identifer()));
                }
                string_view paramname = lhs.text();
                parse_expr rhs = kind();
                return produce_nary(expr::PARAM, paramname, { rhs });
            }
            return lhs;
        }

        // type = value or typename: type = value
        parse_expr default_param()
        {
            parse_expr target = param(); // type or name: type
            if (accept(token::EQUAL, token::BACKWARD_ARROW)) 
            {
                // TODO assert lhs is param? actually type is fine too
                parse_expr deflt = block();
                return produce_nary(expr::DEFAULT_PARAM, "", { target, deflt });
            }
            return target;
        }

        // type tuple must be parens since comma should mean end of varaible expr.
        parse_expr paren_tuple_kind()
        {
            vector<parse_expr> exprs;
            if (!check(token::RIGHT_PARENTHESIS))
            {
                do 
                {
                    ignore_eol(); // ignore newlines since pending bracket
                    exprs.push_back(default_param()); // TODO check assignment or target? 
                } while(accept(token::COMMA));
            }
            ignore_eol();
            expect(make_expected_rparen(), token::RIGHT_PARENTHESIS);
            return produce_nary(expr::TUPLE_TYPE, "", array<parse_expr>(exprs.begin(), exprs.end()));
        }

        // TODO struct type with {}

        parse_expr function_kind()
        {
            parse_expr params = primary_kind();
            if (accept(token::FORWARD_ARROW))
            {
                parse_expr rett = function_kind();
                return produce_nary(expr::FUNCTION_TYPE, "", { params, rett });
            }
            return params;
        }
        
        parse_expr kind()
        {
            return function_kind();
        }

        // assume id is accepted
        parse_expr variable()
        {
            string_view varname = curr.text();
            parse_expr typ = produce_blank();
            if (accept(token::COLON))
            {
                typ = kind();
                return produce_nary(expr::TYPED_VARIABLE, varname, { typ });
            }
            return produce_nary(expr::VARIABLE, varname, {}); // TODO should it be blank or just size 0? techinally not a blank expr like return or branch
        }

        parse_expr intrinsic()
        {
            string_view intrname = curr.text();
            return produce_term(expr::INTRINSIC, intrname);
        }

        parse_expr primary()
        {
            if (accept(isliteral))
            {
                string text = literal_text(curr);
                return produce_term(token_to_expr_literal_kind(curr.kind()), text);
            }
            if (accept(token::IDENTIFIER))
            {
                return variable();
            }
            if (accept(token::INTRINSIC))
            {
                return intrinsic();
            }
            if (accept(token::LEFT_PARENTHESIS))
            {
                return paren_tuple();
            }
            throw parse_except(p_log->push(make_expected_primary()));
        }

        parse_expr call()
        {
            parse_expr e = primary();
            if (accept(token::LEFT_PARENTHESIS))
            {
                vector<parse_expr> exprs;
                exprs.push_back(move(e)); // callee
                exprs.push_back(paren_tuple()); // call args
                return produce_nary(expr::CALL, "", array<parse_expr>(exprs.begin(), exprs.end())); // should have size 2.
            }
            return e; 
        }

        parse_expr block()
        {
            if (accept(token::LEFT_BRACE))
            {
                vector<parse_expr> exprs;
                while (!accept(token::RIGHT_BRACE))
                {
                    ignore_eol(); // this is optional but will result in blank anyways.
                    parse_expr e = statement();
                    if (e.kind() != expr::BLANK)
                    {
                        exprs.push_back(move(e));
                    }
                }
                return produce_nary(expr::BLOCK, "", array<parse_expr>(exprs.begin(), exprs.end()));
            }
            return call();
        }

        parse_expr function()
        {
            parse_expr params = block();
            if (accept(token::FORWARD_ARROW))
            {
                if (!assignable(params))
                {
                    throw parse_except(p_log->push(make_expected_target())); // function just uses any assignable as params
                }
                parse_expr body = function(); // if boyd is another function, this function returns a function
                return produce_nary(expr::FUNCTION, "", { params, body });
            }
            return params;
        }

        // assignment
        parse_expr assignment()
        {
            parse_expr lhs = function();
            if (accept(token::EQUAL, token::BACKWARD_ARROW)) 
            {
                // TODO assert && lhs is assignable
                if (!assignable(lhs))
                {
                    throw parse_except(p_log->push(make_expected_target()));
                }
                parse_expr rhs = assignment();
                return produce_nary(expr::ASSIGN, "", { lhs, rhs });
            }
            return lhs;
        }

        // (assumes LHS bracket accepted already)
        // an explicit parenthesis tuple can contain 0..n values, unlike an comma-implicit tuple which must have 2+
        parse_expr paren_tuple()
        {
            vector<parse_expr> exprs;
            if (!check(token::RIGHT_PARENTHESIS))
            {
                do 
                {
                    ignore_eol(); // ignore newlines since pending bracket
                    exprs.push_back(assignment());
                } while(accept(token::COMMA));
            }
            ignore_eol();
            expect(make_expected_rparen(), token::RIGHT_PARENTHESIS);
            return produce_nary(expr::TUPLE, "", array<parse_expr>(exprs.begin(), exprs.end()));
        }

        // tuple (implicit), started by comma, of at leasst 2+ subexprs.
        // TODO gifure out grammar for creating a tuple with defaults, ie (int = 0, int) = ???, 3. maybe using keyword default?
        parse_expr tuple()
        {
            parse_expr first = assignment();
            if (accept(token::COMMA))
            {
                vector<parse_expr> exprs;
                exprs.push_back(move(first));
                do 
                {
                    exprs.push_back(assignment());
                } while(accept(token::COMMA));
                return produce_nary(expr::TUPLE, "", array<parse_expr>(exprs.begin(), exprs.end()));
            }
            return first;
        }

        // global assign is lower precedence than tuple to allow parallel assign. 
        // expr global_assign()
        // {
        //     expr lhs = tuple();
        //     if (accept(token::COLON_EQUAL)) 
        //     {
        //         // && lhs is assignable
        //         expr rhs = global_assign();
        //         return produce_nary(expr::ASSIGN, "", { lhs, rhs });
        //     }
        //     return lhs;
        // }

        parse_expr expression()
        {
            return tuple();
        }

        parse_expr expression_statement()
        {
            parse_expr e = expression();
            expect(make_expected_eoe(), isendofexpr);
            return e;
        }

        parse_expr branch_statement()
        {
            if (accept(token::BRANCH_KEYWORD))
            {
                string_view to_label;
                parse_expr cond = produce_blank();
                if (accept(token::LABEL)) // if no label, implicitly execute next statement iff true.
                {
                    to_label = curr.text(); 
                }
                if (!check(isendofexpr))
                {
                    cond = expression();
                }
                expect(make_expected_eoe(),isendofexpr);
                return (produce_nary(expr::RETURN, to_label, move(cond)));
            }
            return expression_statement();
        }

        parse_expr return_statement()
        {
            if (accept(token::RETURN_KEYWORD))
            {
                string_view to_label;
                parse_expr e = produce_blank();
                if (accept(token::LABEL)) // if no label, implicitly return from innermost scope
                {
                    to_label = curr.text(); 
                }
                if (!check(isendofexpr))
                {
                    e = expression();
                }
                expect(make_expected_eoe(), isendofexpr);
                return (produce_nary(expr::RETURN, to_label, move(e)));
            }
            return branch_statement();
        }

        // TODO labelled is lowest precedence

        parse_expr statement()
        {
            // empty/null expression, just ignore it since semantically no significance
            if (accept(isendofexpr)) return produce_blank();
            return return_statement();
        }

        void mark_srcrefloc()
        {
            expr_srcref = next.srcref();
            expr_loc = next.loc();
        }

        // get next top level, unlike statement(), 
        void parse()
        {
            if (stop())
            {
                return;
            }
            mark_srcrefloc();
            parse_expr stmt = statement();
            p_log->push(make_parse_expr_info(stmt));
            make_top(move(stmt));
        }
    };
}

parse_result parse(const source* p_src, parse_expr_tree* p_exprs, diag_logger* p_log)
{
    parse_result res = parse_result::PARSE_OK;
    source_location loc;
    source_reference sr(p_src, 0, p_src->size());
    internal::parser parser(&sr, p_exprs, &loc, p_log);
    while (!parser.stop()) 
    {
        try {
            parser.parse(); // TOOD invaild is not best signal that parse is done
        }
        catch (const lex_except& e) {
            res = parse_result::PARSE_FAIL;
            if (p_log->fatal(e.dg))
            {
                break;
            }
        }
        catch (const diag_except& e) {
            res = parse_result::PARSE_FAIL;
            if (p_log->fatal(e.dg))
            {
                break;
            }
            parser.sync();
        }
    }
    return res;
    // while (true)
    // {
    //     token t = lex(src, loc);
    //     if (t.is(token::STOP))
    //     {
    //         break;
    //     }
    // }

    // while (!(parser.isend() || parser.isstop()))
    // {
    //     unique_ptr<statement::statement> stmt = parser.next_stmt();
    //     if (stmt != nullptr) stmts.push_back(move(stmt));
    // }
}
}