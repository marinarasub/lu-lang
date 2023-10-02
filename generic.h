#ifndef LU_GENERIC_H
#define LU_GENERIC_H

#include <algorithm>

namespace lu
{
namespace generic
{

// TODO
// template <typename Iterable>
// foldr()
// {

// }

template <typename Indexable, typename BaseT, typename FoldT>
BaseT foldr(const Indexable& items, size_t begin, size_t end, BaseT base, FoldT func)
{
    BaseT res = base;
    for (size_t i = begin; i < end; ++i)
    {
        res = func(res, items[i]);
    }
    return res;
}

template <typename InputIterable, typename Pred>
bool any(const InputIterable& in, Pred p)
{
    return std::any_of(in.begin(), in.end(), p);
}

// alias of any
// template <typename InputIterable, typename Pred>
// constexpr bool exists(const InputIterable& in, Pred p)
// {
//     return any(in, p);
// }

template <typename InputIterable, typename Pred>
bool all(const InputIterable& in, Pred p)
{
    return std::all_of(in.begin(), in.end(), p);
}

}
}

#endif // LU_GENERIC_H
