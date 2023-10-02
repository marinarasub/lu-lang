#include "intrinsic.h"

#include "string.h"
#include "print.h"
#include "internal/constexpr.h"
#include "except.h"
#include <cstdint>

namespace lu
{
    namespace internal
    {
        namespace intrinsic_functions
        {
            void i32print(void*, const void* op)
            {
                const int32_t* p_int32 = static_cast<const int32_t*>(op); 
                print(to_string(*p_int32));
            }

            void i32add(void* dest, const void* op)
            {
                int32_t* p_dest = static_cast<int32_t*>(dest);
                const int32_t* p_op = static_cast<const int32_t*>(op);
                *p_dest = *p_dest + *p_op;
            }
        }
    
    }
    
const char* intrinsic_code_cstr(intrinsic_code icode)
{
    switch (icode)
    {
    case I32PRINT:
        return "I32PRINT";
    case I64PRINT:
        return "I64PRINT";
    case U32PRINT:
        return "U32PRINT";
    case U64PRINT:
        return "U64PRINT";
    case I32ADD:
        return "I32ADD";
    case I64ADD:
        return "I64ADD";
    case U32ADD:
        return "U32ADD";
    case U64ADD:
        return "U64ADD";
    case BPRINT:
        return "BPRINT";
    case ASCIIPRINT:
        return "ASCIIPRINT";
    case LNEG:
        return "LNEG";
    case LAND:
        return "LAND";
    case LOR:
        return "LOR";
    default:
        throw internal_except_unhandled_switch(to_string(icode));
    }
}

} // namespace lu
