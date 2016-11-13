/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file parse_tree.hpp
 * \date Nov 12, 2016
 */

#ifndef _qsh_parser_parse_tree_hpp_
#define _qsh_parser_parse_tree_hpp_

#include "qsh/util/shared_ptr.hpp"
#include "qsh/util/noncopyable.hpp"

namespace qsh {

enum node_type
{
    tINT,
    tFLOAT,
    tCHAR,
    tSTRING
};

enum node_array
{
    aNO,
    aYES
};

typedef union
{
    long    vlong;
    double  vfloat;
    char    vchar;
    char*   vstring;
    long*   vlongarr;
    double* vfloatarr;
    char**  vstringarr;
} node_fields;

struct default_allocator : noncopyable
{
    void* allocate(size_t sz)
    {
        return std::malloc(sz);
    }

    void deallocate(void* p)
    {
        std::free(p);
    }
};

class node_allocator : noncopyable
{
    typedef void* (*alloc_fn)(void*, size_t);
    typedef void (*dealloc_fn)(void*);
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
        std::free(s);
    }

  public:
    template <class actual, class... Ts>
    node_allocator(Ts... args)
    {
        void* p = std::malloc(sizeof(actual));
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

struct node_data
{
    node_type   type :  2;
    node_array  array:  1;
    uint64_t    len  : 61;
    node_fields data;
};

} // namespace qsh

#endif/*_qsh_parser_parse_tree_hpp_*/
