#include "type.h"

#include <type_traits>
#include "enum.h"
#include "generic.h"
#include "except.h"

namespace lu
{
const type_id type_id::UNDEFINED = type_id();
const type_id type_id::VOID = type_id(type_class::VOID, 0);


bool type_id::is(type_class tc) const
{
    return tclass == tc;
}

type_id intrinsic_type::operator[](size_t idx) const
{
    assert(idx < 2);

    return params[idx];
}

bool intrinsic_type::operator<(const intrinsic_type& other) const
{
    if (this->config != other.config)
    {
        return this->config < other.config;
    }
    else if (this->params[0] != other.params[0])
    {
        return this->params[0] < other.params[0];
    }
    else
    {
        return this->params[1] < other.params[1];
    }
}

bool keyword_type::operator<(const keyword_type& other) const
{
    return (this->tid != other.tid) ? (this->tid < other.tid) : (this->name < other.name);
}

bool keyword_type::operator==(const keyword_type& other) const
{
    return (this->tid == other.tid) && (this->name == other.name);
}
bool keyword_type::operator!=(const keyword_type& other) const
{
    return !(*this == other);
}

bool tuple_type::operator<(const tuple_type& other) const
{
    return this->members < other.members;
}

bool function_type::operator<(const function_type& other) const
{
    return (this->params != other.params) ? (this->params < other.params) : (this->ret < other.ret);
}

union_type& union_type::operator |(type_id id)
{
    types.insert(id);
    return *this;
}

bool union_type::operator<(const union_type& other) const
{
    return this->types < other.types;
}

type type::create_literal_type(literal_type&& lit)
{
    type t;
    t.tclass = LITERAL;
    t.lit = move(lit);
    return t;
}

type type::create_builtin_type(builtin_type&& bin)
{
    type t;
    t.tclass = BUILTIN;
    t.bin = move(bin);
    return t;
}

// type type::create_intrinsic_type(intrinsic_type&& itr)
// {
//     type t;
//     t.tclass = INTRINSIC;
//     t.itr = move(itr);
//     return t;
// }

type type::create_void_type()
{
    type t;
    t.tclass = VOID;
    return t;
}

type type::create_function_type(function_type&& fun)
{
    type t;
    t.tclass = FUNCTION;
    t.fun = move(fun);
    return t;
}

type type::create_tuple_type(tuple_type&& tup)
{
    type t;
    t.tclass = TUPLE;
    t.tup = move(tup);
    return t;
}

type type::create_union_type(union_type&& un)
{
    type t;
    t.tclass = UNION;
    t.un = move(un);
    return t;
}

type type::create_intrinsic_type(intrinsic_type&& intr)
{
    type t;
    t.tclass = INTRINSIC;
    t.intr = move(intr);
    return t;
}

type::type() : tclass(UNDEFINED) {}

type::type(type&& other)
{
    create(move(other));
}

type::type(const type& other)
{
    create((other));
}

type::~type()
{
    destroy();
}

type& type::operator=(type&& other)
{
    assign(move(other));
    return *this;
}

type& type::operator=(const type& other)
{
    assign((other));
    return *this;
}

bool type::operator<(const type& other) const
{
    return less_than_cmp(other);
}

bool type::less_than_cmp(const type& other) const
{
    if (this->tclass != other.tclass)
    {
        return this->tclass < other.tclass;
    }
    else
    {
        switch (this->tclass)
        {
        case type_class::UNDEFINED:
            return false;
        case type_class::VOID:
            return false;
        case type_class::LITERAL:
            return this->lit < other.lit;
        case type_class::BUILTIN:
            return this->bin < other.bin;
        case type_class::FUNCTION:
            return this->fun < other.fun;
        case type_class::INTRINSIC:
            return this->intr < other.intr;
        case type_class::TUPLE:
            return this->tup < other.tup;
        case type_class::STRUCT:
           throw internal_except_todo();
        case type_class::UNION:
            return this->un < other.un;
        case type_class::INTERSECT:
            throw internal_except_todo();
        default:
            throw internal_except_with_location("missing enum type in switch statement for");
        }
    }
}

bool type::callable() const
{
    return tclass == type_class::FUNCTION || tclass == type_class::INTRINSIC;// || (tclass == type_class::BUILTIN && this->bin == builtin_type::INTRINSIC);// TODO labmda, functor etc
}
// if callable, supports:
type_id type::return_type() const
{
    assert(callable());

    switch (tclass)
    {
    case type_class::FUNCTION:
        return this->fun.ret;
    case type_class::INTRINSIC:
        return type_id::VOID;
    default:
        throw internal_except_with_location("return_type() called on non-callable type");
    }
}

size_t type::param_count() const
{
    assert(callable());

    switch (tclass)
    {
    case type_class::FUNCTION:
        return this->fun.param_count();
    case type_class::INTRINSIC:
        return 2;
    default:
        throw internal_except_with_location("param_count() called on non-callable type");
    }
}

type_id type::param_type(size_t idx) const
{
    assert(callable());

    switch (tclass)
    {
    case type_class::FUNCTION:
        return this->fun[idx].tid;
    case type_class::INTRINSIC:
        return this->intr[idx];
    default:
        throw "NOT CALLABLE";
    }
}

type& type::assign(type&& other)
{
    this->destroy();
    return create(move(other));
}

type& type::assign(const type& other)
{
    this->destroy();
    return create((other));
}

type& type::create(type&& other)
{
    this->tclass = other.tclass;
    other.tclass = type_class::UNDEFINED;
    switch (this->tclass)
    {
    case type_class::UNDEFINED:
        break;
    case type_class::VOID:
        break;
    case type_class::LITERAL:
    {
        new(&this->lit) literal_type(move(other.lit));
        break;    
    }
    case type_class::BUILTIN:
    {
        new(&this->bin) builtin_type(move(other.bin));
        break;
    }
    case type_class::INTRINSIC:
    {
        new(&this->intr) intrinsic_type(move(other.intr));
        break;
    }
    case type_class::FUNCTION:
    {
        new(&this->fun) function_type(move(other.fun));
        break;
    }
    case type_class::TUPLE:
    {
        new(&this->tup) tuple_type(move(other.tup));
        break;
    }
    case type_class::STRUCT:
        throw "TODO";
    case type_class::UNION:
    {
        new(&this->un) union_type(move(other.un));
        break;
    }
    case type_class::INTERSECT:
        throw "TODO";
    default:
        throw "TODO";
    }
    return *this;
}

type& type::create(const type& other)
{
    this->tclass = other.tclass;
    switch (this->tclass)
    {
    case type_class::UNDEFINED:
        break;
    case type_class::VOID:
        break;
    case type_class::LITERAL:
    {
        new(&this->lit) literal_type(other.lit);
        break;    
    }
    case type_class::BUILTIN:
    {
        new(&this->bin) builtin_type(other.bin);
        break;
    }
    case type_class::INTRINSIC:
    {
        new(&this->intr) intrinsic_type((other.intr));
        break;
    }
    case type_class::FUNCTION:
    {
        new(&this->fun) function_type(other.fun);
        break;
    }
    case type_class::TUPLE:
    {
        new(&this->tup) tuple_type(other.tup);
        break;
    }
    case type_class::STRUCT:
        throw "TODO";
    case type_class::UNION:
    {
        new(&this->un) union_type(other.un);
        break;
    }
    case type_class::INTERSECT:
        throw "TODO";
    default:
        throw "TODO";
    }
    return *this;
}

void type::destroy()
{
    switch (this->tclass)
    {
    case type_class::UNDEFINED:
        break;
    case type_class::VOID:
        break;
    case type_class::LITERAL:
        break;
    case type_class::BUILTIN:
        break;
    case type_class::INTRINSIC:
    {
        this->intr.~intrinsic_type();
        break;
    }
    case type_class::FUNCTION:
    {
        this->fun.~function_type();
        break;
    }
    case type_class::TUPLE:
    {
        this->tup.~tuple_type();
        break;
    }
    case type_class::STRUCT:
        throw "TODO";
    case type_class::UNION:
    {
        this->un.~union_type();
        break;
    }
    case type_class::INTERSECT:
        throw "TODO";
    default:
        throw "TODO";
    }
}

literal_type expr_kind_to_literal_type(expr::expr_kind kind)
{
    switch (kind)
    {
    case expr::TRUE_LITERAL:
        return literal_type::TRUE;
    case expr::FALSE_LITERAL:
        return literal_type::FALSE;
    case expr::STRING_LITERAL:
        return literal_type::STRING;
    case expr::DECIMAL_LITERAL:
        return literal_type::DECIMAL;
    case expr::INTEGER_LITERAL:
        return literal_type::INTEGER;
    default:
        throw "INTERNAL ERROR";
    }
}

const char* type_class_str(type_class tc)
{
    switch (tc)
    {
    case type_class::VOID:
        return "VOID";
    case type_class::UNDEFINED:
        return "UNDEFINED";
    case type_class::LITERAL:
        return "LITERAL";
    case type_class::BUILTIN:
        return "BUILTIN";
    case type_class::INTRINSIC:
        return "INTRINSIC";
    case type_class::FUNCTION:
        return "FUNCTION";
    case type_class::TUPLE:
        return "TUPLE";
    case type_class::STRUCT:
        return "STRUCT";
    case type_class::UNION:
        return "UNION";
    case type_class::INTERSECT:
        return "INTERSECT";
    default:
        return "???";
    }
}

string to_string(const type_id& t)
{
    return string(type_class_str(t.tclass)).append("#").append(to_string(t.idx));
}

string to_string(literal_type lt)
{
    switch (lt)
    {
    case literal_type::STRING: return "literal#string";
    case literal_type::INTEGER : return "literal#int";
    case literal_type::DECIMAL: return "literal#dec";
    case literal_type::TRUE: return "literal#true";
    case literal_type::FALSE: return "literal#false";
    case literal_type::EMPTY_TUPLE: return "literal#()";
    default:
        return "???";
    }
}

string to_string(builtin_type bt)
{
    return eng_us_name(bt);
}

string eng_us_name(builtin_type bt)
{
    switch (bt)
    {
    case builtin_type::BOOL: return "bool";
    case builtin_type::INT8: return "int8";
    case builtin_type::INT16: return "int16";
    case builtin_type::INT32: return "int32";
    case builtin_type::INT64: return "int64";
    case builtin_type::UINT8: return "uint8";
    case builtin_type::UINT16: return "uint16";
    case builtin_type::UINT32: return "uint32";
    case builtin_type::UINT64: return "uint64";
    case builtin_type::FLOAT16: return "float16";
    case builtin_type::FLOAT32: return "float32";
    case builtin_type::FLOAT64: return "float64";
    //case builtin_type::ASCII_STRING: return "ascii";
    case builtin_type::BYTE: return "byte";
    case builtin_type::ASCII: return "ascii";
    case builtin_type::UNICODE: return "unicode";
    case builtin_type::TYPEID: return "typeid";
    default:
        return "???";
    }
}

type_registry::type_registry()
{
    // undefined is always available
    _id2t_map[type_class::UNDEFINED].push_back(type());
}

type& type_registry::find_type(type_id id)
{
    if (id.idx >= _id2t_map[id.tclass].size())
    {
        throw "TODO";
    }
    return _id2t_map[id.tclass][id.idx];
}

const type& type_registry::find_type(type_id id) const
{
    if (id.idx >= _id2t_map[id.tclass].size())
    {
        throw "TODO";
    }
    return _id2t_map[id.tclass][id.idx];
}

type_id type_registry::find_type_id(const type& ty) const
{
    auto it = _t2id_map.find(ty);
    if (it == _t2id_map.end())
    {
        throw "TODO";
        // return register_type(ty);
    }
    return it->second;
}

type_id type_registry::find_void_type() const
{
    return find_type_id(type::create_void_type());
}

type_id type_registry::find_builtin_type_id(builtin_type binty) const
{
    return find_type_id(type::create_builtin_type(move(binty)));
}

type_id type_registry::find_literal_type_id(literal_type litty) const
{
    return find_type_id(type::create_literal_type(move(litty)));
}

type_id type_registry::find_type_id_auto_register(const type& ty)
{
    auto it = _t2id_map.find(ty);
    if (it == _t2id_map.end())
    {
        if (ty.tclass == LITERAL || ty.tclass == BUILTIN || ty.tclass == VOID)
        {
            throw "TODO";
        }
        return register_type(ty);
    }
    return it->second;
}

void type_registry::check_exists(type_id tid)
{
    if (!exists(tid))
    {
        throw internal_except(string::join("type_registry::check_exists(): type_id doesn't exist: ", to_string(tid)));
    }
}

void type_registry::check_subtypes(const type& ty)
{
    switch (ty.tclass)
    {
    case type_class::VOID:
        return;
    case type_class::UNDEFINED:
        return;
    case type_class::LITERAL:
        return;
    case type_class::BUILTIN:
        return;
    case type_class::INTRINSIC:
    {
        switch (ty.intr.config)
        {
        case intrinsic_type::NONE:
            break;
        case intrinsic_type::DEST_ONLY:
            check_exists(ty.intr[intrinsic_type::DEST_PARAM]);
            break;
        case intrinsic_type::OP_ONLY:
            check_exists(ty.intr[intrinsic_type::OP_PARAM]);
            break;
        case intrinsic_type::BOTH:
            check_exists(ty.intr[intrinsic_type::DEST_PARAM]);
            check_exists(ty.intr[intrinsic_type::OP_PARAM]);
            break;
        default:
            throw internal_except_unhandled_switch(to_string(ty.intr.config));
        }
        return;
    }
    case type_class::FUNCTION:
    {
        for (size_t i = 0; i < ty.fun.param_count(); ++i)
        {
            check_exists(ty.fun[i].tid);
        }
        check_exists(ty.fun.ret);
        return;
    }
    case type_class::TUPLE:
    {
        for (size_t i = 0; i < ty.tup.arity(); ++i)
        {
            check_exists(ty.tup[i].tid);
        }
        return;
    }
    case type_class::STRUCT:
        return;
    case type_class::UNION:
        return;
    case type_class::INTERSECT:
        return;
    default:
        throw "WTF";
    }
}

type_id type_registry::register_type(const type& ty)
{
    assert(!exists(ty));

    check_subtypes(ty);

    type_id id = type_id(ty.tclass, _id2t_map[ty.tclass].size());
    _id2t_map[ty.tclass].push_back(ty);
    _t2id_map[ty] = id;
    return id;
}

bool type_registry::exists(type_id tid) const
{
    if (tid == type_id::UNDEFINED)
    {
        return true; // undef alwyad exists
    }
    constexpr size_t TCLASS_MAP_SIZE = sizeof(_id2t_map) / sizeof(_id2t_map[0]);
    return tid.tclass < TCLASS_MAP_SIZE && tid.idx < _id2t_map[tid.tclass].size();
}

bool type_registry::exists(const type& ty) const
{
    return _t2id_map.find(ty) != _t2id_map.end();
}

// void register_void(type_registry& types)
// {
//     types.register_type(type::create_void_type());
// }

// void register_literal_types(type_registry& types)
// {
//     enum_iterable<literal_type, literal_type::_label_FIRST, literal_type::_label_LAST> e;
//     for (auto it = e.begin(); it != e.end(); ++it)
//     {
//         types.register_type(type::create_literal_type(*it));
//     }
// }

// void register_builtin_types(type_registry& types)
// {
//     enum_iterable<builtin_type, builtin_type::_label_FIRST, builtin_type::_label_LAST> e;
//     for (auto it = e.begin(); it != e.end(); ++it)
//     {
//         types.register_type(type::create_builtin_type(*it));
//     }
// }

string type_registry::tyname(type_id tid) const
{
    return tyname(find_type(tid));
}

string type_registry::tyname(const type& ty) const
{
    switch (ty.tclass)
    {
    case type_class::UNDEFINED:
        return "undefined";
    case type_class::LITERAL:
        return to_string(ty.lit);
    case type_class::BUILTIN:
        return to_string(ty.bin);
    case type_class::FUNCTION:
    {
        string s = generic::foldr(
            ty.fun,
            0,
            ty.fun.param_count(),
            string(),
            [this](const string& acc, const function_type::param& prm)
            {
                return string::join(acc, tyname(prm.tid)); // TODO param name
            });
        return string::join(s, " -> ", tyname(ty.fun.ret));
    }
    case type_class::TUPLE:
    {
        string s = "(";
        if (ty.tup.arity() > 0)
        {
            for (size_t i = 0; i < ty.tup.arity() - 1; ++i)
            {
                s.append(tyname(find_type(ty.tup[i].tid)));
                s.append(", ");
            }
            s.append(tyname(find_type(ty.tup[ty.tup.arity() - 1].tid)));
        }
        return s.append(")");
    }
    case type_class::STRUCT:
        return "STRUCT TODO";
    case type_class::UNION:
        return "UNION TODO";
    case type_class::INTERSECT:
        return "INTERSECT TODO";
    default:
        return "???";
    }
}

// literal_type& type_registry::find_literal_info(type_idx);
// builtin_type& type_registry::find_builtin_info(type_idx);
// function_type& type_registry::find_function_info(type_idx);
// //struct_type& find_struct(type_idx);
// tuple_type& type_registry::find_tuple_info(type_idx);
// union_type& type_registry::find_union_info(type_idx);

// type_id type_registry::find_literal(const literal_type& )
// {
//     throw "TODO";
// }

// // type_id type_registry::find_builtin(const builtin_type&);
// // type_id type_registry::find_function(const function_type&);
// // //type_idx find_struct(const struct_type&);
// // type_id type_registry::find_tuple(const tuple_type&);
// // type_id type_registry::find_union(const union_type&);

// // FIND will registrer tuype if not found.
// type_id type_registry::register_literal_info(literal_type lt)
// {
//     // TODO use set to check existence?
//     type_idx idx = _l_map.size();
//     _l_map.push_back(lt);
//     return type_id(type_class::LITERAL, idx);
// }

// // type_id type_registry::register_builtin_info(builtin_type);
// // type_id type_registry::register_function_info(function_type&&);
// // //type_idx register_struct(struct_type&&);
// // type_id type_registry::register_union_info(union_type&&);

// // registered minimum supported types
// //void type_registry::register_required();

// bool type_registry::exists(type_id) const
// {
//     throw "TODO";
//}

}