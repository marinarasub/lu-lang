#ifndef LU_DEBUG_H
#define LU_DEBUG_H

#include <cassert>

#ifdef NDEBUG
#define debug(...)
#else
#define debug(...) __VA_ARGS__
#endif // NDEBUG

#ifdef NDEBUG
#define ndebug(...) __VA_ARGS__
#else
#define ndebug(...)
#endif // NDEBUG

#ifndef __PRETTY_FUNCTION__
#define LU_FUNCTION_NAME __PRETTY_FUNCTION__
#elif !defined(LU_NOFUNC)
#define LU_FUNCTION_NAME __func__
#else
#define LU_FUNCTION_NAME "<__func__>"
#endif


#endif // LU_DEBUG_H