/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file stack_pimpl.hpp
 * \date Nov 12, 2016
 */

#ifndef _qsh_util_stack_pimpl_hpp_
#define _qsh_util_stack_pimpl_hpp_

#include "qsh/config.h"
#include "qsh/util/stack_storage.hpp"
#include "qsh/util/noncopyable.hpp"

#include <utility>

namespace qsh {

template <class impl, int sz>
class stack_pimpl : noncopyable
{
  public:
    stack_pimpl() : m_data(), m_init(false)
    { }

    template <class... Ts>
    stack_pimpl(Ts... args) : m_data(), m_init(false)
    {
        reset(std::forward<Ts>(args)...);
    }

    stack_pimpl(stack_pimpl&& rhs) QSH_NOEXCEPT : m_data(), m_init(false)
    {
        std::swap(rhs.m_data, m_data);
        std::swap(rhs.m_init, m_init);
    }

    ~stack_pimpl()
    {
        destroy();
    }

    stack_pimpl& operator=(stack_pimpl&& rhs) QSH_NOEXCEPT
    {
        if (this != &rhs) {
            std::swap(rhs.m_data, m_data);
            std::swap(rhs.m_init, m_init);
        }
        return *this;
    }

    template <class... Ts>
    void reset(Ts... args)
    {
        if (m_init) destroy();
        new (&(m_data.data)) impl(std::forward<Ts>(args)...);
        m_init = true;
    }

    void destroy()
    {
        auto p = get();
        if (p) p->~impl();
    }

    impl* get()
    {
        return m_init ? reinterpret_cast<impl*>(&m_data.data) : 0;
    }

    const impl* get() const
    {
        return m_init ? reinterpret_cast<const impl*>(&m_data.data) : 0;
    }

    impl* operator->()
    {
        return get();
    }

    const impl* operator->() const
    {
        return get();
    }
  private:
    using storage = stack_storage<sz>;
    storage m_data;
    bool m_init;
};

} // namespace qsh

#endif/*_qsh_util_stack_pimpl_hpp_*/
