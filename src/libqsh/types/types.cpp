/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file types.cpp
 * \date Nov 13, 2016
 */

#include "qsh/types/types.hpp"
#include "qsh/alloc/allocator.hpp"
#include "qsh/alloc/arena.hpp"

#include <unordered_map>

namespace qsh {

struct type_key
{
    type::kind_type kind;
    types_range     types;
};

bool operator==(type_key const& a, type_key const& b)
{
    return a.kind == b.kind &&
           a.types.size() == b.types.size() &&
           std::equal(a.types.begin(), a.types.end(), b.types.begin());
}

size_t hash_range(void const* b, size_t sz, size_t seed = 0)
{
    size_t const prime = 17;
    size_t h = seed;
    size_t mul = prime;
    auto bytes = static_cast<uint8_t const*>(b);
    for (size_t i = 0; i < sz; ++i) {
        h += bytes[i]*mul;
        mul *= prime;
    }
    return h;
}

struct hash_type_key
{
    size_t operator()(type_key const& x) const
    {
        auto h = hash_range(&x, sizeof(type::kind_type));
        return hash_range(x.types.begin(), x.types.size()*sizeof(type*), h);
    }
};

struct type_factory::impl
{
    using type_map = std::unordered_map<
                        type_key,
                        type const*,
                        hash_type_key,
                        std::equal_to<type_key>,
                        alloc<std::pair<type_key const, type const*>>>;

    arena m_arena;
    type_map m_map;
};

size_t type_size(size_t ncomps)
{
    return sizeof(type) + ncomps*sizeof(void*);
}

type_factory::type_factory() : m_impl()
{
    static_assert(sizeof(impl) <= IMPL_SIZE, "type_factory pimpl size too small!");
    m_impl.reset();
}

type_factory::~type_factory()
{
}

type const* type_factory::get(type::kind_type k, types_range ts)
{
    auto& map = m_impl->m_map;
    auto it = map.find({k, ts});
    if (it != map.end()) return it->second;
    assert(ts.size() == 0 || k == type::TUPLE);
    auto ntypes = ts.size();
    auto sz = type_size(ntypes);
    auto p = reinterpret_cast<type*>(m_impl->m_arena.allocate(sz));
    p->m_kind = k;
    p->m_num_types = ntypes;
    p->m_impl = nullptr;
    auto t = p->types_begin();
    for (size_t i = 0; i < ntypes; ++i) {
        t[i] = ts[i];
    }
    map[{k, ts}] = p;
    return p;
}

} // namespace qsh
