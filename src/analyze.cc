#include "analyze.h"

#include "utility.h"
#include "value.h"
#include "enum.h"
#include "internal/analyze_printer.h"

namespace lu
{
bool hassymbol(const analyze_expr& e)
{
    return e.is(expr::VARIABLE, expr::TYPED_VARIABLE);
}

bool hastype(const analyze_expr& e)
{
    return e.is(expr::NAMED_TYPE, expr::TUPLE_TYPE, expr::FUNCTION_TYPE);
}

bool hasintrinsic(const analyze_expr& e)
{
    return e.is(expr::INTRINSIC);
}

namespace diags
{
    diag ANALYZE_NOT_CONVERTIBLE = diag(diag::ERROR_LEVEL, 3010);
    diag ANALYZE_NOT_CALLABLE = diag(diag::ERROR_LEVEL, 3011);
    diag ANALYZE_UNSUPPORTED_INTRINSIC = diag(diag::ERROR_LEVEL, 3015);
    diag ANALYZE_VARIABLE_ALREADY_DECLARED = diag(diag::ERROR_LEVEL, 3020);
    diag ANALYZE_CALL_ARITY_MISMATCH = diag(diag::ERROR_LEVEL, 3021);
    diag ANALYZE_TUPLE_ARITY_MISMATCH = diag(diag::ERROR_LEVEL, 3022);
    diag ANALYZE_VARIABLE_UNUSED = diag(diag::WARN_LEVEL, 3200);
    diag ANALYZE_EXPR_INFO = diag(diag::DEBUG_LEVEL, 3900);
}

analyze_expr analyze_expr::create_vanilla(const parse_expr& pe, type_id btid, array<analyze_expr>&& subs)
{
    assert(btid != type_id::UNDEFINED);
    
    analyze_expr ae(pe);

    assert(!hasintrinsic(ae) && !hastype(ae) && !hassymbol(ae));

    ae._btid = btid;
    ae._etid = btid;
    ae._subs = move(subs);
    return ae;
}

analyze_expr analyze_expr::create_hassymbol(const parse_expr& pe, type_id btid, symbol_id sid, array<analyze_expr>&& subs)
{
    assert(btid != type_id::UNDEFINED);
    assert(sid != symbol::INVALID_ID);

    analyze_expr ae(pe);

    assert(hassymbol(ae));

    ae._btid = btid;
    ae._etid = btid;
    ae._sid = sid;
    ae._subs = move(subs);
    return ae;
}

analyze_expr analyze_expr::create_hasintrinsic(const parse_expr& pe, type_id btid, intrinsic_id iid)
{
    assert(btid != type_id::UNDEFINED);
    assert(iid != intrinsic::INVALID_ID);

    analyze_expr ae(pe);

    assert(hasintrinsic(ae));

    ae._btid = btid;
    ae._etid = btid;
    ae._iid = iid;
    ae._subs = {};
    return ae;
}

analyze_expr analyze_expr::create_hastype(const parse_expr& pe, type_id btid, type_id tid)
{
    assert(btid != type_id::UNDEFINED);
    assert(tid != type_id::UNDEFINED);

    analyze_expr ae(pe);

    assert(hastype(ae));

    ae._btid = btid;
    ae._etid = btid;
    ae._tid = tid;
    return ae;
}

analyze_expr::analyze_expr() : expr(), _text(""), _etid(type_id::UNDEFINED), _sid(symbol::INVALID_ID), _flags(_flags.NONE)  {}

analyze_expr::analyze_expr(expr::expr_kind kind) : expr(kind)
{}

analyze_expr::analyze_expr(const parse_expr& pe) : expr(pe.kind(), pe.srcref(), pe.loc()), _text(pe.text())
{}

analyze_expr::~analyze_expr()
{
    destroy();
}

symbol_id analyze_expr::sid() const
{
    assert(kind() != expr::INTRINSIC);

    return _sid;
}

intrinsic_id analyze_expr::iid() const
{
    assert(kind() == expr::INTRINSIC);

    return _iid;
}

type_id analyze_expr::tid() const
{
    assert(kind() == expr::TUPLE_TYPE || kind() == expr::NAMED_TYPE || kind() == expr::FUNCTION_TYPE);

    return _tid;
}

void analyze_expr::destroy()
{
    if (hastype(*this))
    {
        this->_tid.~type_id();
    }
    else
    {
        // pass
    }
}


size_t analyze_expr_tree::size() const
{
    return _top_exprs.size();
}

analyze_expr& analyze_expr_tree::push_top_expr(analyze_expr&& e)
{
    _top_exprs.push_back(move(e));
    return _top_exprs.back();
}

analyze_expr_tree::analyze_expr_tree()
{
    // TODO move to analyzer?
    // register_void(_ctxt.types());
    // register_literal_types(_ctxt.types());
    // register_builtin_types(_ctxt.types());
    //declare_intrinsics(_ctxt.types(), _ctxt.symbols());
    //declare_global_builtin_types(_ctxt.types(), _ctxt.symbols());
}

namespace internal
{
    string_view intr_name(string_view sv)
    {
        assert(sv.size() > 0 && sv[0] == '$');

        return sv.subview(1);
    }

