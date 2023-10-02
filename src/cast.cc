#include "cast.h"
#include "internal/debug.h"
#include "except.h"
#include "string.h"
#include "type.h"
#include <cstdlib> // for strtof etc.
#include <limits>
#include <type_traits>

namespace lu
{

// check from base to check type
template <typename IntCheckT, typename IntBaseT>
bool int_cast_in_range(IntBaseT x)
{
    using std::numeric_limits;

    // if both same sign
    if (std::is_signed<IntBaseT>::value == std::is_signed<IntCheckT>::value)
    {
        return x >= static_cast<IntBaseT>(numeric_limits<IntCheckT>::min()) &&
            x <= static_cast<IntBaseT>(numeric_limits<IntCheckT>::max());
    }
    // only base is signed (check cannot have -)
    else if (std::is_signed<IntBaseT>::value)
    {
        return x >= 0 &&
            x >= static_cast<IntBaseT>(numeric_limits<IntCheckT>::min()) &&
            x <= static_cast<IntBaseT>(numeric_limits<IntCheckT>::max());
    }
    // only check is signed (base cannot be negative so only check positive limit)
    else// if (std::is_signed<IntCheckT>::value)
    {
        return x <= static_cast<IntBaseT>(numeric_limits<IntCheckT>::max());
    }    
}

intermediate_value bool_literal_to_builtin_cast(const type_registry& types, type_id to, const intermediate_value& val)
{
    type_id from = val.tid();
    literal_type fromlit = types.find_type(from).lit ;

    assert(from.is(LITERAL));
    assert(fromlit == literal_type::TRUE || fromlit == literal_type::FALSE);
    assert(to.is(BUILTIN));

    intermediate_value rval(to);

    builtin_type bint = types.find_type(to).bin;
    switch (bint)
    {
    case builtin_type::BOOL:
    {
        if (fromlit == literal_type::TRUE)
        {
            rval.bin.b = true;
        }
        else // FALSE
        {
            rval.bin.b = false;
        }
        return rval;
    }
    default:
        throw internal_except_unhandled_switch(to_string(bint));
    }
}

// TODO give logger for converison warning??
intermediate_value int_literal_to_builtin_cast(const type_registry& types, type_id to, const intermediate_value& val)
{
    type_id from = val.tid();

    assert(from.is(LITERAL));
    assert(types.find_type(from).lit == literal_type::INTEGER);
    assert(to.is(BUILTIN));

    intermediate_value rval(to);
    char* end;
    unsigned long long ull = std::strtoull(val.lit.text.buffer(), &end, 10);

    assert(end == (val.lit.text.buffer() + val.lit.text.size()));

    builtin_type bint = types.find_type(to).bin;
    switch (bint)
    {
    //case builtin_type::BOOL:
    case builtin_type::INT8:
    {
        if (!int_cast_in_range<decltype(rval.bin.i8)>(ull))
        {
            // TODO warn narrowing
            throw internal_except_todo();
        }
        rval.bin.i8 = static_cast<decltype(rval.bin.i8)>(ull);
        return rval;
    }
    case builtin_type::INT16:
    {
        if (!int_cast_in_range<decltype(rval.bin.i16)>(ull))
        {
            // TODO warn narrowing
            throw internal_except_todo();
        }
        rval.bin.i16 = static_cast<decltype(rval.bin.i16)>(ull);
        return rval;
    }
    case builtin_type::INT32:
    {
        if (!int_cast_in_range<decltype(rval.bin.i32)>(ull))
        {
            // TODO warn narrowing
            throw internal_except_todo();
        }
        rval.bin.i32 = static_cast<decltype(rval.bin.i32)>(ull);
        return rval;
    }
    case builtin_type::INT64:
    {
        if (!int_cast_in_range<decltype(rval.bin.i64)>(ull))
        {
            // TODO warn narrowing
            throw internal_except_todo();
        }
        rval.bin.i64 = static_cast<decltype(rval.bin.i64)>(ull);
        return rval;
    }
    case builtin_type::UINT8:
    {
        if (!int_cast_in_range<decltype(rval.bin.u8)>(ull))
        {
            // TODO warn narrowing
            throw internal_except_todo();
        }
        rval.bin.u8 = static_cast<decltype(rval.bin.u8)>(ull);
        return rval;
    }
    case builtin_type::UINT16:
    {
        if (!int_cast_in_range<decltype(rval.bin.u16)>(ull))
        {
            // TODO warn narrowing
            throw internal_except_todo();
        }
        rval.bin.u16 = static_cast<decltype(rval.bin.u16)>(ull);
        return rval;
    }
    case builtin_type::UINT32:
    {
        if (!int_cast_in_range<decltype(rval.bin.u32)>(ull))
        {
            // TODO warn narrowing
            throw internal_except_todo();
        }
        rval.bin.u32 = static_cast<decltype(rval.bin.u32)>(ull);
        return rval;
    }
    case builtin_type::UINT64:
    {
        if (!int_cast_in_range<decltype(rval.bin.u64)>(ull))
        {
            // TODO warn narrowing
            throw internal_except_todo();
        }
        rval.bin.u64 = static_cast<decltype(rval.bin.u64)>(ull);
        return rval;
    }
    // case builtin_type::FLOAT16:
    // case builtin_type::FLOAT32:
    // case builtin_type::FLOAT64:
    // case builtin_type::BYTE:
    // case builtin_type::ASCII:
    default:
        throw internal_except_unhandled_switch(to_string(bint));
    }
}

intermediate_value string_literal_to_builtin_cast(const type_registry& types, type_id to, const intermediate_value& val)
{
    type_id from = val.tid();
    literal_type fromlit = types.find_type(from).lit;

    assert(from.is(LITERAL));
    assert(fromlit == literal_type::STRING);
    assert(to.is(BUILTIN));

    intermediate_value rval(to);

    builtin_type bint = types.find_type(to).bin;
    switch (bint)
    {
    case builtin_type::ASCII:
    {
        if (val.lit.text.size() != 1)
        {
            throw internal_except(val.lit.text); // TODO diag
        }
        rval.bin.ascii = val.lit.text[0]; //note: unescaped string
        return rval;
    }
    // TODO: ascii array etc.
    default:
        throw internal_except_unhandled_switch(to_string(bint));
    }
}

intermediate_value literal_to_builtin_cast(const type_registry& types, type_id to, const intermediate_value& val)
{
    type_id from = val.tid();

    assert(from.is(LITERAL));
    assert(to.is(BUILTIN));
    
    literal_type lit = types.find_type(from).lit;
    switch (lit)
    {
    case literal_type::STRING:
        return string_literal_to_builtin_cast(types, to, val);
    case literal_type::INTEGER:
        return int_literal_to_builtin_cast(types, to, val);
    case literal_type::DECIMAL:
        throw internal_except_todo();
    case literal_type::TRUE: // falltrough
    case literal_type::FALSE:
        return bool_literal_to_builtin_cast(types, to, val);
    case literal_type::EMPTY_TUPLE:
        throw internal_except_todo();
    default:
        throw internal_except_unhandled_switch(to_string(lit));
    }
}

}