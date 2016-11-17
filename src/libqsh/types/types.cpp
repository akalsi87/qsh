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
#include <vector>

namespace qsh {

namespace {

template <class T, bool is_array>
struct value_type_picker;

template <class T>
struct value_type_picker<T, false>
{
    using value_type = T;
};

template <class T>
struct value_type_picker<T, true>
{
    using value_type = std::vector<T>;
};

template <class value_t, bool is_array>
class simple_type_impl
{
    using value_type = typename value_type_picker<value_t, is_array>::value_type;

    static
    value_type* pvalue(value_ptr x)
    {
        return reinterpret_cast<value_type*>(x);
    }

    static
    value_type const* pvalue(const_value_ptr x)
    {
        return reinterpret_cast<value_type const*>(x);
    }
  public:
    static const size_t type_size = sizeof(value_type);

    static
    void copy(value_ptr dst, const_value_ptr src)
    {
        *pvalue(dst) = *pvalue(src);
    }

    static
    void destroy(value_ptr dst)
    {
        pvalue(dst)->~value_type();
    }

    static
    size_t size(const_value_ptr dst, int _=int())
    {
        using fake = typename std::enable_if<is_array, size_t>::type;
        static_cast<void>(fake());
        return pvalue(dst)->size();
    }

    static
    size_t size(const_value_ptr)
    {
        return 1;
    }

    static
    bool equal(const_value_ptr a, const_value_ptr b)
    {
        return *pvalue(a) == *pvalue(b);
    }

    static
    bool less(const_value_ptr a, const_value_ptr b)
    {
        return *pvalue(a) < *pvalue(b);
    }
};

} // namespace

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

static inline
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

type_impl* type::create_type_impl(arena& a, type::kind_type k)
{
    auto timpl = nullptr;
    switch (k) {
#define CASE_(ty, cty, arr) \
    case (ty):    \
    {             \
        timpl = reinterpret_cast<type_impl*>(a.allocate(sizeof(type_impl))); \
        using impl = simple_type_impl<cty, arr>; \
        timpl->m_type_size = impl::type_size; \
        timpl->m_copy = impl::copy; \
        timpl->m_destroy = impl::destroy; \
        timpl->m_size = impl::size; \
        timpl->m_equal = impl::equal; \
        timpl->m_less = impl::less; \
    } \
    break
    CASE_(type::INT, type::int_type, false);
    CASE_(type::FLOAT, type::float_type, false);
    CASE_(type::CHAR, type::char_type, false);
    CASE_(type::STRING, type::string_type, false);
    CASE_(type::INT_ARR, type::int_type, true);
    CASE_(type::FLOAT_ARR, type::float_type, true);
    CASE_(type::STRING_ARR, type::string_type, true);
    default: break;
    }
    return timpl;
}

type const* type_factory::get(type::kind_type k, types_range ts)
{
    auto& map = m_impl->m_map;
    auto it = map.find({k, ts});
    if (it != map.end()) return it->second;

    // not found, create and memoize
    assert(ts.size() == 0 || k == type::TUPLE);
    auto ntypes = ts.size();
    auto sz = type_size(ntypes);
    auto p = reinterpret_cast<type*>(m_impl->m_arena.allocate(sz));
    p->m_kind = k;
    p->m_num_types = ntypes;
    p->m_impl = type::create_type_impl(m_impl->m_arena, k);
    auto t = p->types_begin();
    for (size_t i = 0; i < ntypes; ++i) {
        t[i] = ts[i];
    }
    map[{k, ts}] = p;

    // return!
    return p;
}

} // namespace qsh
