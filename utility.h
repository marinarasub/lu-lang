#ifndef LU_UTILITY_H
#define LU_UTILITY_H

#include <utility>
#include <memory>
#include <algorithm>
#include <cstddef>

namespace lu
{
using std::move;
using std::forward;

template <typename T>
using unique = std::unique_ptr<T>;

template <typename T>
unique<T> make_unique(T* ptr)
{
    return unique<T>(ptr);
}

}

#endif // LU_UTILITY_H