    struct analyzer
    {
        analyzer(const parse_expr_tree* p_pet, analyze_expr_tree* p_aet, diag_logger* p_log) : p_pet(p_pet), p_aet(p_aet), p_log(p_log), idx(0), printer(p_aet)
        {
            p_scope = symbols().top();

            register_void();
            register_literal_types();
            register_builtin_types();
            declare_intrinsics();
            declare_global_builtin_types();
        }

        const parse_expr_tree* p_pet;
        analyze_expr_tree* p_aet;
        lexical_scope* p_scope;
        diag_logger* p_log;
        size_t idx; // 0..size of pet

        analyze_expr_printer printer;

        diag_context make_analyze_expr_info(const analyze_expr& ae)
        {
            return diag_context(
                diags::ANALYZE_EXPR_INFO,
                ae.srcref(),
                ae.loc(),
                move(string("analyzed expression: ").append(printer(ae)))
            );
        }

        diag_context make_analyze_already_declared(const parse_expr& pe)
        {
            return diag_context(
                diags::ANALYZE_VARIABLE_ALREADY_DECLARED,
                pe.srcref(),
                pe.loc(),
                move(string("symbol was previously declared '").append(pe.text()).append("'"))
            );
        }

        diag_context make_analyze_tuple_arity_mismatch(const parse_expr& pe, const type& t1, const type& t2)
        {
            assert(t1.tclass == TUPLE && t2.tclass == TUPLE);
            return diag_context(
                diags::ANALYZE_TUPLE_ARITY_MISMATCH,
                pe.srcref(),
                pe.loc(),
                move(string("tuples must be of same arity (size) ").append(types().name(t1)).append(" <-> ").append(types().name(t2)))
            );
        }

        diag_context make_analyze_call_arity_mismatch(const parse_expr& pe, const type& t1, const type& t2)
        {
            assert(t1.callable() && t2.tclass == TUPLE);
            return diag_context(
                diags::ANALYZE_TUPLE_ARITY_MISMATCH,
                pe.srcref(),
                pe.loc(),
                move(string("call arguments must be of same arity as function parameters (size)").append(types().name(t1)).append(" <-> ").append(types().name(t2)))
            );
        }

        diag_context make_not_callable(const parse_expr& pe, const type& t)
        {
            assert(!t.callable());
            return diag_context(
                diags::ANALYZE_NOT_CALLABLE,
                pe.srcref(),
                pe.loc(),
                string::join("callee in call expression must be callable but is '", types().name(t), "'")
            );
        }

        diag_context make_unsupported_intrinsic(const parse_expr& pe)
        {
            return diag_context(
                diags::ANALYZE_UNSUPPORTED_INTRINSIC,
                pe.srcref(),
                pe.loc(),
                string::join("intrinsic function '", pe.text(), "'", " is not supported")
            );
        }

        diag_context make_not_convertible(const parse_expr& pe, const type& t1, const type& t2)
        {
            return diag_context(
                diags::ANALYZE_NOT_CONVERTIBLE,
                pe.srcref(),
                pe.loc(),
                move(string("cannot convert from ").append(types().name(t1)).append(" to ").append(types().name(t2)))
            );
        }

        bool stop() const
        {
            return idx >= p_pet->size();
        }

        void advance()
        {
            ++idx;
        }

        type_registry& types()
        {
            return p_aet->context().types();
        }

        symbol_table& symbols()
        {
            return p_aet->context().symbols();
        }

        intermediate_value_table& static_values()
        {
            return p_aet->context().static_values();
        }

