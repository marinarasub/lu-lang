#ifndef LU_AVL_H
#define LU_AVL_H

#include <cstdint>
#include <cstddef>
#include <iterator>
#include <cassert>
#include <algorithm>
#include <limits>
#include "../../utility.h"

namespace lu
{
namespace internal
{

template <typename T>
struct avl_tree
{
protected:
    struct node
    {
        node() = delete;
        
        node(const T& data, node* parent) : links{ nullptr, nullptr, parent }, height(1), data(data)
        {

        }

        ~node()
        {
            //destroy();
        }

        node(const node& other)
        {
            copy(other);
        }

        constexpr static size_t LEFT = 0;
        constexpr static size_t RIGHT = 1;
        constexpr static size_t PARENT = 2;

        bool leaf() const
        {
            return left() == nullptr && right() == nullptr;
        }

        node*& parent() { return links[PARENT]; }
        node*& left() { return links[LEFT]; }
        node*& right() { return links[RIGHT]; }

        node*& child(node* which)
        {
            assert(which == left() || which == right());

            return which == left() ? left() : right();
        }

        node*& operator[](size_t idx) { assert(idx < 2); return links[idx]; }

        // int8_t bf()
        // {
        //     return node_height(right()) - node_height(left());
        // }
        node* links[3];
        int8_t height;
        T data;

        void copy(const node& other)
        {
            this->links[LEFT] = new node(*other->links[LEFT]);
            this->links[RIGHT] = new node(*other->links[RIGHT]);
            this->links[PARENT] = new node(*other->links[PARENT]);
            this->data = other.data;
            this->height = other.height;
        }

        void clear()
        {
            //destroy();
            links = { nullptr, nullptr, nullptr };
        }
    };

public:

    static int8_t node_height(node* subtree)
    {
        if (subtree == nullptr)
        {
            return 0;
        }
        return subtree->height;
    }

    static void calc_node_height(node* subtree)
    {
        subtree->height = 1 + std::max(node_height(subtree->left()), node_height(subtree->right()));
    }

    // assume subtree has a left child
    static node* last(node* subtree)
    {
        //subtree = subtree->left();
        while ((subtree)->right() != nullptr)
        {
            subtree = (subtree)->right();
        }
        return subtree;
    }

    // assume subtree has right child
    static node* first(node* subtree) //(leftmost)
    {
        //subtree = subtree->right();
        while ((subtree)->left() != nullptr) 
        {
            subtree = (subtree)->left();
        }
        return subtree;
    }

    static node* next(node* subtree)
    {
        if (subtree->right() == nullptr)
        {
            while (subtree->parent() != nullptr && subtree->parent()->right() == subtree)
            {
                subtree = subtree->parent();
            }
            subtree = subtree->parent();
        }
        else
        {
            subtree = first(subtree->right());
        }
        return subtree;
    }

    static node* prev(node* subtree)
    {
        if (subtree->left() == nullptr)
        {
            while (subtree->parent() != nullptr && subtree->parent()->left() == subtree)
            {
                subtree = subtree->parent();
            }
            subtree = subtree->parent();
        }
        else
        {
            subtree = last(subtree->left());
        }
        return subtree;
    }

    struct const_iterator
    {
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const T*;
        using reference         = const T&;

        friend struct avl_tree;

        const_iterator(node* curr) : _curr(curr), _prev(curr == nullptr ? curr : prev(curr)) {}
        const_iterator(const const_iterator& other) : _curr(other._curr), _prev(other._prev) {}

        bool operator==(const const_iterator& other) { return this->_curr == other._curr; }
        bool operator!=(const const_iterator& other) { return this->_curr != other._curr; }

        const_iterator& operator++() { next(); return *this; }
        const_iterator operator++(int) { const_iterator ret(*this); get_next(); return ret; }

        const_iterator& operator--() { prev(); return *this; }
        const_iterator operator--(int) { const_iterator ret(*this); get_prev(); return ret; }

        const T& operator*() { return _curr->data; }

    protected:
        const_iterator(node* curr, node* prev) : _curr(curr), _prev(prev) {}

    private:
        const_iterator& get_next()
        {
            _prev = _curr;
            _curr = next(_curr);
        }

        const_iterator& get_prev()
        {
            _prev = prev(_prev);
            _curr = _prev;
        }

        node* _curr;
        node* _prev;
    };

    using iterator = const_iterator; // TODO allow non const iterator ie map needs to modify value but not key?

    avl_tree();
    ~avl_tree();

    avl_tree(avl_tree&&);
    avl_tree(const avl_tree&);
    avl_tree& operator=(avl_tree&&);
    avl_tree& operator=(const avl_tree&);
    
    size_t size() const { return _size; }
    bool empty() const { return _size == 0; };
    size_t height() const;

    void clear();

