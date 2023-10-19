#include "intermediate.h"

#include "except.h"
#include "utility.h"
#include "string.h"
#include "print.h"
#include "intrinsic.h"
#include "cast.h"
#include "expr.h"
#include "internal/debug.h"
#include "internal/type_printer.h"
#include "internal/value_printer.h"
#include "internal/intermediate_printer.h"

namespace lu
{

namespace diags
{
    diag INTERMEDIATE_INFO = diag(diag::DEBUG_LEVEL, 4900);
}

intermediate_store_symbol::intermediate_store_symbol(intermediate_store_symbol&& other) : sid(move(other.sid)), eval(move(other.eval))
{}

intermediate_store_symbol::intermediate_store_symbol(const intermediate_store_symbol& other) : sid((other.sid)), eval(make_unique(new intermediate(*other.eval)))
{}


intermediate_branch::intermediate_branch(intermediate_branch&& other) : base(move(other.base)), condition(move(other.condition)), offset(move(other.offset))
{
}

intermediate_branch::intermediate_branch(const intermediate_branch& other) : base((other.base)), condition(make_unique(new intermediate(*other.condition))), offset(other.offset)
{
}

intermediate intermediate::create_load_constant(intermediate_load_constant&& imm)
{
    intermediate i;
    i._inst = LOAD_CONSTANT;
    new (&i.imm) intermediate_load_constant(move(imm));
    return i;
}

intermediate intermediate::create_load_symbol(intermediate_load_symbol load)
{
    assert(load.sid != symbol::INVALID_ID);

    intermediate i;
    i._inst = LOAD_SYMBOL;
    i.load = load;
    return i;
}

intermediate intermediate::create_store_symbol(intermediate_store_symbol&& store)
{
    assert(store.sid != symbol::INVALID_ID);

    intermediate i;
    i._inst = STORE_SYMBOL;
    new (&i.store) intermediate_store_symbol(move(store));
    return i;
}

intermediate intermediate::create_intrinsic(intermediate_intrinsic intr)
{
    intermediate i;
    i._inst = INTRINSIC;
    i.intr = (intr);
    return i;
}

intermediate intermediate::create_call(intermediate_call&& call)
{
    intermediate i;
    i._inst = CALL;
    new (&i.call) intermediate_call(move(call));
    return i;
}

intermediate intermediate::create_tuple(intermediate_tuple&& tup)
{
    intermediate i;
    i._inst = TUPLE;
    new (&i.tup) intermediate_tuple(move(tup));
    return i;
}

intermediate intermediate::create_halt()
{
    intermediate i;
    i._inst = HALT;
    return i;
}

intermediate::intermediate() : _inst(ILLEGAL) {}

intermediate::~intermediate()
{
    destroy();
}

intermediate::intermediate(intermediate&& other)
{
    this->create(move(other));
}

intermediate::intermediate(const intermediate& other)
{
    this->create(other);
}

intermediate& intermediate::operator=(intermediate&& other)
{
    return this->assign(move(other));
}

intermediate& intermediate::operator=(const intermediate& other)
{
    return this->assign(move(other));
}

intermediate& intermediate::create(intermediate&& other)
{
    this->_inst = other._inst;
    other._inst = intermediate::ILLEGAL;
    switch (_inst)
    {
    case intermediate::ILLEGAL:
        break;
    case intermediate::LOAD_CONSTANT:
        new (&this->imm) intermediate_load_constant(move(other.imm));
        break;
    case intermediate::LOAD_SYMBOL:
        new(&this->load) intermediate_load_symbol(move(other.load));
        break;
    case intermediate::STORE_SYMBOL:
        new(&this->store) intermediate_store_symbol(move(other.store));
        break;
    case intermediate::INTRINSIC:
        new(&this->intr) intermediate_intrinsic(move(other.intr));
        break;
    case intermediate::BLOCK:
        new(&this->blk) intermediate_block(move(other.blk));
        break;
    case intermediate::CALL:
        new(&this->call) intermediate_call(move(other.call));
        break;
    case intermediate::TUPLE:
        new(&this->tup) intermediate_tuple(move(other.tup));
        break;
    case intermediate::RETURN:
        new(&this->ret) intermediate_return(move(other.ret));
        break;
    case intermediate::BRANCH:
        new(&this->br) intermediate_branch(move(other.br));
        break;
    case intermediate::HALT:
        break;
    default:
        throw internal_except_unhandled_switch(to_string(_inst));
    }
    return *this;
}

intermediate& intermediate::create(const intermediate& other)
{
    this->_inst = other._inst;
    switch (_inst)
    {
    case intermediate::ILLEGAL:
        break;
    case intermediate::LOAD_CONSTANT:
        new (&this->imm) intermediate_load_constant((other.imm));
        break;
    case intermediate::LOAD_SYMBOL:
        new(&this->load) intermediate_load_symbol((other.load));
        break;
    case intermediate::STORE_SYMBOL:
        new(&this->store) intermediate_store_symbol((other.store));
        break;
    case intermediate::INTRINSIC:
        new(&this->intr) intermediate_intrinsic((other.intr));
        break;
    case intermediate::BLOCK:
        new(&this->blk) intermediate_block((other.blk));
        break;
    case intermediate::CALL:
        new(&this->call) intermediate_call((other.call));
        break;
    case intermediate::TUPLE:
        new(&this->tup) intermediate_tuple(other.tup);
        break;
    case intermediate::RETURN:
        new(&this->ret) intermediate_return((other.ret));
        break;
    case intermediate::BRANCH:
        new(&this->br) intermediate_branch((other.br));
        break;
    case intermediate::HALT:
        break;
    default:
        throw internal_except_unhandled_switch(to_string(_inst));
    }
    return *this;
}

intermediate& intermediate::assign(intermediate&& other)
{
    this->destroy();
    return this->create(move(other));
}

intermediate& intermediate::assign(const intermediate& other)
{
    this->destroy();
    return this->create(other);
}

void intermediate::destroy()
{
    switch (_inst)
    {
    case intermediate::ILLEGAL:
        break;
    case intermediate::LOAD_CONSTANT:
        imm.~intermediate_load_constant();
        break;
    case intermediate::LOAD_SYMBOL:
        load.~intermediate_load_symbol();
        break;
    case intermediate::STORE_SYMBOL:
        store.~intermediate_store_symbol();
        break;
    case intermediate::INTRINSIC:
        intr.~intermediate_intrinsic();
        break;
    case intermediate::BLOCK:
        blk.~intermediate_block();
        break;
    case intermediate::CALL:
        call.~intermediate_call();
        break;
    case intermediate::TUPLE:
        tup.~intermediate_tuple();
        break;
    case intermediate::RETURN:
        ret.~intermediate_return();
        break;
    case intermediate::BRANCH:
        br.~intermediate_branch();
        break;
    case intermediate::HALT:
        break;
    default:
        throw internal_except_unhandled_switch(to_string(_inst));
    }
}

intermediate_context::intermediate_context(analyze_context&& ac)
{
    _syms = move(ac.symbols());
    _types = move(ac.types());
    _svals = move(ac.static_values());
}

void intermediate_program::set_context(analyze_context&& ac)
{
    // TODO perform context merge, for now just replace:
    _ctxt = intermediate_context(move(ac));
}

intermediate& intermediate_program::operator[](intermediate_addr iaddr)
{
    assert(iaddr < _insts.size());
    
    return _insts[iaddr];
}

const intermediate& intermediate_program::operator[](intermediate_addr iaddr) const
{
    assert(iaddr < _insts.size());
    
    return _insts[iaddr];
}

intermediate_addr intermediate_program::fpush(intermediate&& i)
{
    intermediate_addr iaddr = _finsts.size();
    _finsts.push_back(move(i));
    return iaddr;
}

intermediate_addr intermediate_program::push(intermediate&& i)
{
    intermediate_addr iaddr = _insts.size();
    _insts.push_back(move(i));
    return iaddr;
}

//intermediate_addr intermediate_program::write(intermediate_addr, intermediate&&); // overwrite if needed - only use for hard-coded addresses (probably unnecessary)

size_t intermediate_program::size() const
{
    return _insts.size();
}

const char* intermediate_op_cstr(intermediate::intermediate_op op)
{
    switch (op)
    {
    case intermediate::ILLEGAL:
        return "ILLEGAL";
    case intermediate::LOAD_CONSTANT:
        return "LOAD_CONSTANT";
    case intermediate::LOAD_SYMBOL:
        return "LOAD_SYMBOL";
    case intermediate::STORE_SYMBOL:
        return "STORE_SYMBOL";
    case intermediate::INTRINSIC:
        return "INTRINSIC";
    case intermediate::BLOCK:
        return "BLOCK";
    case intermediate::CALL:
        return "CALL";
    case intermediate::TUPLE:
        return "TUPLE";
    case intermediate::RETURN:
        return "RETURN";
    case intermediate::BRANCH:
        return "BRANCH";
    case intermediate::HALT:
        return "HALT";
    default:
        throw internal_except_unhandled_switch(to_string(op));
    }   
}

namespace internal
{