        void begin_scope()
        {
            p_scope = p_scope->push_sub();
        }

        void end_scope()
        {
            p_scope = p_scope->up();
        }

        lexical_scope* curr_scope()
        {
            return p_scope;
        }

        const parse_expr& curr()
        {
            return (*p_pet)[idx];
        }

        bool doessymbolexist(string_view varname)
        {
            symbol_id sid = symbols().find_innermost(curr_scope(), varname);
            return sid != symbol::INVALID_ID;
        }

        symbol_id find_local(string_view varname)
        {
            return symbols().find_local(curr_scope(), varname);
        }

        symbol_id find_innermost(string_view varname)
        {
            return symbols().find_innermost(curr_scope(), varname);
        }

        void make_top(analyze_expr&& ae)
        {
            p_aet->push_top_expr(move(ae));
        }

        void throw_diag(const diag_context& dc)
        {
            p_log->push(dc);
            throw analyze_except(dc.dg);
        }

        void register_void()
        {
            types().register_type(type::create_void_type());
        }

        void register_literal_types()
        {
            enum_iterable<literal_type, literal_type::_label_FIRST, literal_type::_label_LAST> e;
            for (auto it = e.begin(); it != e.end(); ++it)
            {
                types().register_type(type::create_literal_type(*it));
            }
        }

        void register_builtin_types()
        {
            enum_iterable<builtin_type, builtin_type::_label_FIRST, builtin_type::_label_LAST> e;
            for (auto it = e.begin(); it != e.end(); ++it)
            {
                types().register_type(type::create_builtin_type(*it));
            }
        }

        void declare_intrinsics()
        {
            //type_id void_id = types.find_type_id(type::create_void_type()),
            type_id int32_id = types().find_builtin_type_id(builtin_type::INT32);
            type_id int64_id = types().find_builtin_type_id(builtin_type::INT64);
            type_id bool_id = types().find_builtin_type_id(builtin_type::BOOL);
            type_id ascii_id = types().find_builtin_type_id(builtin_type::ASCII);

            symbols().declare_intrinsic(intrinsic("i32add", I32ADD, intrinsic_type::BOTH, int32_id, int32_id)); // TODO using $ in string in fragile in case i change it later
            symbols().declare_intrinsic(intrinsic("i64add", I64ADD, intrinsic_type::BOTH, int64_id, int64_id)); // TODO using $ in string in fragile in case i change it later
            symbols().declare_intrinsic(intrinsic("i32print", I32PRINT, intrinsic_type::OP_ONLY, type_id::UNDEFINED, int32_id));
            symbols().declare_intrinsic(intrinsic("i64print", I64PRINT, intrinsic_type::OP_ONLY, type_id::UNDEFINED, int64_id));
            symbols().declare_intrinsic(intrinsic("bprint", BPRINT, intrinsic_type::OP_ONLY, type_id::UNDEFINED, bool_id));
            symbols().declare_intrinsic(intrinsic("asciiprint", ASCIIPRINT, intrinsic_type::OP_ONLY, type_id::UNDEFINED, ascii_id));
            symbols().declare_intrinsic(intrinsic("lneg", LNEG, intrinsic_type::DEST_ONLY, bool_id, type_id::UNDEFINED));
        }

        void declare_global_builtin_types()
        {
            enum_iterable<builtin_type, builtin_type::_label_FIRST, builtin_type::_label_LAST> e;
            for (auto it = e.begin(); it != e.end(); ++it)
            {
                type ty = type::create_builtin_type(*it);
                if (types().exists(ty))
                {
                    type_id tidtid = types().find_builtin_type_id(builtin_type::TYPEID);
                    type_id tytid = types().find_type_id(ty);
                    // TODO pass in naming fnction
                    string tyname = eng_us_name(ty.bin);
                    symbol& sym = symbols().declare_global(symbol(tidtid, tyname, symbol_flag::STATIC));

                    assert(sym.tid == tidtid);

                    intermediate_value val(sym.tid);
                    val.bin.tid = tytid;
                    static_values()[sym.sid] = val;
                }
            }
        }

