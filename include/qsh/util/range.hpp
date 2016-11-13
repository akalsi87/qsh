/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file range.hpp
 * \date Nov 13, 2016
 */

#ifndef _qsh_util_range_hpp_
#define _qsh_util_range_hpp_

#include <type_traits>
#include <utility>

namespace qsh {

template <class T>
class pointer_range
{
  public:
    pointer_range(T* begin, T* end) : m_begin(begin), m_end(end)
    { }

    pointer_range(T* begin, size_t n) : m_begin(begin), m_end(begin+n)
    { }

    T begin() const
    {
        return m_begin;
    }

    T end() const
    {
        return m_end;
    }

    size_t size() const
    {
        return static_cast<size_t>(m_end-m_begin);
    }
  private:
    mutable T* m_begin;
    mutable T* m_end;
};

template <class T>
using range = pointer_range<T>;

template <class T>
using const_range = pointer_range<const T>;

} // namespace qsh

#endif/*_qsh_util_range_hpp_*/