    // bool isliteral(const analyze_expr& ae)
    // {
    //     return expr::_label_LITERAL_FIRST <= ae.kind() && ae.kind() <= expr::_label_LITERAL_LAST;
    // }

    // bool isvariable(const analyze_expr& ae)
    // {
    //     return ae.kind() == expr::VARIABLE || ae.kind() == expr::TYPED_VARIABLE;
    // }

    // bool istypedvariable(const analyze_expr& ae)
    // {
    //     return ae.kind() == expr::TYPED_VARIABLE;
    // }

    // bool isassign(const analyze_expr& ae)
    // {
    //     return ae.kind() == expr::ASSIGN;
    // }

    // bool iscall(const analyze_expr& ae)
    // {
    //     return ae.kind() == expr::CALL;
    // }

    // bool istuple(const analyze_expr& ae)
    // {
    //     return ae.kind() == expr::TUPLE;
    // }

    // bool istype(const analyze_expr& ae)
    // {
    //     return ae.kind() == expr::TUPLE_TYPE || ae.kind() == expr::FUNCTION_TYPE || ae.kind() == expr::NAMED_TYPE;
    // }

    // bool isintrinsic(const analyze_expr& ae)
    // {
    //     return ae.kind() == expr::INTRINSIC;
    // }

    

    struct intermediate_transformer
    {
        intermediate_transformer(analyze_expr_tree* aet, intermediate_program* ip, diag_logger* log)
            : p_aet(aet), p_ip(ip), p_log(log), idx(0), printer(ip)
        {
            p_ip->set_context(move(p_aet->context()));
        }

