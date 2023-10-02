#ifndef LU_MAP_H
#define LU_MAP_H

#include "vector.h"
#include "search.h"

#include <map>
#include <unordered_map>
#include <cstddef>
#include <cmath>

namespace lu
{
template <typename KeyT, typename ValT>
struct flat_map
{
    struct const_key_value
    {
        const KeyT& key;
        const ValT& value;
    };

    struct key_value
    {
        const KeyT& key;
        ValT& value;
    };

    struct const_iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        //using value_type        = key_value;
        using difference_type   = std::ptrdiff_t;
        //using pointer           = const T*;
        using reference         = key_value;

        const_iterator(const vector<KeyT>& keys, const vector<ValT>& vals, size_t idx) : _keys(keys), _vals(vals), _pos(idx) {}

        bool operator<(const const_iterator& other) { return this->_pos < other._pos; }
        bool operator==(const const_iterator& other) { return this->_pos == other._pos; }
        bool operator!=(const const_iterator& other) { return this->_pos != other._pos; }
        const_iterator operator+(difference_type diff)
        {
            difference_type dpos = static_cast<difference_type>(_pos) + diff; // assert _pos + diff> 0?
            return const_iterator(_keys, _vals, static_cast<size_t>(dpos)); 
        }
        const_iterator& operator++() { ++_pos; return *this; }
        const_iterator operator++(int) { const_iterator ret(_keys, _vals, _pos); ++_pos; return ret; }
        const_key_value operator*() { return { _keys[_pos], _vals[_pos] }; }
    private:
        const vector<KeyT>& _keys;
        const vector<ValT>& _vals;
        size_t _pos;
    };

    struct iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        //using value_type        = key_value;
        using difference_type   = std::ptrdiff_t;
        //using pointer           = const T*;
        using reference         = key_value;

        iterator(const vector<KeyT>& keys, vector<ValT>& vals, size_t idx) : _keys(keys), _vals(vals), _pos(idx) {}

        bool operator<(const iterator& other) { return this->_pos < other._pos; }
        bool operator==(const iterator& other) { return this->_pos == other._pos; }
        bool operator!=(const iterator& other) { return this->_pos != other._pos; }
        iterator operator+(difference_type diff)
        {
            difference_type dpos = static_cast<difference_type>(_pos) + diff; // assert _pos + diff> 0?
            return iterator(_keys, _vals, static_cast<size_t>(dpos)); 
        }
        iterator& operator++() { ++_pos; return *this; }
        iterator operator++(int) { iterator ret(_keys, _vals, _pos); ++_pos; return ret; }
        const key_value operator*() { return { _keys[_pos], _vals[_pos] }; }
    private:
        const vector<KeyT>& _keys;
        vector<ValT>& _vals;
        size_t _pos;
    };

    size_t size() const
    {
        return _keys.size();
    }

    iterator begin()
    {
        return iterator(_keys, _vals, 0);
    }

    iterator end()
    {
        return iterator(_keys, _vals, size());
    }

    const_iterator begin() const
    {
        return const_iterator(_keys, _vals, 0);
    }

    const_iterator end() const
    {
        return const_iterator(_keys, _vals, size());
    }

    template <typename LikeKeyT>
    iterator find(const LikeKeyT& key)
    {
        binary_search_result bsr = binary_search(_keys, key);
        return bsr.found ? iterator(_keys, _vals,  bsr.index) : end();
    }

    template <typename LikeKeyT>
    const_iterator find(const LikeKeyT& key) const
    {
        binary_search_result bsr = binary_search(_keys, key);
        return bsr.found ? const_iterator(_keys, _vals,  bsr.index) : end();
    }

    template <typename LikeKeyT>
    const ValT& operator[](const LikeKeyT& key) const
    {
        return (*find(key)).value;
    }

    template <typename LikeKeyT>
    ValT& operator[](const LikeKeyT& key)
    {
        auto it = find(key);
        if (it != end())
        {
            return (*it).value;
        }
        return (*insert(key, ValT())).value;
    }

    template <typename LikeKeyT, typename LikeValT>
    iterator insert(const LikeKeyT& key, const LikeValT& val)
    {
        binary_search_result bsr = binary_search(_keys, key);
        if (bsr.found)
        {
            _vals[bsr.index] = val;
            return iterator(_keys, _vals, bsr.index);
        }
        else
        {
            if (bsr.index > _keys.size())
            {
                _keys.emplace_back(key);
                _vals.emplace_back(val);
            }
            else
            {
                _keys.emplace(_keys.begin() + static_cast<typename vector<KeyT>::difference_type>(bsr.index), key);
                _vals.emplace(_vals.begin() + static_cast<typename vector<KeyT>::difference_type>(bsr.index), val);
            }
            return iterator(_keys, _vals, bsr.index);
        }
    }

private:

    vector<KeyT> _keys; // share index with vals, so should always be same size
    vector<ValT> _vals;
    
};


// template <typename KeyT, typename ValT>
// struct tree_map
// {
//     struct key_value
//     {
//         const KeyT& key;
//         const ValT& value;
//     };

// private:
//     struct key_value_data
//     {
//         KeyT key;
//         ValT value;
//     };

//     internal::avl_tree<key_value_data> _tree; 
// };

// template <typename KeyT, typename ValT>
// using map = tree_map<KeyT, ValT>;

using std::map;
using std::unordered_map;

}

#endif // LU_MAP_H
