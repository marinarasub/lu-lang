#ifndef LU_INT_UTIL_H
#define LU_INT_UTIL_H

#include <algorithm>

#include "constexpr.h"

namespace int_util
{    
template <typename T>
LU_CONSTEXPR T clamp(T val, T min, T max) noexcept(noexcept(std::min(max, std::max(min, val))))
{
    return std::min(max, std::max(min, val));
}
// returns integer result ceil(x / y) as if x, y were reals, T must be intengral type
template <typename T>
LU_CONSTEXPR T idiv_ceil(T x, T y)
{
    return x / y + (x % y != 0);
}

template <typename T>
LU_CONSTEXPR bool is_pwr2(T n)
{
    return (n & (n - 1)) == 0;
}

template <typename T>
LU_CONSTEXPR_IF_CXX14 T next_pwr2(T n)
{
    //printf("%zu -> ", n);
    unsigned char lsh = 0;
    while (n > 0)
    {
        n >>= 1;
        ++lsh;
    }
    //printf("%zu\n", ((size_t)1U) << lsh);
    return ((T)1U) << lsh;
}

template <typename T>
LU_CONSTEXPR_IF_CXX14 T next_pwr2_ifnotpwr2(T n)
{
    return is_pwr2(n) ? n : next_pwr2(n);
}

/*template <typename T>
static inline T align(T n, T align)
{
    return n / align + align;
}*/

}

#endif // LU_INT_UTIL_H
