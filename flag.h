#ifndef LU_FLAG_H
#define LU_FLAG_H

#include "internal/constexpr.h"

namespace lu
{
template <typename BitT>
struct flags
{
    using flag_type = BitT;

    LU_CONSTEXPR flags() : _flags(flag_type(0)) {}
    LU_CONSTEXPR flags(flag_type raw_flags) : _flags(raw_flags) {}
    LU_CONSTEXPR flags(const flags& other) : _flags(other._flags) {}

    flags& operator=(flags other)
    {
        this->_flags = other._flags;
        return *this;
    }

    void clear()
    {
        this = NONE;
    }

    bool any(flags mask) const
    {
        return (this->_flags & mask._flags) != 0;
    }

    bool all(flags mask) const
    {
        return (this->_flags & mask._flags) == mask._flags;
    }

    LU_CONSTEXPR friend flags operator|(flags lhs, flags rhs);
    LU_CONSTEXPR friend flags operator&(flags lhs, flags rhs);
    LU_CONSTEXPR friend flags operator^(flags lhs, flags rhs);
    LU_CONSTEXPR friend flags operator~(flags);

    const static flags NONE;
    const static flags ALL;

protected:
    flag_type _flags;
};

template <typename BitT> const flags<BitT> flags<BitT>::NONE = flags<BitT>(flags<BitT>::flag_type(0));
template <typename BitT> const flags<BitT> flags<BitT>::ALL = ~flags<BitT>::NONE;

template <typename BitT>
LU_CONSTEXPR flags<BitT> operator|(flags<BitT> lhs, flags<BitT> rhs)
{
    return flags<BitT>(lhs._flags | rhs._flags);
}

template <typename BitT>
LU_CONSTEXPR flags<BitT> operator&(flags<BitT> lhs, flags<BitT> rhs)
{
    return flags<BitT>(lhs._flags & rhs._flags);
}

template <typename BitT>
LU_CONSTEXPR flags<BitT> operator^(flags<BitT> lhs, flags<BitT> rhs)
{
    return flags<BitT>(lhs._flags ^ rhs._flags);
}

template <typename BitT>
LU_CONSTEXPR flags<BitT> operator~(flags<BitT> f)
{
    return flags<BitT>(~f._flags);
}

}

#endif // LU_FLAG_H
