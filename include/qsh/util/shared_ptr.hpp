/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file shared_ptr.hpp
 * \date Nov 12, 2016
 */

#ifndef _qsh_util_shared_ptr_hpp_
#define _qsh_util_shared_ptr_hpp_

#include "qsh/config.h"

#include <new>
#include <utility>
#include <cstdlib>

namespace qsh {

template <class type>
class shared_ptr
{
  public:
    template <class... Ts>
    static shared_ptr create(Ts... args)
    {
        auto p = allocate(sizeof(size_t) + sizeof(type));
        if (!p) throw std::bad_alloc();
        new (p) size_t(1);
        auto pt = new (static_cast<char*>(p) + sizeof(size_t)) type(std::forward<Ts>(args)...);
        return shared_ptr(pt);
    }

    shared_ptr() QSH_NOEXCEPT : m_ptr()
    { }

    shared_ptr(const shared_ptr& rhs) QSH_NOEXCEPT : m_ptr(rhs.m_ptr)
    {
        inc_ref();
    }

    shared_ptr(shared_ptr&& rhs) QSH_NOEXCEPT : m_ptr()
    {
        std::swap(m_ptr, rhs.m_ptr);
    }

    shared_ptr& operator=(const shared_ptr& rhs)
    {
        if (this != &rhs) {
            dec_ref();
            m_ptr = rhs.m_ptr;
            inc_ref();
        }
        return *this;
    }

    shared_ptr& operator=(shared_ptr&& rhs) QSH_NOEXCEPT
    {
        if (this != &rhs) {
            std::swap(m_ptr, rhs.m_ptr);
        }
        return *this;
    }

    ~shared_ptr() QSH_NOEXCEPT
    {
        dec_ref();
    }

    operator bool() const
    {
        return m_ptr ? true : false;
    }

    type* get()
    {
        return m_ptr;
    }

    const type* get() const
    {
        return m_ptr;
    }

    type* operator->()
    {
        return m_ptr;
    }

    const type* operator->() const
    {
        return m_ptr;
    }

    type& operator*()
    {
        return *m_ptr;
    }

    const type& operator*() const
    {
        return *m_ptr;
    }

    size_t use_count() const
    {
        return m_ptr ? (*refs()) : 0;
    }

    void reset()
    {
        dec_ref();
        m_ptr = 0;
    }

  private:
    type* m_ptr;

    static void* allocate(size_t sz)
    {
        return std::malloc(sz);
    }

    static void deallocate(void* p)
    {
        std::free(p);
    }

    shared_ptr(type* ptr) : m_ptr(ptr)
    { }

    size_t* refs()
    {
        return reinterpret_cast<size_t*>(m_ptr)-1;
    }

    const size_t* refs() const
    {
        return reinterpret_cast<size_t*>(m_ptr)-1;
    }

    void inc_ref()
    {
        if (m_ptr) {
            auto prefs = refs();
            ++(*prefs);
        }
    }

    void dec_ref()
    {
        if (m_ptr) {
            auto prefs = refs();
            if (--(*prefs) == 0) {
                m_ptr->~type();
                deallocate(prefs);
            }
        }
    }
};

} // namespace qsh

#endif/*_qsh_util_shared_ptr_hpp_*/
