/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file allocator.hpp
 * \date Nov 13, 2016
 */

#ifndef _qsh_alloc_allocator_hpp_
#define _qsh_alloc_allocator_hpp_

#include "qsh/config.h"
#include "qsh/util/noncopyable.hpp"

#include <new>
#include <utility>

namespace qsh {

QSH_API
void* allocate(size_t sz);

QSH_API
void deallocate(void* p);

struct default_allocator : noncopyable
{
    void* allocate(size_t sz)
    {
        return qsh::allocate(sz);
    }

    void deallocate(void* p)
    {
        qsh::deallocate(p);
    }
};

class node_allocator : noncopyable
{
    typedef void* (*alloc_fn)(void*, size_t);
    typedef void (*dealloc_fn)(void*, void*);
    typedef void (*destroy_fn)(void*);

    void* m_state;
    alloc_fn m_alloc;
    dealloc_fn m_dealloc;
    destroy_fn m_destroy;

    template <class T>
    static void* alloc(void* s, size_t sz)
    {
        return static_cast<T*>(s)->allocate(sz);
    }

    template <class T>
    static void dealloc(void* s, void* p)
    {
        return static_cast<T*>(s)->deallocate(p);
    }

    template <class T>
    static void destroy(void* s)
    {
        static_cast<T*>(s)->~T();
        qsh::deallocate(s);
    }

  public:
    template <class actual, class... Ts>
    node_allocator(Ts... args)
    {
        void* p = qsh::allocate(sizeof(actual));
        if (!p) throw std::bad_alloc();
        m_state = new (p) actual(std::forward<Ts>(args)...);
        m_alloc = alloc<actual>;
        m_dealloc = dealloc<actual>;
        m_destroy = destroy<actual>;
    }

    ~node_allocator()
    {
        m_destroy(m_state);
    }

    void* allocate(size_t s) const
    {
        return m_alloc(m_state, s);
    }

    void deallocate(void* p) const
    {
        m_dealloc(m_state, p);
    }
};

} // namespace qsh

#endif/*_qsh_alloc_allocator_hpp_*/
