#ifndef LU_ENUM_H
#define LU_ENUM_H

#include <type_traits>
#include <iterator>

namespace lu
{

template <typename EnumT, typename = typename std::enable_if<std::is_enum<EnumT>::value>::type>
typename std::underlying_type<EnumT>::type to_underlying_type(EnumT e)
{
    return static_cast<typename std::underlying_type<EnumT>::type>(e);
}

template <typename EnumT, EnumT First, EnumT Last>
struct enum_iterator
{
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = EnumT;
    using difference_type   = std::ptrdiff_t;
    using pointer           = const EnumT*;
    using reference         = const EnumT&;

    static_assert(std::is_enum<EnumT>::value, "enum iterator must be on enum");

    using enum_type = typename std::underlying_type<EnumT>::type;

public:
    enum_iterator(EnumT e) : _val(to_underlying_type(e)) {}

    enum_iterator operator+(difference_type diff)
    {
        difference_type dval = static_cast<difference_type>(_val) + diff;
        return enum_iterator(static_cast<enum_type>(_val)); 
    }
    enum_iterator& operator++() { ++_val; return *this; }
    enum_iterator operator++(int) { enum_iterator ret(_val); ++_val; return ret; }

    EnumT operator*() { return static_cast<EnumT>(_val); }

    bool operator==(enum_iterator other) const
    {
        return this->_val == other._val;
    }

    bool operator!=(enum_iterator other) const
    {
        return this->_val != other._val;
    }

    bool operator<(enum_iterator other) const
    {
        return this->_val < other._val;
    }

private:
    enum_type _val;
};

template <typename EnumT, EnumT First, EnumT Last, typename = typename std::enable_if<std::is_enum<EnumT>::value>::type>
struct enum_iterable
{
    using iterator = enum_iterator<EnumT, First, Last>;
    using const_iterator = iterator;

    const_iterator begin() const
    {
        return const_iterator(First);
    }

    const_iterator end() const
    {
        return ++const_iterator(Last);
    }
};

}

#endif // LU_ENUM_H