        analyze_expr_tree* p_aet;
        intermediate_program* p_ip;
        diag_logger* p_log;
        size_t idx; // 0..size of aet
        intermediate_printer printer;

        diag_context make_intermediate_info(intermediate_addr iaddr, const intermediate& i)
        {
            return diag_context(
                diags::INTERMEDIATE_INFO,
                i.srcref(),
                i.loc(),
                string::join("emitted intermediate: ", printer.print(iaddr, i))
            );
        }

        bool stop() const
        {
            return idx >= p_aet->size();
        }

        void advance()
        {
            ++idx;
        }

        const analyze_expr& curr()
        {
            return (*p_aet)[idx];
        }

        const type_registry& types()
        {
            return p_ip->context().types();
        }

        const symbol_table& symbols()
        {
            return p_ip->context().symbols();
        }

        void emit(intermediate&& i)
        {
            intermediate_addr iaddr = p_ip->push(move(i));
            p_log->push(make_intermediate_info(iaddr, (*p_ip)[iaddr]));
        }

        intermediate_value value_static_cast(type_id to, intermediate_value&& val)
        {
            // lit to bin cast
            type_id from = val.tid();
            if (from.is(LITERAL) && to.is(BUILTIN))
            {
                val = literal_to_builtin_cast(types(), to, val);
                return move(val);
            }
            else
            {
                throw internal_except_todo();
            }
        }

