#ifndef LU_BYTE_H
#define LU_BYTE_H

#include <cstddef>
#include "internal/constexpr.h"

namespace lu
{
#ifdef __cpp_lib_byte
using byte = std::byte;
#else // __cpp_lib_byte
enum class byte : unsigned char {};
// TODO byte operators
#endif // __cpp_lib_byte

namespace byte_size_literals
{
    LU_CONSTEXPR size_t KB_SIZE = 1024;
    LU_CONSTEXPR size_t MB_SIZE = 1024 * KB_SIZE;
    LU_CONSTEXPR size_t GB_SIZE = 1024 * KB_SIZE;
    LU_CONSTEXPR size_t TB_SIZE = 1024 * KB_SIZE;

    LU_CONSTEXPR size_t operator ""_b(unsigned long long int b)
    {
        return b;
    }

    LU_CONSTEXPR size_t operator ""_KB(long double kb)
    {
        return static_cast<size_t>(kb * KB_SIZE);
    }

    LU_CONSTEXPR size_t operator ""_MB(long double mb)
    {
        return static_cast<size_t>(mb * MB_SIZE);
    }

    LU_CONSTEXPR size_t operator ""_GB(long double gb)
    {
        return static_cast<size_t>(gb * GB_SIZE);
    }

    LU_CONSTEXPR size_t operator ""_TB(long double tb)
    {
        return static_cast<size_t>(tb * TB_SIZE);
    }

    LU_CONSTEXPR size_t operator ""_KB(unsigned long long int kb)
    {
        return static_cast<size_t>(kb * KB_SIZE);
    }

    LU_CONSTEXPR size_t operator ""_MB(unsigned long long int mb)
    {
        return static_cast<size_t>(mb * MB_SIZE);
    }

    LU_CONSTEXPR size_t operator ""_GB(unsigned long long int gb)
    {
        return static_cast<size_t>(gb * GB_SIZE);
    }

    LU_CONSTEXPR size_t operator ""_TB(unsigned long long int tb)
    {
        return static_cast<size_t>(tb * TB_SIZE);
    }
}

}

#endif // LU_BYTE_H
