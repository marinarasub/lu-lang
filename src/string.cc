#include "string.h"

#include "internal/int_util.h"
#include "internal/constexpr.h"

#include <cassert>
#include <algorithm>
#include <cstring>
#include <cstdlib>

namespace lu
{

// const encoding ascii;// = char;
// const encoding utf8;// = char32_t; // we store all unicode as 32 for fast random access.

namespace internal
{
    LU_CONSTEXPR size_t choose_cap(size_t size)
    {
        return size == 0 ? 0 : int_util::next_pwr2_ifnotpwr2(size);
    }
}

string::~string()
{
    if (_buf != nullptr) delete[] _buf;
}

void string::clear()
{
    if (_buf != nullptr) delete[] _buf;
    _size = 0;
    _cap = 0;
}

string::string(const string::CharT* cstr) : string(cstr, strlen(cstr)) {}

string::string(size_t size, CharT c) : _size(size), _cap(internal::choose_cap(size + 1)) // +1 for null char
{
    if (size > 0)
    {
        _buf = new CharT[_cap];
        memset(_buf, c, size);
        _buf[_size] = '\0';
    }
    else
    {
        _buf = nullptr;
    }
}

string::string(const string::CharT* cstr, size_t size) : _size(size), _cap(internal::choose_cap(size + 1)) // +1 for null char
{
    if (size > 0)
    {
        _buf = new CharT[_cap];
        memcpy(_buf, cstr, _size);
        _buf[_size] = '\0';
    }
    else
    {
        _buf = nullptr;
    }
}

string::string(const string& s) : string(s._buf, s._size) {}

string::string(string&& s) : _buf(s._buf), _size(s._size), _cap(s._cap)
{
    s._buf = nullptr;
    s._size = 0;
    s._cap = 0;
}

string::string(const string_view& sv) : string(sv.buffer(), sv.size())
{}

string& string::operator=(const string& other)
{
    clear();
    _size = (other._size);
    _cap = (internal::choose_cap(_size + 1));
    if (_size > 0)
    {
        _buf = new CharT[_cap];
        memcpy(_buf, other._buf, _size);
        _buf[_size] = '\0';
    }
    else
    {
        _buf = nullptr;
    }
    return *this;
}

string& string::operator=(string&& other)
{
    clear();
    this->_buf = other._buf;
    this->_size = other._size;
    this->_cap = other._cap;
    other._buf = nullptr;
    other._size = 0;
    other._cap = 0;
    return *this;
}

size_t string::size() const
{
    return _size;
}

bool string::empty() const
{
    return _size == 0;
}

string::CharT& string::operator[](size_t idx)
{
    assert(idx < _size);

    return _buf[idx];        
}

const string::CharT& string::operator[](size_t idx) const
{
    assert(idx < _size);

    return _buf[idx];
}

void string::reserve(size_t new_cap)
{
    new_cap = new_cap + 1; // for null char
    if (new_cap > _cap)
    {
        _cap = internal::choose_cap(new_cap);
        CharT* new_buf = new CharT[_cap];
        if (_buf != nullptr)
        {
            memcpy(new_buf, _buf, _size + 1);
        }
        delete[] _buf;
        _buf = new_buf;
    }
}

string& string::append(CharT c)
{
    reserve(_size + 1);
    _buf[_size] = c;
    _size += 1;
    _buf[_size] = '\0';
    return *this;
}

string& string::append(const CharT* cstr)
{
    return append(cstr, strlen(cstr));
}

string& string::append(const CharT* cstr, size_t n)
{
    if (n > 0)
    {
        reserve(_size + n);
        memcpy(&_buf[_size], cstr, n);
        _size += n;
        _buf[_size] = '\0';
    }
    return *this;
}

string& string::append(const string& s)
{
    return append(s._buf, s._size);
}

string& string::append(const string_view& sv)
{
    return append(sv.buffer(), sv.size());
}



//string_view::string_view(const string::CharT*) {}

string_view& string_view::operator=(const string_view& sv)
{
    this->_buf = sv._buf;
    this->_size = sv._size;
    return *this;
}

const string::CharT& string_view::operator[](size_t idx) const
{
    assert(idx < _size);

    return _buf[idx];
}

size_t string_view::size() const
{
    return _size;
}

bool string_view::empty() const
{
    return _size == 0;
}

string_view string_view::subview(size_t offset, size_t new_len) const
{
    const string::CharT* new_buf = _buf + offset;

    assert(offset <= _size);

    return string_view(new_buf, std::min(new_len, _size - offset));
}

string string_view::str() const
{
    return string(_buf, _size);
}

std::ostream& operator<<(std::ostream& os, string_view sv)
{
    for (string::CharT c : sv)
    {
        os << c;
    }
    return os;
}

bool operator<(string_view lhs, string_view rhs)
{
    if (lhs._size == 0 || rhs._size == 0) // sz should be 0
    {
        return lhs._size < rhs._size;
    }
    int cmp = memcmp(lhs._buf, rhs._buf, std::min(lhs.size(), rhs.size()));
    return cmp < 0 || (cmp == 0 && lhs.size() < rhs.size());
}

bool operator==(string_view lhs, string_view rhs)
{
    return lhs.size() == rhs.size() && memcmp(lhs._buf, rhs._buf, lhs.size()) == 0;
}

bool operator!=(string_view lhs, string_view rhs)
{
    return lhs.size() != rhs.size() || memcmp(lhs._buf, rhs._buf, lhs.size()) != 0;
}

bool operator>(string_view lhs, string_view rhs)
{
    if (lhs._size == 0 || rhs._size == 0) // sz should be 0
    {
        return lhs._size > rhs._size;
    }
    int cmp = memcmp(lhs._buf, rhs._buf, std::min(lhs.size(), rhs.size()));
    return cmp > 0 || (cmp == 0 && lhs.size() > rhs.size());
}

string to_string(string_view sv)
{
    return string(sv.buffer(), sv.size());
}

template <typename T>
string to_string_snprintf(const char* fmt, T val)
{
    int n = snprintf(nullptr, 0, fmt, val);
    
    assert(n > 0);

    if (n + 1 > 32)
    {
        char* buf = new char[static_cast<size_t>(n) + 1];
        snprintf(buf, static_cast<size_t>(n) + 1, fmt, val);
        return string(buf, static_cast<size_t>(n));
    }
    else
    {
        char buf[32];
        snprintf(buf, 32, fmt, val);
        return string(buf, static_cast<size_t>(n));
    }
}

string to_string(float val)
{
    return to_string_snprintf("%f", val);
}

string to_string(double val)
{
    return to_string_snprintf("%f", val);
}

string to_string(long double val)
{
    return to_string_snprintf("%Lf", val);
}

string to_string(int val)
{
    return to_string_snprintf("%d", val);
}

string to_string(long val)
{
    return to_string_snprintf("%ld", val);
}

string to_string(long long val)
{
    return to_string_snprintf("%lld", val);
}

string to_string(unsigned val)
{
    return to_string_snprintf("%u", val);
}

string to_string(unsigned long val)
{
    return to_string_snprintf("%lu", val);
}

string to_string(unsigned long long val)
{
    return to_string_snprintf("%llu", val);
}

}