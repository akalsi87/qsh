/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file types.hpp
 * \date Nov 13, 2016
 */

#ifndef _qsh_types_types_hpp_
#define _qsh_types_types_hpp_

#include "qsh/alloc/arena.hpp"
#include "qsh/util/noncopyable.hpp"

#include <cassert>

namespace qsh {

using value_ptr = void*;
using const_value_ptr = void const*;

class type_factory;
class type;
class type_impl;

struct type_impl : noncopyable
{
  public:
    void copy(value_ptr dst, const_value_ptr src) const
    {
        assert(m_copy);
        if (dst != src) m_copy(dst, src);
    }

    void destroy(value_ptr dst) const
    {
        m_destroy ? m_destroy(dst) : 0;
    }

    size_t size(const_value_ptr dst) const
    {
        m_size ? m_size(dst) : 1;
    }

    bool equal(const_value_ptr a, const_value_ptr b)
    {
        assert(m_equal);
        return a == b || m_equal(a, b);
    }

    bool less(const_value_ptr a, const_value_ptr b)
    {
        assert(m_less);
        return a == b ? false :  m_less(a, b);
    }
  private:
    typedef int (*copy_fn)(value_ptr dst, const_value_ptr src);
    typedef int (*destroy_fn)(value_ptr dst);
    typedef size_t (*size_fn)(const_value_ptr dst);
    typedef bool (*equal_fn)(const_value_ptr a, const_value_ptr b);
    typedef bool (*less_fn)(const_value_ptr a, const_value_ptr b);

    copy_fn m_copy;
    destroy_fn m_destroy;
    size_fn m_size;
    equal_fn m_equal;
    less_fn m_less;

    type_impl() = default;
    friend class type;
};

class type : noncopyable
{
  public:
    enum kind_type
    {
        INT,
        FLOAT,
        CHAR,
        STRING,
        INT_ARR,
        FLOAT_ARR,
        STRING_ARR,
        TUPLE,
        UNKNOWN
    };

    kind_type kind() const
    {
        return m_kind;
    }

    bool is_unknown() const
    {
        return m_kind == UNKNOWN;
    }

    bool is_tuple() const
    {
        return m_kind == TUPLE;
    }

    size_t num_components() const
    {
        return m_num_types;
    }

    const type* get_type(size_t i) const
    {
        auto comp = reinterpret_cast<const type* const*>(this+1);
        return comp[i];
    }

    // range<const type*> types_range() const
    // {
    //     auto comp = reinterpret_cast<const type* const*>(this+1);
    //     return range(comp, comp + m_num_types);
    // }
  private:
    kind_type m_kind : 4;
    uint64_t m_num_types : 60;
    type_impl* m_impl;

    type() = default;
    friend class type_factory;
};

class type_factory : noncopyable
{
  public:
    type_factory();
    ~type_factory();
    const type* get(type::kind_type k, size_t comps = 0, const type* ts);
  private:
    class impl;
    impl* m_impl;
};

} // namespace qsh

#endif/*_qsh_types_types_hpp_*/
