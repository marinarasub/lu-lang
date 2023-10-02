#ifndef LU_STRING_H
#define LU_STRING_H

#include <cstddef>
#include <limits>
#include <cstdint>
#include <ostream>
#include <iterator>

#include "internal/constexpr.h"

namespace lu
{

struct encoding // char traits?
{

};

const encoding ascii;// = char;
const encoding utf8;// = char32_t; // we store all unicode as 32 for fast random access.

struct string_view;

struct string
{
    using CharT = char;

    LU_CONSTEXPR static size_t npos = std::numeric_limits<size_t>::max();

    friend struct string_view;

    struct iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = typename string::CharT;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const typename string::CharT*;
        using reference         = const typename string::CharT&;

        iterator(string::CharT* buf, size_t idx) : _buf(buf), _pos(idx) {}

        bool operator<(const iterator& other) { return this->_pos < other._pos; }
        bool operator==(const iterator& other) { return this->_pos == other._pos; }
        bool operator!=(const iterator& other) { return this->_pos != other._pos; }
        iterator operator+(difference_type diff)
        {
            difference_type dpos = static_cast<difference_type>(_pos) + diff;
            return iterator(_buf, static_cast<size_t>(dpos)); 
        }
        iterator& operator++() { ++_pos; return *this; }
        iterator operator++(int) { iterator ret(_buf, _pos); ++_pos; return ret; }
        const string::CharT& operator*() { return _buf[_pos]; }
    private:
        string::CharT* _buf;
        size_t _pos;
    };

    struct const_iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = typename string::CharT;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const typename string::CharT*;
        using reference         = const typename string::CharT&;

        const_iterator(const string::CharT* buf, size_t idx) : _buf(buf), _pos(idx) {}

        bool operator<(const const_iterator& other) { return this->_pos < other._pos; }
        bool operator==(const const_iterator& other) { return this->_pos == other._pos; }
        bool operator!=(const const_iterator& other) { return this->_pos != other._pos; }
        const_iterator operator+(difference_type diff)
        {
            difference_type dpos = static_cast<difference_type>(_pos) + diff;
            return const_iterator(_buf, static_cast<size_t>(dpos)); 
        }
        const_iterator& operator++() { ++_pos; return *this; }
        const_iterator operator++(int) { const_iterator ret(_buf, _pos); ++_pos; return ret; }
        const string::CharT& operator*() { return _buf[_pos]; }
    private:
        const string::CharT* _buf;
        size_t _pos;
    };

    LU_CONSTEXPR string() : _buf(nullptr), _size(0), _cap(0) {}
    ~string();
    string(const CharT*);
    string(const CharT*, size_t n);
    string(size_t n, CharT);
    string(const string&);
    string(string&&);
    string(const string_view&);

    void clear();

    string& operator=(const string&);
    string& operator=(string&&);

    size_t size() const;
    bool empty() const;

    const CharT* buffer() const noexcept { return _buf; }

    CharT& operator[](size_t);
    const CharT& operator[](size_t) const;

    void reserve(size_t n);

    string& append(CharT);
    string& append(const CharT*);
    string& append(const CharT*, size_t);
    string& append(const string&);
    string& append(const string_view&);

    template <typename StrLikeT>
    static string join(const StrLikeT& first)
    {
        return first;
    }

    template <typename StrLikeT, typename... RestT>
    static string join(const StrLikeT& first, RestT ...rest)
    {
        string s(first);
        return s.append(join(rest...));
    }

    iterator begin() { return iterator(_buf, 0); }
    iterator end() { return iterator(_buf, _size); }

    const_iterator begin() const { return const_iterator(_buf, 0); }
    const_iterator end() const { return const_iterator(_buf, _size); }

    //string_view view() const;

private:
    // TODO SBO. not a priority rn.
    CharT* _buf;
    size_t _size;
    size_t _cap;
};

// constexpr strlen

namespace internal
{
    LU_CONSTEXPR size_t strlen(const string::CharT* cstr, size_t count)
    {
        return cstr[count] == '\0' ? count : strlen(cstr, count + 1);
    }
}

LU_CONSTEXPR size_t strlen(const string::CharT* cstr)
{
    return internal::strlen(cstr, 0);
}

//

struct string_view
{
    friend struct string;
    friend bool operator<(string_view, string_view);
    friend bool operator==(string_view, string_view);
    friend bool operator!=(string_view, string_view);
    friend bool operator>(string_view, string_view);

    LU_CONSTEXPR static size_t npos = std::numeric_limits<size_t>::max();
    
    struct const_iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = typename string::CharT;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const typename string::CharT*;
        using reference         = const typename string::CharT&;

        const_iterator(const string::CharT* buf, size_t idx) : _buf(buf), _pos(idx) {}

        bool operator<(const const_iterator& other) { return this->_pos < other._pos; }
        bool operator==(const const_iterator& other) { return this->_pos == other._pos; }
        bool operator!=(const const_iterator& other) { return this->_pos != other._pos; }
        const_iterator operator+(difference_type diff)
        {
            difference_type dpos = static_cast<difference_type>(_pos) + diff;
            return const_iterator(_buf, static_cast<size_t>(dpos)); 
        }
        const_iterator& operator++() { ++_pos; return *this; }
        const_iterator operator++(int) { const_iterator ret(_buf, _pos); ++_pos; return ret; }
        const string::CharT& operator*() { return _buf[_pos]; }
    private:
        const string::CharT* _buf;
        size_t _pos;
    };

    // 1. construct an empty string
    LU_CONSTEXPR string_view() noexcept : _buf(nullptr), _size(0) {}
    LU_CONSTEXPR string_view(const string_view& sv) : _buf(sv._buf), _size(sv._size) {}
    // 2. construct string view over a string
    LU_CONSTEXPR string_view(const string& s) : _buf(s._buf), _size(s._size) {}
    LU_CONSTEXPR string_view(const string::CharT* cstr, size_t size) : _buf(cstr), _size(size) {}
    LU_CONSTEXPR string_view(const string::CharT* cstr) : _buf(cstr), _size(strlen(cstr)) {}

    string_view& operator=(const string_view& sv);

    size_t size() const;
    bool empty() const;

    const string::CharT& operator[](size_t) const;

    string_view subview(size_t offset, size_t len = npos) const;

    // return a copy of contents
    string str() const;

    const string::CharT* buffer() const noexcept { return _buf; }

    const_iterator begin() const { return const_iterator(_buf, 0); }
    const_iterator end() const { return const_iterator(_buf, _size); }

private:
    const string::CharT* _buf;
    size_t _size;
};

std::ostream& operator<<(std::ostream&, string_view);

bool operator<(string_view, string_view);
bool operator==(string_view, string_view);
bool operator>(string_view, string_view);
bool operator!=(string_view, string_view);

string to_string(string_view);
string to_string(float);
string to_string(double);
string to_string(long double);
string to_string(int);
string to_string(long);
string to_string(long long);
string to_string(unsigned);
string to_string(unsigned long);
string to_string(unsigned long long);

}

#endif // LU_STRING_H