        intermediate make_load_literal(const analyze_expr& ae)
        {
            //ae.eval_type();
            intermediate_value val(ae.base_type());
            if (val.tid().is(LITERAL))
            {
                val.lit = literal_value(ae.text());
            }
            else if (val.tid().is(BUILTIN))
            {
                throw internal_except_todo();
            }
            else
            {
                throw internal_except_with_location("invalid type for literal conversion");
            }
            if (ae.base_type() != ae.eval_type())
            {
                // todo cast
                val = (value_static_cast(ae.eval_type(), move(val)));
            }
            intermediate i = (intermediate::emplace_load_constant(move(val)));
            return i;
        }

        intermediate make_load_variable(const analyze_expr& ae)
        {
            if (istypedvariable(ae))
            {
                return (intermediate::emplace_load_symbol(ae.sid()));
            }
            else
            {
                return (intermediate::emplace_load_symbol(ae.sid()));
            }
            throw internal_except("TODO???");
        }

        intermediate make_tuple(const analyze_expr& ae)
        {
            assert(istuple(ae));
            assert(ae.base_type().is(TUPLE));

            array<intermediate> subs(ae.arity());
            for (size_t i = 0; i < ae.arity(); ++i)
            {
                subs[i] = make_rhs(ae[i]);
            }
            return intermediate::emplace_tuple(move(subs));
        }

        symbol_id get_target_sid(const analyze_expr& ae)
        {
            if (isvariable(ae))
            {
                assert(symbols().exists(ae.sid()));

                return ae.sid();
            }
            // else if (istuple(ae))
            // {
            //     return ae.sid();
            // }
            throw internal_except("TODO???");
        }

        intermediate make_rhs(const analyze_expr& ae)
        {
            if (isliteral(ae))
            {
                return make_load_literal(ae);
            }
            else if (isvariable(ae))
            {
                return make_load_variable(ae);
            }
            else if (istuple(ae))
            {
                return make_tuple(ae);
            }
            else if (iscall(ae))
            {
                return make_call(ae);
            }
            else if (isfunction(ae))
            {
                return make_function(ae);
            }
            // TODO
            throw internal_except_todo();
        }

        intermediate make_function(const analyze_expr& ae)
        {
            assert(ae.is(expr::FUNCTION));

            const analyze_expr& params = ae[expr::FUNCTION_PARAMS_IDX];
            const analyze_expr& body = ae[expr::FUNCTION_BODY_IDX];

            throw internal_except_todo();
        }

        intermediate make_call(const analyze_expr& ae)
        {
            assert(ae.is(expr::CALL));
            assert(ae[expr::CALL_ARGS_IDX].is(expr::TUPLE));

            const analyze_expr& callee = ae[expr::CALL_CALLEE_IDX];
            const analyze_expr& args = ae[expr::CALL_ARGS_IDX];

            if (callee.base_type().is(INTRINSIC))
            {
                const type& ty = types().find_type(callee.base_type());
                symbol_id dest = symbol::INVALID_ID;
                symbol_id op = symbol::INVALID_ID;

                switch (ty.intr.config)
                {
                case intrinsic_type::NONE:
                {
                    assert(args.arity() == 0);

                    break;
                }
                case intrinsic_type::DEST_ONLY:
                {
                    assert(args.arity() == 1);
                    assert(args[0].is(expr::VARIABLE));

                    dest = args[0].sid();
                    break;
                }
                case intrinsic_type::OP_ONLY:
                {
                    assert(args.arity() == 1);
                    assert(args[0].is(expr::VARIABLE));

                    op = args[0].sid();
                    break;
                }
                case intrinsic_type::BOTH:
                {
                    assert(args.arity() == 2);
                    assert(args[0].is(expr::VARIABLE));
                    assert(args[1].is(expr::VARIABLE));

                    dest = args[0].sid();
                    op = args[1].sid();
                    break;
                }
                default:
                    throw internal_except_unhandled_switch(to_string(ty.intr.config));
                }
                const intrinsic& intr = symbols().find_intrinsic(callee.iid());
                return (intermediate::emplace_intrinsic(intr.icode, callee.iid(), dest, op));
            }
            else
            {
                throw internal_except_todo();
            }
            //ae[1]; // args (tuple)
            
        }

