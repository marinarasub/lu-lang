#ifndef LU_SEARCH_H
#define LU_SEARCH_H

#include "compare.h"
#include <cassert>

namespace lu
{
struct binary_search_result
{
    size_t index; // if not found, where the element would go (before index, so 0 means at start of container, n means end of container).
    bool found;
};

namespace internal
{
    // i must be between l and r (inclusive)
    template <typename RAContainerT, typename T, typename CompareLess>
    binary_search_result binary_search(const RAContainerT& elems, const T& key, size_t l, size_t r, CompareLess less_than)
    {
        size_t i = (l + r) / 2;

        if (i >= elems.size())
        {
            return binary_search_result{ elems.size(), false };
        }
        
        bool lt = less_than(key, elems[i]);
        bool gt = less_than(elems[i], key);
        if (l >= r) // i should also equal both here
        {
            return binary_search_result{ i, !lt && !gt };
        }
        else if (!lt && !gt) // equal to semantically
        {
            return binary_search_result{ i, true };
        }
        else if (lt)
        {
            return binary_search(elems, key, l, i, less_than);
        }
        else
        {
            return binary_search(elems, key, i + 1, r, less_than);
        }
    }
}

template <typename RAContainerT, typename T, typename CompareLess = less<T>>
binary_search_result binary_search(const RAContainerT& elems, const T& elem)
{
    if (elems.size() == 0)
    {
        return binary_search_result{ 0, false };
    }
    CompareLess less_than;
    return internal::binary_search(elems, elem, 0, elems.size(), less_than);
}
}

#endif // LU_SEARCH_H
