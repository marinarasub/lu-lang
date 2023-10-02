#include "symbol.h"
#include "utility.h"

namespace lu
{
symbol::symbol() : tid(type_id::UNDEFINED), name(), sid(INVALID_ID), flags()
{}

symbol::symbol(type_id tid, string_view name, lu::flags<symbol_flag> flags)
    : tid(tid), name(name), sid(INVALID_ID), flags(flags)
{}

symbol::symbol(symbol&& other) : tid(move(other.tid)), name(move(other.name)), sid(move(other.sid)), flags(move(other.flags))
{
    other.tid = type_id::UNDEFINED;
    other.sid = symbol::INVALID_ID;
}

symbol& symbol::operator=(symbol&& other)
{
    this->tid = move(other.tid);
    this->name = move(other.name);
    this->sid = move(other.sid);
    this->flags = move(other.flags);
    other.tid = type_id::UNDEFINED;
    other.sid = symbol::INVALID_ID;
    return *this;
}

symbol& symbol::operator=(const symbol& other)
{
    this->tid = (other.tid);
    this->name = (other.name);
    this->sid = (other.sid);
    this->flags = (other.flags);
    return *this;
}

symbol::symbol(const symbol& other) : tid((other.tid)), name((other.name)), sid((other.sid)), flags(other.flags)
{}

string symbol_id_string(symbol_id sid)
{
    return sid == symbol::INVALID_ID ? "INVALID" : to_string(sid);
}

}