        intermediate make_halt()
        {
            return intermediate::create_halt();
        }

        void transform_top_analyze_expr(const analyze_expr& ae)
        {
            if (isliteral(ae))
            {
                // TODO unused?
            }
            else if (isintrinsic(ae))
            {
                // TODO unused?
            }
            else if (isvariable(ae))
            {
                // TODO unused?
            }
            else if (istype(ae))
            {
                throw internal_except("HUH");
            }
            else if (isassign(ae))
            {
                const analyze_expr& target = ae[expr::ASSIGN_TARGET_IDX];
                const analyze_expr& rhsexpr = ae[expr::ASSIGN_RHS_IDX];

                if (isvariable(target))
                {
                    symbol_id dest = get_target_sid(target);
                    intermediate rhs = make_rhs(rhsexpr);
                    emit(intermediate::emplace_store_symbol(dest, make_unique(new intermediate(move(rhs)))));
                }
                else if (istuple(target))
                {
                    // TODO use tuple access isntead of parallel assign? - this requires recursively setting eval_type() which is annoying.
                    // assert all lhs targets are symbols?
                    if (istuple(rhsexpr))
                    {
                        assert(target.arity() == rhsexpr.arity()); // should be checked by analyze

                        for (size_t i = 0; i < target.arity(); ++i)
                        {
                            assert(isvariable(target[i]));
                            //assert(target[i].eval_type() == rhsexpr[i].eval_type()); // TODO convertible, if needed

                            emit(intermediate::emplace_store_symbol(get_target_sid(target[i]), make_unique(new intermediate(make_rhs(rhsexpr[i])))));
                        }
                    }
                    else if (rhsexpr.base_type().is(TUPLE))
                    {
                        throw internal_except_todo();
                        // TODO: we need member acces for this
                        // assert(types().find_type(rhsexpr.eval_type()).tup.arity() == target.arity());

                        // for (size_t i = 0; i < target.arity(); ++i)
                        // {
                        //     emit(intermediate::emplace_store_symbol(get_target_sid(target[i], make_unique(new intermediate()))));
                        // }
                    }
                    else
                    {
                        throw internal_except("immediate assignment tuple is not valid");
                    } 
                }
                else
                {
                    throw internal_except("immediate assignment target is not valid");
                }
            }
            else if (iscall(ae))
            {
                emit(make_call(ae));
            }
            else if (istuple(ae))
            {
                // for each arg do...
                for (size_t i = 0; i < ae.arity(); ++i)
                {
                    transform_top_analyze_expr(ae[i]);
                }
            }
            else if (isblock(ae))
            {
                for (size_t i = 0; i < ae.arity(); ++i)
                {
                    transform_top_analyze_expr(ae[i]);
                }
            }
            else if (isblank(ae))
            {
                // pass

            }
            else 
            {
                throw internal_except_todo();
            }
        }

        void transform_next()
        {
            transform_top_analyze_expr(curr());
            advance();
            if (stop())
            {
                emit(make_halt());
            }
        }

        void transform()
        {
            if (stop())
            {
                return;
            }
            transform_next();
            // make_top(move(ae));
        }

    };
};

intermediate_transform_result intermediate_transform(analyze_expr_tree* p_aet, intermediate_program* p_ip, diag_logger* p_log)
{
    intermediate_transform_result res = intermediate_transform_result::INTERMEDIATE_TRANSFORM_OK;
    internal::intermediate_transformer transformer(p_aet, p_ip, p_log);
    while (!transformer.stop())
    {
        try
        {
            transformer.transform();
        }
        catch(const transform_except& e)
        {
            res = intermediate_transform_result::INTERMEDIATE_TRANSFORM_FAIL;
            if (p_log->fatal(e.dg))
            {
                break;
            }
            transformer.advance();
        }
    }

    return res;
}

}