        void check_call(const parse_expr& e, const type& callee, const type& args)
        {
            assert(callee.callable());
            assert(args.tclass == TUPLE);

            if (callee.tclass == INTRINSIC)
            {
                // TODO parital match, optinal args etc.
                if (callee.intr.param_count() != args.tup.arity())
                {
                    throw_diag(make_analyze_call_arity_mismatch(e, callee, args));
                }
                switch (callee.intr.config)
                {
                case intrinsic_type::NONE:
                    break;
                case intrinsic_type::DEST_ONLY:
                {
                    if (callee.intr[intrinsic_type::DEST_PARAM] != args.tup[0].tid)
                    {
                        throw_diag(make_not_convertible(e, types().find_type(args.tup[0].tid), types().find_type(callee.intr[intrinsic_type::DEST_PARAM])));
                    }
                    break;
                }
                case intrinsic_type::OP_ONLY:
                {
                    if (callee.intr[intrinsic_type::OP_PARAM] != args.tup[0].tid)
                    {
                        throw_diag(make_not_convertible(e, types().find_type(args.tup[0].tid), types().find_type(callee.intr[intrinsic_type::OP_PARAM])));
                    }
                    break;
                }
                case intrinsic_type::BOTH:
                {
                    if (callee.intr[intrinsic_type::DEST_PARAM] != args.tup[0].tid)
                    {
                        throw_diag(make_not_convertible(e, types().find_type(args.tup[0].tid), types().find_type(callee.intr[intrinsic_type::DEST_PARAM])));
                    }
                    if (callee.intr[intrinsic_type::OP_PARAM] != args.tup[1].tid)
                    {
                        throw_diag(make_not_convertible(e, types().find_type(args.tup[1].tid), types().find_type(callee.intr[intrinsic_type::OP_PARAM])));
                    }
                    break;
                }
                default:
                    throw internal_except_unhandled_switch(to_string(callee.intr.config));
                }
            }
            else
            {
                // TODO implciit conversion
                throw internal_except_todo();
                // for (size_t i = 0; i < args.tup.arity(); ++i)
                // {
                //     // check arg matches
                // }
            }
        }

        type_id choose_runtime_literal_type(literal_type lit)
        {
            switch (lit)
            {
            case literal_type::STRING:
                throw internal_except_todo();
            case literal_type::INTEGER:
                return types().find_type_id_auto_register(type::create_builtin_type(builtin_type::INT64));
            case literal_type::DECIMAL:
                return types().find_type_id_auto_register(type::create_builtin_type(builtin_type::FLOAT64));
            case literal_type::TRUE: // fallthrough
            case literal_type::FALSE:
                return types().find_type_id_auto_register(type::create_builtin_type(builtin_type::BOOL));
            case literal_type::EMPTY_TUPLE:
                return types().find_type_id_auto_register(type::emplace_tuple_type());
            default:
                throw internal_except_unhandled_switch(to_string(lit));
            }
        }

        analyze_expr analyze_block(const parse_expr& pe)
        {
            array<analyze_expr> subs(pe.arity());

            begin_scope();
            for (size_t i = 0; i < pe.arity(); ++i)
            {
                subs[i] = analyze_parse_expr(pe[i], true);
                // TODO branch and return type
            }
            end_scope();
            // TODO return type
            type_id rtid = types().find_void_type();
            return analyze_expr::create_vanilla(pe, rtid, move(subs));
        }

        analyze_expr analyze_type(const parse_expr& pe)
        {
            // TODO analyze the type. add it to type reg.
            // TYPEID cannot be converted.
            type_id typeid_tid =  types().find_builtin_type_id(builtin_type::TYPEID); // itself is typeid, now figure out what is expressed type
            type_id user_tid;
            
            switch (pe.kind())
            {
            case expr::NAMED_TYPE:
            {
                symbol_id nty_sid = symbols().find_global(pe.text()); // symbol for the static builtin typeid
                if (nty_sid == symbol::INVALID_ID)
                {
                    // TODO: no type name found
                    throw internal_except_todo();
                }
                symbol nty_sym = symbols()[nty_sid];
                if (nty_sym.tid != typeid_tid)
                {
                    // TODO: name is not a type, but is "actual type of name"
                    throw internal_except_todo();
                }
                intermediate_value nty_val = static_values()[nty_sid];

                // if we setup the buitin static properly, the typeid for this name should be a existing static global entry.
                assert(nty_val.tid() == typeid_tid);

                user_tid = nty_val.bin.tid;
                break;
            }
            default:
                throw internal_except_unhandled_switch(to_string(pe.kind())); // TODO string functionfor kind.
            }

            return analyze_expr::create_hastype(pe, /*btid*/ typeid_tid, user_tid); // base type of a type-expr is typeid, and tid (the user tid) is what type is expressed
        }

