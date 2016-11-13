/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file arena.hpp
 * \date Nov 13, 2016
 */

#ifndef _qsh_alloc_arena_hpp_
#define _qsh_alloc_arena_hpp_

#include "qsh/config.h"
#include "qsh/util/noncopyable.hpp"

#include <cstring>
#include <new>
#include <type_traits>
#include <utility>

namespace qsh {

class QSH_API arena : noncopyable
{
  public:
    arena(size_t sz = 1024);

    ~arena();

    const char* create_string(const char* str)
    {
        return create_string(str, std::strlen(str));
    }

    const char* create_string(const char* str, size_t len)
    {
        auto arr = create_array<char>(len + 1);
        std::memcpy(arr, str, len);
        arr[len] = '\0';
        return arr;
    }

    template <class T, class... Args>
    T* create(Args&&... args)
    {
        const bool trivial_dtor = std::is_trivially_destructible<T>::value;
        void* mem = allocate(sizeof(T), (trivial_dtor ? nullptr : destroy_scalar<T>::act));
        return new (mem) T(std::forward<Args>(args)...);
    }

    template <class T>
    T* create_array(size_t n)
    {
        const bool trivial_dtor = std::is_trivially_destructible<T>::value;
        if (trivial_dtor) {
            return reinterpret_cast<T*>(allocate(n*sizeof(T), nullptr));
        }
        void* mem = allocate(sizeof(size_t)+(n*sizeof(T)), destroy_array<T>::act);
        auto plen = static_cast<size_t*>(mem);
        *plen++ = n;
        return reinterpret_cast<T*>(plen);
    }

  private:
    struct impl;

    template <class T>
    struct destroy_scalar
    {
        static void act(void* ptr)
        {
            static_cast<T*>(ptr)->~T();
        }
    };

    template <class T>
    struct destroy_array
    {
        static void act(void* ptr)
        {
            auto plen = static_cast<size_t*>(ptr);
            auto len = *plen++;
            T* arr = reinterpret_cast<T*>(plen);
            for (size_t i = 0; i < len; ++i) {
                arr[i].~T();
            }
        }
    };

    typedef void (*destroyfn)(void*);
    void* allocate(size_t sz, destroyfn fn);

    impl* impl_;
};

} // namespace qsh

#endif/*_qsh_alloc_arena_hpp_*/
