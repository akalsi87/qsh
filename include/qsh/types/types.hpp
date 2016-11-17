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
#include "qsh/util/range.hpp"

#include <cassert>
#include <vector>

namespace qsh {

using value_ptr = void*;
using const_value_ptr = void const*;

class type_factory;
class type;
class type_impl;

class arena;

struct QSH_API type_impl : noncopyable
{
  public:
    size_t type_size() const
    {
        return m_type_size;
    }

    void copy(value_ptr dst, const_value_ptr src) const
    {
        assert(m_copy);
        if (dst != src) m_copy(dst, src);
    }

    void destroy(value_ptr dst) const
    {
        m_destroy ? m_destroy(dst), 0 : 0;
    }

    size_t size(const_value_ptr dst) const
    {
        return m_size ? m_size(dst) : 1;
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
    typedef void (*copy_fn)(value_ptr dst, const_value_ptr src);
    typedef void (*destroy_fn)(value_ptr dst);
    typedef size_t (*size_fn)(const_value_ptr dst);
    typedef bool (*equal_fn)(const_value_ptr a, const_value_ptr b);
    typedef bool (*less_fn)(const_value_ptr a, const_value_ptr b);

    size_t m_type_size;
    copy_fn m_copy;
    destroy_fn m_destroy;
    size_fn m_size;
    equal_fn m_equal;
    less_fn m_less;

    type_impl() = default;
    friend class type;
};

using types_range = const_range<type const*>;

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

    using int_type = ssize_t;
    using float_type = double;
    using char_type = char;
    using string_type = std::vector<char>;

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

    types_range types() const
    {
        auto comp = reinterpret_cast<type const* const*>(this+1);
        return types_range(comp, comp + m_num_types);
    }

    type_impl const* impl() const
    {
        return m_impl;
    }
  private:
    kind_type m_kind : 4;
    uint64_t m_num_types : 60;
    type_impl* m_impl;

    type const** types_begin()
    {
        return reinterpret_cast<const type**>(this+1);
    }

    static
    type_impl* create_type_impl(arena& ar, type::kind_type k);

    type() = default;
    friend class type_factory;
};

class QSH_API type_factory : noncopyable
{
  public:
    type_factory();
    ~type_factory();
    const type* get(type::kind_type k, types_range ts = types_range());
  private:
    class impl;
    static const int IMPL_SIZE = 96;
    stack_pimpl<impl, IMPL_SIZE> m_impl;
};

} // namespace qsh

#endif/*_qsh_types_types_hpp_*/
