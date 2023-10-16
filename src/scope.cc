#include "scope.h"
#include "intrinsic.h"
#include "type.h"
#include "enum.h"

namespace lu
{

lexical_scope::~lexical_scope()
{
    destroy();
}

void lexical_scope::destroy()
{
    for (const lexical_scope* p_sub : _subs)
    {
        delete p_sub;
    }
}

void lexical_scope::declare(string_view sname, symbol_id sid)
{
    assert(find_local(sname) == symbol::INVALID_ID);

    _sym_map.insert(sname, sid);
}

lexical_scope* lexical_scope::push_sub()
{
    lexical_scope* p_sub = new lexical_scope();
    p_sub->_parent = this;
    _subs.push_back(p_sub);
    return p_sub;
}

symbol_id lexical_scope::find_innermost_local(string_view sname)
{
    symbol_id ret = find_local(sname);
    if (ret != symbol::INVALID_ID || _parent == nullptr)
    {
        return ret;
    }
    return _parent->find_innermost_local(sname);
}

symbol_id lexical_scope::find_local(string_view sname)
{
    auto it =  _sym_map.find(sname);
    if (it != _sym_map.end())
    {
        return (*it).value;
    }
    else
    {
        return symbol::INVALID_ID;
    }
}

symbol_table::symbol_table() : _top(nullptr)
{}

symbol_table::~symbol_table()
{
    destroy();
}

symbol_table::symbol_table(symbol_table&& other)
{
    this->create(move(other));
}

symbol_table& symbol_table::operator=(symbol_table&& other)
{
    this->destroy();
    this->create(move(other));
    return *this;
}

lexical_scope* symbol_table::top()
{
    if (_top == nullptr)
    {
        _top = new lexical_scope();
    }
    return _top;
}

symbol& symbol_table::declare(lexical_scope* p_scope, symbol&& sym)
{
    symbol_id sid = next_id();
    sym.sid = sid;
    p_scope->declare(sym.name, sym.sid);
    _syms.push_back(move(sym));
    return _syms.back();
}

symbol_id symbol_table::find_innermost(lexical_scope* p_scope, string_view sname) const
{
    symbol_id sid = find_global(sname);
    if (sid != symbol::INVALID_ID)
    {
        return sid; 
    }
    return p_scope->find_innermost_local(sname);
}

symbol_id symbol_table::find_innermost_local(lexical_scope* p_scope, string_view sname) const
{
    return p_scope->find_innermost_local(sname);
}

symbol_id symbol_table::find_local(lexical_scope* p_scope, string_view sname) const
{
    return p_scope->find_local(sname);
}

symbol& symbol_table::declare_global(symbol&& sym)
{
    symbol_id sid = next_id();
    
    assert(_globs.find(sym.name) == _globs.end());
    
    sym.sid = sid;
    _globs[sym.name] = sym.sid;
    _syms.push_back(move(sym));
    return _syms.back();
}

symbol_id symbol_table::find_global(string_view sname) const
{
    auto it = _globs.find(sname);

    if (it == _globs.end())
    {
        return symbol::INVALID_ID; 
    }
    return (*it).second;
}

intrinsic& symbol_table::declare_intrinsic(intrinsic&& intr)
{
    intrinsic_id iid = next_intr_id();

    assert(_intr_map.find(intr.name) == _intr_map.end());
    _intr_map.insert(intr.name, iid);
    _intrs.push_back(move(intr));

    return _intrs.back();
}

intrinsic& symbol_table::find_intrinsic(intrinsic_id iid)
{
    assert(iid < _intrs.size());

    return _intrs[iid];
}

const intrinsic& symbol_table::find_intrinsic(intrinsic_id iid) const
{
    assert(iid < _intrs.size());

    return _intrs[iid];
}

intrinsic_id symbol_table::find_intrinsic_id(string_view iname) const
{
    auto it = _intr_map.find(iname);
    if (it == _intr_map.end())
    {
        return intrinsic::INVALID_ID;
    }

    return (*it).value;
}

void symbol_table::destroy()
{
    if (_top != nullptr)
    {
        delete _top;
    }
}

void symbol_table::create(symbol_table&& other)
{
    this->_top = other._top;
    this->_intr_map = move(other._intr_map);
    this->_globs = move(other._globs);

    this->_intrs = move(other._intrs);
    this->_syms = move(other._syms);

    other._top = nullptr;
}

}