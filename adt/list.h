#ifndef LU_LIST_H
#define LU_LIST_H

#include <cstddef>
#include <cassert>
#include "../internal/int_util.h"

namespace lu
{
#define LU_LIST_NODE_SIZE 256

// a convinient list iterface - use unrolled lsit for balance b/t cache and copy/resize performance?? good for mid sized lists
template <typename T>
class unrolled_list
{
protected:
    struct node
    {
        node() : size(0) {}
        node(node* next, node* prev) : next(next), prev(prev), size(0) {}
        //node(node* next) : next(nullptr), size(0) {}

        ~node()
        {
            for (size_t i = 0; i < size; ++i) { get(i)->~T(); }
        }

        static constexpr size_t best_cap()
        {
            return ((LU_LIST_NODE_SIZE - (2 * sizeof(node*) + sizeof(uint16_t))) / sizeof(T)) <= 4 ? 4 : ((LU_LIST_NODE_SIZE - (2 * sizeof(node*) + sizeof(uint16_t))) / sizeof(T));
        }

        static constexpr size_t CAPACITY = best_cap();

        bool empty() const
        {
            return size == 0;
        }

        void push_front(const T& elem)
        {
            assert(size < CAPACITY);

            slide_up();
            new(get(0)) T(elem);
            ++size;
        }

        void push_back(const T& elem)
        {
            assert(size < CAPACITY);

            new(get(size)) T(elem);
            ++size;
        }

        void pop_front()
        {
            assert(size > 0);

            //get(0)->~T(); // will be freed by slide down operator=
            slide_down();
            --size;
            get(size)->~T();
        }

        void pop_back()
        {
            assert(size > 0);

            --size;
            get(size)->~T();
        }
        
        bool full() const
        {
            return size == CAPACITY;
        }

        T& front()
        {
            assert(!empty());

            return (*this)[0];
        }

        T& back()
        {
            assert(!empty());

            return (*this)[size - 1];
        }

        T& operator[](size_t idx)
        {
            assert(idx < size);
            return *get(idx);
        }

        T* get(size_t idx)
        {
            return reinterpret_cast<T*>(&elems[idx * sizeof(T)]);
        }

        node* next;
        node* prev;
        unsigned char elems[CAPACITY * sizeof(T)];
        uint16_t size;

    private:
        void slide_up()
        {
            for (uint16_t i = size; i > 0; --i)
            {
                (*this)[i] = std::move((*this)[i - 1]);
            }
        }

        void slide_down()
        {
            for (uint16_t i = 1; i < size; ++i)
            {
                (*this)[i - 1] = std::move((*this)[i]);
            }
        }
    };

public:

    struct const_iterator
    {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = const T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const T*;
        using reference         = const T&;

        const_iterator(node* curr, size_t idx, size_t curr_idx) : _curr(curr), _idx(idx), _curr_idx(curr_idx) {}

        bool operator<(const const_iterator& other) { return this->_idx < other._idx; }
        bool operator==(const const_iterator& other) { return this->_idx == other._idx; }
        bool operator!=(const const_iterator& other) { return this->_idx != other._idx; }

        const_iterator& operator++() { next(); return *this; }
        const_iterator operator++(int) { const_iterator ret(_curr, _idx, _curr_idx); next(); return ret; }
        const_iterator& operator--() { prev(); return *this; }
        const_iterator operator--(int) { const_iterator ret(_curr, _idx, _curr_idx); prev(); return ret; }
        reference operator*() { return (*_curr)[_curr_idx]; }
    private:
        void next()
        {
            // assert not end iterator
            assert(_curr_idx < _curr->size);

            ++_idx;
            ++_curr_idx;
            if (_curr_idx >= _curr->size)
            {
                _curr_idx = 0;
                if (_curr->next != nullptr)
                {
                    _curr = _curr->next;
                }
            }
        }

        void prev()
        {
            // assert not begin iterator
            assert(_idx > 0);

            --_idx;
            if (_curr_idx == 0)
            {
                _curr = _curr->prev;

                assert(_curr->size > 0);

                _curr_idx = _curr->size - 1;
            }
            else
            {
                --_curr_idx;
            }
        }

        node* _curr;
        size_t _idx;
        size_t _curr_idx;
    };

    struct iterator
    {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        iterator(node* curr, size_t idx, size_t curr_idx) : _curr(curr), _idx(idx), _curr_idx(curr_idx) {}

        bool operator<(const iterator& other) { return this->_idx < other._idx; }
        bool operator==(const iterator& other) { return this->_idx == other._idx; }
        bool operator!=(const iterator& other) { return this->_idx != other._idx; }

        iterator& operator++() { next(); return *this; }
        iterator operator++(int) { iterator ret(_curr, _idx, _curr_idx); next(); return ret; }
        iterator& operator--() { prev(); return *this; }
        iterator operator--(int) { iterator ret(_curr, _idx, _curr_idx); prev(); return ret; }
        reference operator*() { return (*_curr)[_curr_idx]; }
    private:
        void next()
        {
            // assert not end iterator
            assert(_curr_idx < _curr->size);

            ++_idx;
            ++_curr_idx;
            if (_curr_idx >= _curr->size)
            {
                _curr_idx = 0;
                if (_curr->next != nullptr)
                {
                    _curr = _curr->next;
                }
            }
        }

        void prev()
        {
            // assert not begin iterator
            assert(_idx > 0);

            --_idx;
            if (_curr_idx == 0)
            {
                _curr = _curr->prev;

                assert(_curr->size > 0);

                _curr_idx = _curr->size - 1;
            }
            else
            {
                --_curr_idx;
            }
        }

