#ifndef LU_SYMBOL_H
#define LU_SYMBOL_H

#include "adt/vector.h"
#include "type.h"
#include "string.h"
#include "flag.h"
#include <limits>
#include <cstdint>

namespace lu
{
    
using symbol_id = size_t;

enum class symbol_flag : uint32_t
{
    STATIC = (1 << 0),
    // INTRINSIC = (1 << 2),
};

struct symbol
{
    constexpr static symbol_id INVALID_ID = std::numeric_limits<symbol_id>::max();

    symbol();
    symbol(type_id, string_view name, flags<symbol_flag> = lu::flags<symbol_flag>::NONE); // id is generated
    symbol(symbol&&);
    symbol(const symbol&);

    symbol& operator=(symbol&&);
    symbol& operator=(const symbol&);

    type_id tid;
    string name; // name shouldn't be changed
    symbol_id sid; // TODO id shouldbe be changed after creation
    flags<symbol_flag> flags;
};

string symbol_id_string(symbol_id);

}

#endif // LU_SYMBOL_H
