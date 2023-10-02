#include "value.h"
#include "except.h"
#include "utility.h"

namespace lu
{
union_value::union_value(union_value&& other) : active(move(other.active)), val(move(other.val))
{}

union_value::union_value(const union_value& other) : active((other.active)), val(make_unique(new intermediate_value(*other.val)))
{}

tuple_value::tuple_value(tuple_value&& other) : vals(move(other.vals))
{}

tuple_value::tuple_value(const tuple_value& other) : vals(other.vals)
{}

intermediate_value::intermediate_value(type_id tid) : _tid(tid)
{
    create();
}

intermediate_value::intermediate_value(intermediate_value&& other)
{
    create(move(other));
}

intermediate_value::intermediate_value(const intermediate_value& other)
{
    create((other));
}


intermediate_value::~intermediate_value()
{
    destroy();
}

intermediate_value& intermediate_value::operator=(intermediate_value&& other)
{
    return this->assign(move(other));
}

intermediate_value& intermediate_value::operator=(const intermediate_value& other)
{
    return this->assign((other));
}

void intermediate_value::destroy()
{
    switch (_tid.tclass)
    {
    case type_class::UNDEFINED:
        break; // do nothing
    case type_class::VOID:
        break;
    case type_class::LITERAL:
        lit.~literal_value();
        break;
    case type_class::BUILTIN:
        bin.~builtin_value();
        break;
    case type_class::FUNCTION:
        func.~function_value();
        break;
    case type_class::TUPLE:
        tup.~tuple_value();
        break;
    case type_class::UNION:
        un.~union_value();
        break;
    //case type_class::STRUCT: // TODO
    //case type_class::UNION: // TODO get active type
    //case type_class::INTERSECT: // TODO get actual type
    default:
        throw internal_except_unhandled_switch(to_string(_tid.tclass));
        break;
    }
}

intermediate_value& intermediate_value::create()
{
    switch (_tid.tclass)
    {
    case type_class::UNDEFINED:
        break; // do nothing
    case type_class::VOID:
        break;
    case type_class::LITERAL:
        new(&this->lit) literal_value();
        break;
    case type_class::BUILTIN:
        new(&this->bin) builtin_value();
        break;
    case type_class::FUNCTION:
        new(&this->func) function_value();
        break;
    case type_class::INTRINSIC:
        new(&this->intr) intrinsic_value();
        break;
    case type_class::TUPLE:
        new(&this->tup) tuple_value();
        break;
    case type_class::UNION:
        new(&this->un) union_value();
        break;
    //case type_class::STRUCT: // TODO
    //case type_class::UNION: // TODO get active type
    //case type_class::INTERSECT: // TODO get actual type
    default:
        throw internal_except_todo();
    }
    return *this;
}

intermediate_value& intermediate_value::create(intermediate_value&& other)
{
    this->_tid = move(other._tid);
    other._tid = type_id::UNDEFINED;
    switch (_tid.tclass)
    {
    case type_class::UNDEFINED:
        break; // do nothing
    case type_class::VOID:
        break;
    case type_class::LITERAL:
        new(&this->lit) literal_value(move(other.lit));
        break;
    case type_class::BUILTIN:
        new(&this->bin) builtin_value(move(other.bin));
        break;
    case type_class::FUNCTION:
        new(&this->func) function_value(move(other.func));
        break;
    case type_class::INTRINSIC:
        new(&this->intr) intrinsic_value(move(other.intr));
        break;
    case type_class::TUPLE:
        new(&this->tup) tuple_value(move(other.tup));
        break;
    case type_class::UNION:
        new(&this->un) union_value(move(other.un));
        break;
    //case type_class::STRUCT: // TODO
    //case type_class::UNION: // TODO get active type
    //case type_class::INTERSECT: // TODO get actual type
    default:
        throw internal_except_todo();
    }
    return *this;
}

intermediate_value& intermediate_value::create(const intermediate_value& other)
{
    this->_tid = (other._tid);
    switch (_tid.tclass)
    {
    case type_class::UNDEFINED:
        break; // do nothing
    case type_class::VOID:
        break;
    case type_class::LITERAL:
        new(&this->lit) literal_value((other.lit));
        break;
    case type_class::BUILTIN:
        new(&this->bin) builtin_value((other.bin));
        break;
    case type_class::FUNCTION:
        new(&this->func) function_value((other.func));
        break;
    case type_class::INTRINSIC:
        new(&this->intr) intrinsic_value(other.intr);
        break;
    case type_class::TUPLE:
        new(&this->tup) tuple_value((other.tup));
        break;
    case type_class::UNION:
        new(&this->un) union_value((other.un));
        break;
    //case type_class::STRUCT: // TODO
    //case type_class::UNION: // TODO get active type
    //case type_class::INTERSECT: // TODO get actual type
    default:
        throw internal_except_todo();
    }
    return *this;
}

intermediate_value& intermediate_value::assign(intermediate_value&& other)
{
   this->destroy();
   return this->create(move(other));
}

intermediate_value& intermediate_value::assign(const intermediate_value& other)
{
   this->destroy();
   return this->create((other));
}

void intermediate_value_table::probe(symbol_id sid)
{
    auto it = _vals.find(sid);
    if (it == _vals.end())
    {
        _vals.insert(std::pair<symbol_id, intermediate_value>(sid, intermediate_value()));
    }
}

intermediate_value& intermediate_value_table::operator[](symbol_id sid)
{
    probe(sid);
    auto it = _vals.find(sid);

    assert(it != _vals.end());

    return _vals.find(sid)->second;
}

const intermediate_value& intermediate_value_table::operator[](symbol_id sid) const
{
    auto it = _vals.find(sid);

    assert(it != _vals.end());

    return it->second;
}

}