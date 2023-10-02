#ifndef LU_INTRINSIC_H
#define LU_INTRINSIC_H

#include "type.h"
#include "string.h"
#include "internal/constexpr.h"
#include <limits>

namespace lu
{

using intrinsic_id = size_t;

using intrinsic_function_type = void(*)(void*, const void*);

// TODO function is not implemented by cimpiler driver, move elsewhere to vm/interpreter impl.
namespace internal
{
    namespace intrinsic_functions
    {
        // TO simplify things, ALL intrinsics must take in 2 params and return void: void*, const void*. 1st or 2nd may be ignored if not needed

        void i32print(void*, const void* op);
        void i32add(void* dest, const void* op);
        void u32add(void* dest, const void* op);
        void lneg(void* dest, const void*);
        void land(void* dest, const void* op);
        void lor(void* dest, const void* op);
    }
}

enum intrinsic_code
{
    I32PRINT,
    I64PRINT,
    U32PRINT,
    U64PRINT,
    I32ADD,
    I64ADD,
    U32ADD,
    U64ADD,
    BPRINT,
    ASCIIPRINT,
    LNEG,
    LAND,
    LOR,
};

const char* intrinsic_code_cstr(intrinsic_code icode);

struct intrinsic
{
    LU_CONSTEXPR static intrinsic_id INVALID_ID = std::numeric_limits<intrinsic_id>::max();

    intrinsic(string_view name, intrinsic_code code, intrinsic_type::param_config config, type_id tid1, type_id tid2) : name(name), icode(code), itype(intrinsic_type(config, tid1, tid2)) {}

    string name;
    intrinsic_code icode;
    intrinsic_type itype;
    //const intrinsic_function_type func;
    // TODO fptr

    bool operator<(const intrinsic& other) const
    {
        return (this->name != other.name) ? (this->name < other.name) :
            ((this->icode != other.icode) ? (this->icode < other.icode) : (this->itype < other.itype));// && this->func < other.func;
    }

    //void operator()(void* dest, const void* op) const { func(dest, op); }
};

namespace intrinsics
{
    // const intrinsic i32add = intrinsic("i32add", builtin_type::INT32, builtin_type::INT32, internal::intrinsic_functions::i32add);
    // const intrinsic i32print = intrinsic("i32print", builtin_type::VOID, builtin_type::INT32, internal::intrinsic_functions::i32print);
}

}

#endif // LU_INTRINSIC_H
