#ifndef LU_CONSTEXPR_H
#define LU_CONSTEXPR_H

#define LU_INLINE inline

#if __cpp_constexpr >= 200704L
#define LU_CONSTEXPR constexpr
#else
#define LU_CONSTEXPR
#endif // __cpp_constexpr >= 200704L

#if __cpp_constexpr >= 201304L
#define LU_CONSTEXPR_IF_CXX14 constexpr
#else
#define LU_CONSTEXPR_IF_CXX14 inline
#endif // __cpp_constexpr >= 201304L

#endif // LU_CONSTEXPR_H
