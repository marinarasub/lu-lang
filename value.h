#ifndef LU_VALUE_H
#define LU_VALUE_H

#include "type.h"
#include "string.h"
#include "expr.h"
#include "intermediate_common.h"
#include "intrinsic.h"
#include "symbol.h"
#include "adt/map.h"
// TODO
//struct intermediate_expr;

#include <cstdint>

namespace lu
{
using value_id = size_t;

struct intermediate_value;

struct literal_value
{
    literal_value() {}
    literal_value(string_view sv) : text(sv) {}
    
    string text;
};

struct builtin_value
{
    builtin_value() {}

    ~builtin_value() {} // do nothing

    union
    {
        bool b;
        char ascii;
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float f32;
        double f64;
        type_id tid; // TODO: since type_id is not trivial type, this impl. requires type_id destructor to do nothing. maybe used uint64 and bit bask for type_id isntead?
    };
};

struct function_value
{
    intermediate_addr faddr;
};

struct intrinsic_value
{
    intrinsic_id iid;
};

struct union_value
{
    union_value() : active(type_id::UNDEFINED), val(nullptr) {}
    union_value(type_id active, unique<intermediate_value>&& curr) : active(active), val(move(curr)) {}
    union_value(union_value&&);
    union_value(const union_value&);

    type_id active;
    unique<intermediate_value> val;
};

struct tuple_value
{
    tuple_value() : vals() {}
    tuple_value(array<intermediate_value>&& vals) : vals(move(vals)) {}
    tuple_value(tuple_value&&);
    tuple_value(const tuple_value&);

    array<intermediate_value> vals;
};

struct intermediate_value
{
    intermediate_value() : _tid(type_id::UNDEFINED) {}
    intermediate_value(type_id tid);

    intermediate_value(intermediate_value&&);
    intermediate_value(const intermediate_value&);

    ~intermediate_value();

    intermediate_value& operator=(intermediate_value&& other);
    intermediate_value& operator=(const intermediate_value& other);

    type_id tid() const { return _tid; }

    union 
    {
        intermediate_value* p_val; // used for pointer
        void* p_arr; // ptr to bultin array
        literal_value lit;
        function_value func; // for function
        intrinsic_value intr;
        union_value un;
        tuple_value tup;
        builtin_value bin;
    };

private:
    intermediate_value& create();
    intermediate_value& create(intermediate_value&& other);
    intermediate_value& create(const intermediate_value& other);
    intermediate_value& assign(intermediate_value&& other);
    intermediate_value& assign(const intermediate_value& other);
    void destroy();

    type_id _tid;
};

struct intermediate_value_table
{
    intermediate_value_table() {}

    void probe(symbol_id);
    intermediate_value& operator[](symbol_id);
    const intermediate_value& operator[](symbol_id) const;
private:
    unordered_map<symbol_id, intermediate_value> _vals;
};

//using value = intermediate_value;

}

#endif // LU_VALUE_H
