#ifndef LU_SCOPE_H
#define LU_SCOPE_H

#include "string.h"
#include "utility.h"
#include "adt/map.h"
#include "adt/vector.h"
#include "symbol.h"
#include "intrinsic.h"
#include <cstdint>

namespace lu
{

struct lexical_scope
{
public:
    lexical_scope(const string& label) : _label(label), _parent(nullptr) {}

    lexical_scope() : _parent(nullptr) {}

    ~lexical_scope();

    // find local
    //...

    void declare(string_view, symbol_id);

    lexical_scope* sub(size_t idx) { return _subs[idx]; }

    lexical_scope* up() { return _parent; }

    lexical_scope* push_sub();
    
    symbol_id find_innermost_local(string_view);
    symbol_id find_local(string_view);

private:
    void destroy();

    string _label; // namespace name, for FQDN lookup, if no name cannot be referenced from another scope.
    flat_map<string, symbol_id> _sym_map; // def name -> def id, TODO overloding - probably much later, adds quite a bit of complexity to resolution?
    // TODO check each is used & defined.

    lexical_scope* _parent;
    vector<lexical_scope*> _subs;
};



struct symbol_table
{
    symbol_table(); // top is always valid
    ~symbol_table();

    symbol_table(symbol_table&&);

    symbol_table& operator=(symbol_table&&);

    lexical_scope* top();

    symbol& declare(lexical_scope* p_scope, symbol&& sym);
    symbol_id find_innermost(lexical_scope* p_scope, string_view) const;
    symbol_id find_local(lexical_scope* p_scope, string_view) const;

    // unlike most languages, user definitions in top level are still considered file scoped, not global. global can added using keyword or be added by compiler for needed vars that are not intrinsic functions
    symbol& declare_global(symbol&&);
    symbol_id find_global(string_view) const;

    intrinsic& declare_intrinsic(intrinsic&&);
    intrinsic& find_intrinsic(intrinsic_id);
    const intrinsic& find_intrinsic(intrinsic_id) const;
    intrinsic_id find_intrinsic_id(string_view) const;

    symbol& operator[](symbol_id sid) { assert(exists(sid)); return _syms[sid]; }
    const symbol& operator[](symbol_id sid) const { assert(exists(sid)); return _syms[sid]; }

    void merge(const symbol_table&);

    bool exists(symbol_id sid) const { return sid < _syms.size(); }

private:
    symbol_id next_id() const
    {
        return _syms.size();
    }

    intrinsic_id next_intr_id() const
    {
        return _intrs.size();
    }

    void destroy();
    void create(symbol_table&&);

    lexical_scope* _top;
    flat_map<string, intrinsic_id> _intr_map;
    map<string, symbol_id> _globs;

    vector<intrinsic> _intrs; // probalby a pointer to global
    vector<symbol> _syms;
};

// struct lexical_scope_stack
// {
//     lexical_scope& top();
//     void push(lexical_scope&&);
//     void pop();

//     // find innermost
// private:
//     vector<lexical_scope> _scopes;
// };

}

#endif // LU_SCOPE_H
