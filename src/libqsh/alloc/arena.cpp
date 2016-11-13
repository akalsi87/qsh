/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file arena.cpp
 * \date Nov 13, 2016
 */

#include "qsh/alloc/arena.hpp"
#include "qsh/alloc/allocator.hpp"

namespace qsh {

struct header
{
    size_t sz;

    header(size_t s, bool dtor = false) : sz((s & ~size_t(1)) | (dtor ? 1 : 0))
    {
    }

    size_t size() const
    {
        return sz & ~size_t(1);
    }

    bool has_dtor() const
    {
        return sz & size_t(1);
    }

    char* data()
    {
        auto ptr = reinterpret_cast<char*>(this);
        return ptr + sizeof(header) + (has_dtor() ? sizeof(void*) : 0);
    }

    header* next()
    {
        return reinterpret_cast<header*>(data() + size());
    }
};

struct dtor_header : public header
{
    void* ptr;

    dtor_header(size_t s) : header(s, true)
    {
    }

    void destroy_data()
    {
        typedef void (*dtor_fn)(void*);
        reinterpret_cast<dtor_fn>(ptr)(this->data());
    }
};

struct segment
{
    segment* fwd;
    size_t sz;
    size_t left;
    char* curr;

    segment(size_t s) : fwd(nullptr), sz(s), left(s), curr(nullptr)
    {
        curr = reinterpret_cast<char*>(const_cast<segment*>(this) + 1);
    }

    header* begin()
    {
        return reinterpret_cast<header*>(const_cast<segment*>(this) + 1);
    }

    header* end()
    {
        return reinterpret_cast<header*>(curr);
    }

    void call_dtors()
    {
        header* f = begin();
        header* const e = end();
        while (f != e) {
            if (f->has_dtor()) {
                reinterpret_cast<dtor_header*>(f)->destroy_data();
            }
            f = f->next();
        }
    }
};

segment* new_segment(size_t sz)
{
    static const size_t ALIGN = 15;
    // align to nearest multiple of 16
    sz = (sz + ALIGN) & ~ALIGN;
    sz += sizeof(segment);
    void* mem = qsh::allocate(sz + sizeof(segment));
    return new (mem) segment(sz);
}

void destroy_segments(segment* seg)
{
    while (seg) {
        segment* next = seg->fwd;
        seg->call_dtors();
        qsh::deallocate(seg);
        seg = next;
    }
}

void* allocate_from_seg(segment* seg, size_t sz, void* ptr)
{
    header* h = nullptr;
    if (ptr) {
        dtor_header* hdr = new (seg->end()) dtor_header(sz - sizeof(dtor_header));
        hdr->ptr = ptr;
        h = hdr;
    } else {
        header* hdr = new (seg->end()) header(sz - sizeof(header));
        h = hdr;
    }
    seg->curr = reinterpret_cast<char*>(h->next());
    seg->left -= sz;
    return h->data();
}

struct arena::impl
{
    segment* seg_;
    size_t minsz_;

    using destroy_fn = arena::destroyfn;

    impl(size_t sz) : seg_(new_segment(sz)), minsz_(sz)
    {
    }

    ~impl()
    {
        if (seg_) {
            destroy_segments(seg_);
        }
    }

    void* allocate(size_t sz, void* fptr)
    {
        if (seg_->left > sz) {
            return allocate_from_seg(seg_, sz, fptr);
        } else {
            size_t segsz = sz > minsz_ ? sz : minsz_;
            segment* s = new_segment(segsz);
            s->fwd = seg_;
            seg_ = s;
            return allocate_from_seg(s, sz, fptr);
        }
    }
};

arena::arena(size_t sz) : impl_(sz)
{
    static_assert(sizeof(impl) <= IMPL_SIZE, "Stack pimpl for arena too small!");
}

arena::~arena()
{
}

void* arena::allocate(size_t sz, arena::destroyfn fn)
{
    sz = (sz + 7) & ~size_t(7); // align upto 8
    size_t szneeded = sz + sizeof(header) + (fn ? sizeof(void*) : 0);
    return impl_->allocate(szneeded, (void*)fn);
}

} // namespace qsh