    const_iterator insert(const T&);
    bool remove(const T&);
    const_iterator find(const T&) const;

    const T& top()
    {
        return _root->data;
    }

    const T& front();
    const T& back();

    const_iterator begin() const { return const_iterator(first(_root), nullptr); }
    const_iterator end() const { return const_iterator(nullptr, last(_root)); }
private:

    void destroy();
    void copy(const avl_tree&);
    void move(avl_tree&&);

    node* find_node(node* subtree, const T& key) const
    {
        if (subtree == nullptr)
        {
            return nullptr;
        }
        else if (subtree->data == key)
        {
            return subtree;
        }
        else
        {
            size_t link = size_t(subtree->data < key);
            return find_node(subtree->links[link], key);
        }
    }

    node* insert_node(node* parent, size_t link, const T& key)
    {
        if (parent->links[link] == nullptr)
        {
            ++_size;
            return (parent->links[link] = new node(key, parent));
        }
        else
        {
            size_t link2 = size_t(parent->data < key);
            node* ret = insert_node(parent->links[link], link2, key);
            rebalance_node(parent);
            return ret;
        }
    }

    node* insert_node_root(const T& key)
    {
        if (_root == nullptr)
        {
            ++_size;
            return (_root = new node(key, nullptr));
        }
        size_t link = size_t(_root->data < key);
        node* ret = insert_node(_root, link, key);
        rebalance_node(_root);
        return ret;
    }

    node* extract_predecessor(node* subtree)
    {
        assert(subtree->right() == nullptr);

        bool lnull = subtree->left() == nullptr;
        if (lnull)
        {
            subtree->parent()->child(subtree) = nullptr;
            return subtree;
        }
        else
        {
            node* replace = subtree->left();
            if (subtree->parent() != nullptr)
            {
                subtree->parent()->child(subtree) = replace;
            }
            replace->parent() = subtree->parent();

            //rebalance_node_predecessor(subtree);
            rebalance_node(replace); // TODO for replace (also for regular remove replace), only need minus hiehgt instaed of recalcuate after rotate
            return subtree;
        }
    }

    // inplacr remove - current node will not be deleted unless it is leaf (isntead data is moved into current node)
    node* extract_node(node* subtree, node** root)
    {
        bool lnull = subtree->left() == nullptr;
        bool rnull = subtree->right() == nullptr;
        if (lnull && rnull) // leaf
        {
            subtree->parent()->child(subtree) = nullptr;
            return subtree;
        }
        else
        {
            node* replace;
            if (lnull || rnull) // replace with non-null child
            {
                
                //subtree = (rnull) ? subtree->left() : subtree->right();
                if (rnull)
                {
                    replace = subtree->left();
                }
                else
                {
                    replace = subtree->right();
                }

                if (subtree->parent() != nullptr)
                {
                    subtree->parent()->child(subtree) = replace;
                }
                replace->parent() = subtree->parent();
            }
            else // replace with predecessor
            {
                replace = subtree;
                node* subtree = last(subtree->left());
                T new_key = lu::move(subtree->data);
                {
                    // remove the predecessor (right should be null)
                    extract_predecessor(subtree);
                }
                replace->data = lu::move(new_key);
            }

            // rebalance up the tree
            while (replace != nullptr)
            {
                *root = rebalance_node(replace);
                replace = (*root)->parent;
            }
            
            return subtree;
        }
    }

    const_iterator erase_(const T& key)
    {
        const_iterator it = find(key);
        if (it == end())
        {
            return it;
        }
        else if (subtree->data == key)
        {
            node* rmvd = extract_node(subtree);
            delete rmvd;
            --_size;
            return rmvd;
        }
        else
        {
           
        }
    }

    //   a               b     *
    //    \            /  \    *
    //     b    ->    a    c   *
    //   /  \          \       *
    //  d    c          d      *
    static void rotate_left(node* subroot)
    {
        node* new_subroot = subroot->right();
        subroot->right() = new_subroot->left();
        
        if (new_subroot->left() != nullptr)
        {
            new_subroot->left()->parent() = subroot;
        }
        new_subroot->left() = subroot;

        if (subroot->parent() != nullptr)
        {
            subroot->parent()->child(subroot) = new_subroot;
        }
        new_subroot->parent() = subroot->parent();
        subroot->parent() = new_subroot;

        calc_node_height(new_subroot->left());
        calc_node_height(new_subroot);
    }

    //      a             b    *
    //     /            /  \   *
    //    b     ->    c     a  *
    //   / \               /   *
    //  c   d             d    *
    static void rotate_right(node* subroot)
    {
        
        node* new_subroot = subroot->left();
        subroot->left() = new_subroot->right();
        
        if (new_subroot->right() != nullptr)
        {
            new_subroot->right()->parent() = subroot;
        }
        new_subroot->right() = subroot;

        if (subroot->parent() != nullptr)
        {
            subroot->parent()->child(subroot) = new_subroot;
        }
        new_subroot->parent() = subroot->parent();
        subroot->parent() = new_subroot;

        calc_node_height(new_subroot->right());
        calc_node_height(new_subroot);
    }
    
