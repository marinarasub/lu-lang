#ifndef LU_SET_H
#define LU_SET_H

#include "vector.h"
#include "search.h"

#include <iterator>
#include <cstddef>
#include <set>

namespace lu
{
// set using flat array for small sized sets
template <typename T>
struct flat_set
{
    struct const_iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const T*;
        using reference         = const T&;

        const_iterator(const vector<T>& elems, size_t idx) : _elems(elems), _pos(idx) {}

        bool operator<(const const_iterator& other) { return this->_pos < other._pos; }
        bool operator==(const const_iterator& other) { return this->_pos == other._pos; }
        bool operator!=(const const_iterator& other) { return this->_pos != other._pos; }
        const_iterator operator+(difference_type diff)
        {
            difference_type dpos = static_cast<difference_type>(_pos) + diff;
            return const_iterator(_elems, static_cast<size_t>(dpos)); 
        }
        const_iterator& operator++() { ++_pos; return *this; }
        const_iterator operator++(int) { const_iterator ret(_elems, _pos); ++_pos; return ret; }
        const T& operator*() { return _elems[_pos]; }
    private:
        const vector<T>& _elems;
        size_t _pos;
    };

    size_t size() const
    {
        return _elems.size();
    }

    const_iterator begin() const
    {
        return const_iterator(_elems, 0);
    }

    const_iterator end() const
    {
        return const_iterator(_elems, size());
    }

    void insert(const T& elem)
    {
        binary_search_result bsr = binary_search(_elems, elem);
        if (bsr.found)
        {
            _elems[bsr.index] = elem;
        }
        else
        {
            if (bsr.index > _elems.size())
            {
                _elems.push_back(elem);
            }
            else
            {
                _elems.insert(_elems.begin() + static_cast<typename vector<T>::difference_type>(bsr.index), elem);
            }
        }
    }

    const_iterator find(const T& elem) const
    {
        binary_search_result bsr = binary_search(_elems, elem);
        return const_iterator(_elems, bsr.index);
    }

    const_iterator erase(const T& elem)
    {
        binary_search_result bsr = binary_search(_elems, elem);
        if (bsr.found)
        {
            _elems.erase(_elems.begin() + static_cast<typename vector<T>::difference_type>(bsr.index));
        }
        return const_iterator(_elems, bsr.index);
    }

    bool contains(const T& elem) const
    {
        binary_search_result bsr = binary_search(_elems, elem);
        return bsr.found;
    }

    // can compare linearly since both are sorted.
    bool operator<(const flat_set& other) const
    {
        if (this->size() == other.size())
        {
            bool ret = (this->size() > 0);
            size_t i = 0;
            while (ret && i < this->size())
            {
                ret = (this->index(i) < other.index(i));
            }
            return ret;
        }
        else
        {
            return this->size() < other.size();
        }
    }

private:
    const T& index(size_t idx) const
    {
        return _elems.at(idx);
    }

    vector<T> _elems;
};

#include <set>

}


#endif // LU_SET_H
