/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file allocator.cpp
 * \date Nov 13, 2016
 */

#include "qsh/alloc/allocator.hpp"

#include <cassert>
#include <cstdlib>

namespace qsh {

#define SIZE_T_MAGIC 0xBADBEEF

void* allocate(size_t sz)
{
    sz = sizeof(size_t)*((sz+sizeof(size_t)-1)/(sizeof(size_t)));
#ifndef NDEBUG
    sz += 3*sizeof(size_t);
#endif
    size_t* p = reinterpret_cast<size_t*>(std::malloc(sz));
    if (!p) return 0;
#ifndef NDEBUG
    sz -= 3*sizeof(size_t);
    p[0] = SIZE_T_MAGIC;
    p[1] = sz;
    p[2+(sz/sizeof(size_t))] = SIZE_T_MAGIC;
    p += 2;
#endif
    return p;
}

void deallocate(void* ptr)
{
    if (!ptr) return;
    size_t* p = reinterpret_cast<size_t*>(ptr);
#ifndef NDEBUG
    p -= 2;
    assert(p[0] == SIZE_T_MAGIC);
    auto sz = p[1];
    assert(p[2+(sz/sizeof(size_t))] == SIZE_T_MAGIC);
#endif
    std::free(p);
}

} // namespace qsh