    //     a        a             c    *
    //   /         /            /  \   *
    //  b    ->   c     ->    b     a  *
    //   \       /                     *
    //    c     b                      *
    static void rotate_leftright(node* subroot)
    {
        rotate_left(subroot->left());
        rotate_right(subroot);
    }

    //  a        a               c    *
    //   \        \            /  \   *
    //    b   ->   c    ->    a    b  *
    //   /          \                 *
    //  c            b                *
    static void rotate_rightleft(node* subroot)
    {
        rotate_right(subroot->right());
        rotate_left(subroot);
    }

    void rebalance_node_l(node* node)
    {
        int8_t llh = node_height(node->left()->left());
        int8_t lrh = node_height(node->left()->right());
        if (llh >= lrh) // LL -> rotate R // NOTE: ge so we do 1 less rotation if either would work.
        {
            rotate_right(node);
        }
        else // LR -> rotate LR
        {
            rotate_leftright(node);
        }
    }

    void rebalance_node_r(node* node)
    {
        int8_t rlh = node_height(node->right()->left());
        int8_t rrh = node_height(node->right()->right());
        if (rlh <= rrh) // RR -> rotate L 
        {
            rotate_left(node);
        }
        else // RL -> rotate RL
        {
            rotate_rightleft(node);
        }
    }

    // void rebalance_node_predecessor(node* node)
    // {
    //     assert(node_height(node->right()) == 0);
    //     assert(node_height(node->left()) <= 2);
    //     // r height is 0, so l height is bf
    //     if (node_height(node->left()) == 2)
    //     {
    //         rebalance_node_l(node);
    //     }
    //     else
    //     {
    //         calc_node_height(node);
    //     }
    // }

    void rebalance_node(node* node)
    {
        int8_t lh = node_height(node->left());
        int8_t rh = node_height(node->right());
        int8_t bf = rh - lh;

        assert(-2 <= bf && bf <= 2);

        // LEFT heavy
        if (bf == -2) // assert left is bf 1?
        {
            rebalance_node_l(node);
        }
        // RIGHT heavy
        else if (bf == 2)
        {
            rebalance_node_r(node);
        }
        else
        {
            calc_node_height(node);
        }
    }

    void destroy_node(node* subtree)
    {
        if (subtree == nullptr)
        {
            return;
        }
        destroy_node(subtree->left());
        destroy_node(subtree->right());
        delete subtree;
    }

    void inc_size()
    {
        assert(_size < std::numeric_limits<size_t>::max());

        ++_size;
    }

    void dec_size()
    {
        assert(_size > 0);

        --_size;
    }


    // FIELDS

    node* _root;
    size_t _size;
};

template <typename T>
avl_tree<T>::avl_tree() : _root(nullptr), _size(0) {}

template <typename T>
avl_tree<T>::~avl_tree() 
{
    destroy();
}

template <typename T>
avl_tree<T>::avl_tree(avl_tree<T>&& other)
{
    move(lu::move(other));
}

template <typename T>
avl_tree<T>::avl_tree(const avl_tree<T>& other)
{
    copy(other);
}

template <typename T>
avl_tree<T>& avl_tree<T>::operator=(avl_tree<T>&& other)
{
    move(lu::move(other));
}

template <typename T>
avl_tree<T>& avl_tree<T>::operator=(const avl_tree<T>& other)
{
    copy(other);
}

template <typename T>
size_t avl_tree<T>::height() const
{
    return static_cast<size_t>(_root->height);
}

template <typename T>
void avl_tree<T>::copy(const avl_tree<T>& other)
{
    this->_root = new node(*other._root);
}

template <typename T>
void avl_tree<T>::move(avl_tree<T>&& other)
{
    this->_root = other._root;
    other._root = nullptr;
}

template <typename T>
void avl_tree<T>::clear()
{
    destroy();
    _root = nullptr;
}

template <typename T>
void avl_tree<T>::destroy()
{
    destroy_node(_root);
}

template <typename T>
typename avl_tree<T>::const_iterator avl_tree<T>::insert(const T& key)
{
    node* temp = insert_node_root(key);
    return avl_tree<T>::const_iterator(temp);
}

// TODO use iterator to remove all
template <typename T>
bool avl_tree<T>::remove(const T& key)
{
    return remove_node_root(_root, key) != nullptr;
}

template <typename T>
typename avl_tree<T>::const_iterator avl_tree<T>::find(const T& key) const
{
    node* temp = find_node(_root, key);
    return avl_tree<T>::const_iterator(temp);
}


}
}


#endif // LU_AVL_H