        analyze_expr analyze_variable(const parse_expr& pe, type_id hint_tid)
        {
            assert(isvariable(pe));

            string_view varname = pe.text();
            
            if (istypedvariable(pe))
            {
                symbol_id sid = find_local(varname);
                analyze_expr type_ae = analyze_type(pe[expr::TYPED_VARIABLE_TYPE_IDX]);
                if (sid != symbol::INVALID_ID)
                {
                    throw_diag(make_analyze_already_declared(pe));
                }
                else
                {
                    sid = symbols().declare(curr_scope(), symbol(type_ae.tid(), varname)).sid;
                }
                
                return analyze_expr::create_hassymbol(pe, type_ae.tid(), sid, { type_ae });
            }
            else // normal varaible reference (if not exist, implicit declare)
            {
                symbol_id sid = find_innermost(varname);
                if (hint_tid.is(LITERAL))
                {
                    hint_tid = choose_runtime_literal_type(types().find_type(hint_tid).lit);
                }
                if (sid != symbol::INVALID_ID)
                {
                    // lookup reference
                    const symbol& sym = symbols()[sid];
                    return analyze_expr::create_hassymbol(pe, sym.tid, sid, {});
                }
                else
                {
                    // implcit
                    sid = symbols().declare(curr_scope(), symbol(hint_tid, varname)).sid;
                }
                // TODO pick non-literal type base type
                return analyze_expr::create_hassymbol(pe, hint_tid, sid, {});
            }
            // TODO check type conversion from hint_type
        }

        analyze_expr analyze_assignment_target(const parse_expr& pe, type_id rhs_tid)
        {
            if (isvariable(pe))
            {
                return analyze_variable(pe, rhs_tid);
            }
            else if (istuple(pe))
            {
                // check rhs_type is tuple
                type maybe_type = types().find_type(rhs_tid);
                if (!(maybe_type.tclass == TUPLE && maybe_type.tup.arity() == pe.arity())) // TODO now use strict same size, but maybe allow implicit partial unpack etc.
                {
                    throw internal_except_todo();
                }

                array<analyze_expr> subs(pe.arity());
                for (size_t i = 0; i < pe.arity(); ++i)
                {
                    subs[i] = analyze_assignment_target(pe[i], maybe_type.tup[i].tid);
                    maybe_type.tup[i].tid = subs[i].base_type();
                }
                // update type with what target's tuple type shuld be (ie no literals)
                rhs_tid = types().find_type_id_auto_register(maybe_type);
                // TODO partial conversion
                return analyze_expr::create_vanilla(pe, rhs_tid, move(subs));
            }
            // else error
            return analyze_expr();
        }

        analyze_expr analyze_function_params(const parse_expr& pe)
        {
            throw internal_except_todo();
            //return analyze_assignment_target(pe, );
        }

        analyze_expr analyze_function(const parse_expr& pe)
        {
            analyze_expr params = analyze_function_params(pe[expr::FUNCTION_PARAMS_IDX]);
            analyze_expr body = analyze_parse_expr(pe[expr::FUNCTION_BODY_IDX], false);
            array<function_type::param> ftparams;
            if (params.eval_type().is(TUPLE))
            {
                internal_except_todo();
            }
            else
            {
                ftparams = { function_type::param(params.eval_type()) };
            }
            type_id tid = types().find_type_id_auto_register(type::emplace_function_type(/*ret tid*/ body.eval_type(), /*params tids*/ move(ftparams)));

            return analyze_expr::create_vanilla(pe, tid, { params, body });
        }

        analyze_expr analyze_assignment_rhs(const parse_expr& pe)
        {
            return analyze_parse_expr(pe, false);
        }

        // analyze_expr analyze_call()
        // {}

        // analyze_expr analyze_variable()
        // {}

        // analyze_expr analyze_member()
        // {

        // }

        analyze_expr analyze_args(const parse_expr& pe)
        {
            return analyze_parse_expr(pe, false);
        }

        analyze_expr analyze_callee(const parse_expr& pe)
        {
            return analyze_parse_expr(pe, false);
        }