        node* _curr;
        size_t _idx;
        size_t _curr_idx;
    };

    unrolled_list() : _head(nullptr), _tail(nullptr) /*, _avail(nullptr)*/, _size(0) {}

    // todo move, copy

    ~unrolled_list()
    {
        destroy();
    }

    unrolled_list(unrolled_list&& other) : _head(other._head), _tail(other._tail), _size(other._size)
    {
        other._head = nullptr;
        other._tail = nullptr;
        other._size = 0;
    }

    unrolled_list(const unrolled_list& other) : _head(nullptr), _tail(nullptr), _size(other._size)
    {
        if (_size == 0)
        {
            return;
        }
        
        for (const T& item : other)
        {
            push_back(item);
        }
    }

    void clear()
    {
        destroy();
        _head = nullptr;
        _tail = nullptr;
        //_avail = nullptr;
        _size = 0;
    }

    // TODO iterator

    bool empty() const
    {
        return _size == 0;
    }

    size_t size() const
    {
        return _size;
    }
    
    size_t capacity() const
    {
        return node::capacity * count_nodes();
    }

    const_iterator begin() const
    {
        return const_iterator(_head, 0, 0);
    }

    const_iterator end() const
    {
        return const_iterator(_tail, _size, _tail->size);
    }
    //void resize(size_t, const T&);

    // it's slower to pre reserve than just dynamically grow as needed
    // void reserve(size_t nelem)
    // {
    //     if (_head == nullptr)
    //     {
    //         init();
    //     }
    //     if (nelem > capacity())
    //     {
    //         size_t new_cap = int_util::next_pwr2_ifnotpwr2(nelem);
    //         size_t add_cap = new_cap - capacity();
    //         while (add_cap > node::capacity)
    //         {
    //             grow_tail();
    //             add_cap -= node::capacity;
    //         }
    //         grow_tail();
    //     }

    //     assert(capacity() >= nelem);
    // }

    void push_front(const T& elem)
    {
        if (_head == nullptr)
        {
            init();
        }
        else if (_head->full())
        {
            grow_head();
        }
        _head->push_front(elem);
        ++_size;
    }

    void push_back(const T& elem)
    {
        if (_head == nullptr)
        {
            init();
        }
        else if (_tail->full())
        {
            grow_tail();
        }
        _tail->push_back(elem);
        ++_size;
    }

    void insert(size_t, const T&);
    void erase(size_t);

    // TODO pop back/front/erase

    void pop_front()
    {
        _head->pop_front();
        if (_head->empty())
        {
            shrink_head();
        }
        --_size;
    }

    void pop_back()
    {
        _tail->pop_back();
        if (_tail->empty())
        {
            shrink_tail();
        }
        --_size;
    }

    T& front()
    {
        assert(_head != nullptr);

        return _head->front();
    }

    T& back()
    {
        assert(_tail != nullptr);

        return _tail->back();
        // if (!_tail->empty())
        // {
        //     return _tail->back();
        // }
        // else
        // {
        //     assert(_tail->prev != nullptr);

        //     return _tail->prev->back();
        // }
    }

    T& operator[](size_t idx)
    {
        node* curr;
        assert(idx < _size);
        // TODO choose closer (reverse or forward)
        if (idx < _size - idx)
        {
            curr = _head;

            assert(curr != nullptr);

            while (idx >= curr->size)
            {
                assert(curr != nullptr);

                idx -= curr->size;
                curr = curr->next;
            }
        }
        else
        {
            curr = _tail;

            assert(curr != nullptr);

            idx = _size - idx;
            while (idx > curr->size)
            {
                assert(curr != nullptr);

                idx -= curr->size;
                curr = curr->prev;
            }
            idx = curr->size - idx;
        }
        return (*curr)[idx];
    }

protected:

    void init()
    {
        _head = new node(nullptr, nullptr);
        //_avail = _head;
        _tail = _head;
    }
    
    void grow_head()
    {
        node* new_head = new node();
        new_head->prev = nullptr;
        link(new_head, _head);
        _head = new_head;
    }

    void grow_tail()
    {
        node* new_tail = new node();
        new_tail->next = nullptr;
        link(_tail, new_tail);
        _tail = new_tail;
    }

    void shrink_head()
    {
        if (_head == _tail)
        {
            delete _head;
            _head = nullptr;
            _tail = nullptr;
            return;
        }
        _head = _head->next;
        delete _head->prev;
        _head->prev = nullptr;
    }

    void shrink_tail()
    {
        if (_head == _tail)
        {
            delete _head;
            _head = nullptr;
            _tail = nullptr;
            return;
        }
        _tail = _tail->prev;
        delete _tail->prev;
        _tail->next = nullptr;
    }

    static void link(node*& first, node*& next)
    {
        first->next = next;
        next->prev = first;
    }
private:

    void destroy()
    {
        node* curr = _head;
        while (curr != nullptr)
        {
            node* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }

    /*node* alloc_back()
    {
        node* curr = _head;
        while (curr != nullptr)
        {
            curr = curr->next;
        }
        return curr;
    }*/

    size_t count_nodes() const
    {
        size_t i = 0;
        node* curr = _head;
        while (curr != nullptr)
        {
            curr = curr->next;
            ++i;
        }
        return i;
    }

    node* _head;
    //node* _avail; // next available node (backmosst.
    node* _tail;
    size_t _size;
};

#undef LU_LIST_NODE_SIZE

template <typename T>
using list = unrolled_list<T>;
}

#endif // LU_LIST_H
