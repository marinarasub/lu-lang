#ifndef LU_ARRAY_H
#define LU_ARRAY_H

#include <cstddef>
#include <cassert>
#include <iterator>
#include <type_traits>
#include <utility>

namespace lu
{
// non compile time, fixed size array
template <typename T>
struct array
{
    array() : _buf(nullptr), _size(0) {}
    explicit array(size_t size) : _buf(size > 0 ? alloc(size) : nullptr), _size(size) {}

    template <typename = typename std::enable_if<!std::is_same<T, size_t>::value>::type>
    array(T&& val) : _buf(alloc(1)), _size(1)
    {
        _buf[0] = move(val);
    }

    array(std::initializer_list<T>&& il) : array(il.begin(), il.end())
    {}

    template <typename InputIt>
    array(InputIt it, InputIt end)
    {
        using category = typename std::iterator_traits<InputIt>::iterator_category;
        create_from_it(it, end, category());
    }

    array(const T* it, const T* end)
    {
        create_from_ptr(it, end);
    }

    array(const array& other) : _buf(other._size > 0 ? alloc(other._size) : nullptr), _size(other._size)
    {
        for (size_t i = 0; i < _size; ++i)
        {
            _buf[i] = other[i];
        }
    }

    array(array&& other) : _buf(other._buf), _size(other._size)
    {
        other._buf = nullptr;
        other._size = 0;
    }
    
    array& operator=(array&& other)
    {
        this->_buf = other._buf;
        this->_size = other._size;
        other._buf = nullptr;
        other._size = 0;
        return *this;
    }

    array& operator=(const array& other)
    {
        dealloc();
        if (other._size > 0)
        {
            _buf = alloc(other._size);
            _size = other._size;
            for (size_t i = 0; i < _size; ++i)
            {
                _buf[i] = other[i];
            }
        }
        else
        {
            _buf = nullptr;
            _size = 0;
        }
        return *this;
    }

    ~array() { dealloc(); }

    void clear()
    {
        dealloc();
        _size = 0;
    }

    size_t size() const { return _size; }

    T& at(size_t idx) { assert(idx < _size); return _buf[idx]; }
    const T& at(size_t idx) const { assert(idx < _size); return _buf[idx]; }

    T& operator[](size_t idx) { assert(idx < _size); return _buf[idx]; }
    const T& operator[](size_t idx) const { assert(idx < _size); return _buf[idx]; }

    void fill(const T& val)
    {
        for (size_t i = 0; i < size(); ++i) _buf[i] = val;
    }

    // lexicographic compare
    bool operator<(const array& other) const
    {
        size_t n = std::min(this->size(), other.size());
        size_t i;
        for (i = 0; i < n; ++i)
        {
            if (this->at(i) < other.at(i))
            {
                return true;
            }
            else if (other.at(i) < this->at(i))
            {
                return false;
            }
        }
        return this->size() < other.size();
    }

private:
    T* alloc(size_t n)
    {
        return new T[n]; // TODO maybe raw char buf? HOW to do align though
    }

    void dealloc()
    {
        assert(_size == 0 || _buf != nullptr);

        if (_buf != nullptr) delete[] _buf;
    }

    template <typename InputIt>
    void create_from_it(InputIt it, InputIt end, std::random_access_iterator_tag)
    {
        assert(std::distance(it, end) >= 0);
        _size = static_cast<size_t>(std::distance(it, end));
        _buf = alloc(_size);
        for (size_t i = 0; it != end; ++i, ++it)
        {
            _buf[i] = *it;
        }
    }

    void create_from_ptr(const T* it, const T* end)
    {
        assert(end >= it);
        _size = static_cast<size_t>(end - it);
        _buf = alloc(_size);
        for (size_t i = 0; it != end; ++i, ++it)
        {
            _buf[i] = *it;
        }
    }

    T* _buf;
    size_t _size;
};    
} // namespace lu



#endif // LU_ARRAY_H