        analyze_expr analyze_parse_expr(const parse_expr& pe, bool istop)
        {
            if (isliteral(pe))
            {
                // TODO warn unused?
                type t = type::create_literal_type(expr_kind_to_literal_type(pe.kind()));
                type_id tid = types().find_type_id_auto_register(t);
                return analyze_expr::create_vanilla(pe, tid, {});
            }
            else if (isintrinsic(pe))
            {
                string_view iname = intr_name(pe.text());
                intrinsic_id iid = symbols().find_intrinsic_id(iname);
                if (iid == intrinsic::INVALID_ID)
                {
                    throw_diag(make_unsupported_intrinsic(pe));
                }
                intrinsic& intr = symbols().find_intrinsic(iid);
                type_id tid  = types().find_type_id_auto_register(type::emplace_intrinsic_type(intr.itype.config, intr.itype[intrinsic_type::DEST_PARAM], intr.itype[intrinsic_type::OP_PARAM]));
                return analyze_expr::create_hasintrinsic(pe, tid, iid);
            }
            else if (isvariable(pe))
            {
                // warn unused if not top
                return analyze_variable(pe, type_id::UNDEFINED);
            }
            else if (istype(pe))
            {
                return analyze_type(pe);
            }
            else if (isassign(pe))
            { 
                analyze_expr rhs = analyze_assignment_rhs(pe[1]);  assert(rhs.base_type() != type_id::UNDEFINED);
                analyze_expr lhs = analyze_assignment_target(pe[0], rhs.base_type()); assert(lhs.base_type() != type_id::UNDEFINED);
                // lhs needs to be converted to rhs // TODO check cast
                rhs.set_eval_type(lhs.base_type());
                return analyze_expr::create_vanilla(pe, lhs.eval_type(), { lhs, rhs }); // TODO reference type
            }
            else if (iscall(pe))
            {
                analyze_expr callee = analyze_callee(pe[0]);
                // symbols()[callee.sid()];
                type& callee_type = types().find_type(callee.eval_type());
                if (!callee_type.callable())
                {
                    throw_diag(make_not_callable(pe, callee_type));
                }
                analyze_expr args = analyze_args(pe[1]);
                type& args_type = types().find_type(args.eval_type());
                // check arity and type match
                check_call(pe, callee_type, args_type);
                return analyze_expr::create_vanilla(pe, callee_type.return_type(), { callee, args });
            }
            else if (istuple(pe))
            {
                array<analyze_expr> subs(pe.arity());
                array<tuple_type::member> member_types(pe.arity());
                for (size_t i = 0; i < pe.arity(); ++i)
                {
                    subs[i] = analyze_parse_expr(pe[i], false);
                    member_types[i] = tuple_type::member(subs[i].eval_type(), ""); // no names since tuple is not a variable, but unpacking of varaibles.
                }
                type_id tid = types().find_type_id_auto_register(type::create_tuple_type(tuple_type(move(member_types))));
                return analyze_expr::create_vanilla(pe, tid, move(subs)); // no sid since agian, no variables, tuple is unpacking into multiple vars
            }
            else if (isfunction(pe))
            {
                return analyze_function(pe);
            }
            else if (isblock(pe))
            {
                return analyze_block(pe);
            }
            else if (isblank(pe))
            {
                return analyze_expr();
            }
            throw internal_except_unhandled_switch(to_string(pe.kind()));
        }

        analyze_expr analyze_next()
        {
            analyze_expr ae = analyze_parse_expr(curr(), /*istop*/ true);
            advance();
            return ae;
        }

        void analyze()
        {
            if (stop())
            {
                return;
            }
            analyze_expr ae = analyze_next();
            p_log->push(make_analyze_expr_info(ae));
            make_top(move(ae));
        }
    };
}

analyze_result analyze(const parse_expr_tree* p_pet, analyze_expr_tree* p_aet, diag_logger* p_log)
{
    analyze_result res = analyze_result::ANALYZE_OK;
    internal::analyzer analyzer(p_pet, p_aet, p_log);
    while (!analyzer.stop())
    {
        try
        {
            analyzer.analyze();
        }
        catch(const analyze_except& e)
        {
            res = analyze_result::ANALYZE_FAIL;
            if (p_log->fatal(e.dg))
            {
                break;
            }
            analyzer.advance();
        }
    }
    return res;
}


// string to_string(const analyze_expr& e)
// {
//     // TODO interator for
//     return internal::analyze_expr_printer()(e);
// }

// string to_string(const analyze_expr_tree& et)
// {
//     // TODO interator for
//     return internal::analyze_expr_printer()(et);
// }
}
