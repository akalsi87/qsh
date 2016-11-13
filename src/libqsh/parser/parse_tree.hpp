/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file parse_tree.hpp
 * \date Nov 12, 2016
 */

#ifndef _qsh_parser_parse_tree_hpp_
#define _qsh_parser_parse_tree_hpp_

#include "qsh/alloc/allocator.hpp"
#include "qsh/util/shared_ptr.hpp"
#include "qsh/util/noncopyable.hpp"

namespace qsh {

namespace {

template <class T>
T* allocate(size_t n)
{
    auto p = reinterpret_cast<T*>(qsh::allocate(sizeof(T)*n));
    if (!p) throw std::bad_alloc();
    return p;
}

template <class T>
void fill(T* p, size_t n)
{
    if (!std::is_trivial<T>::value) {
        for (size_t i = 0; i < n; ++i) {
            new (p[i]) T;
        }
    }
}

void deallocate(void* a)
{
    qsh::deallocate(a);
}

struct string_data
{
    char* m_data;
    size_t m_len;
};

} // namespace

class string : public string_data
{
  public:
    string(size_t n = 0) : m_data(n ? allocate<char>(n) : 0), m_len(n)
    { }

    string(const char* s, int64_t n = -1) : m_data(), m_len()
    {
        init(s, n);
    }

    string(string&& rhs) QSH_NOEXCEPT : m_data(), m_len()
    {
        std::swap(m_data, rhs.m_data);
        std::swap(m_len, rhs.m_len);
    }

    string(const string& s) : m_data(), m_len()
    { 
        init(s.m_data, s.m_len);
    }

    string& operator=(string&& rhs) QSH_NOEXCEPT
    {
        if (this != &rhs) {
            std::swap(m_data, rhs.m_data);
            std::swap(m_len, rhs.m_len);
        }
        return *this;
    }

    string& operator=(const string& rhs)
    {
        if (this != &rhs) {
            if (rhs.m_len > m_len) {
                deallocate(m_data);
                m_data = allocate<char>(rhs.m_len);
            }
            m_len = rhs.m_len;
            std::memcpy(m_data, rhs.m_data, m_len);
        }
        return *this;
    }

    ~string() QSH_NOEXCEPT
    {
        deallocate(m_data);
    }

    size_t size() const
    {
        return m_len;
    }

    void resize(size_t n)
    {
        if (m_len < n) {
            auto pn = allocate<char>(n);
            std::memcpy(pn, m_data, m_len);
            deallocate(m_data);
            m_data = pn;
        }
        m_len = n;
    }

    bool operator<(const string& s) const
    {
        auto pas = m_data;
        auto pae = pas + m_len;
        auto pbs = s.m_data;
        auto pbe = pbs + s.len;
        while ((pas != pae) && (pbs != pbe)) {
            if (*pas < *pbs) return true;
            if (*pbs < *pas) return false;
            ++pas; ++pbs;
        }
        return (pas == pae) && (pbs != pbe);
    }

    bool operator==(const string& s) const
    {
        return (m_len == s.m_len) && memcmp(m_data, s.m_data, m_len) == 0;
    }

    bool operator!=(const string& s) const
    {
        return !(*this == s);
    }

    char& operator[](size_t i)
    {
        return m_data[i];
    }

    const char& operator[](size_t i) const
    {
        return m_data[i];
    }

  private:
    void init(const char* s, int64_t n)
    {
        if (n == -1) {
            n = s ? std::strlen(s) : 0;
        }
        if ((m_len = n)) {
            m_data = allocate<char>(n);
            std::memcpy(m_data, s, n);
        }
    }
};

enum node_kind
{
    tINT,
    tFLOAT,
    tCHAR,
    tSTRING
};

typedef union
{
    long vlong;
    double vfloat;
    char vchar;
    string_data vstring;
    long* vlongarr;
    double* vfloatarr;
    string_data* vstringarr;
} node_fields;

class node_data
{
  public:
    node_data(kind k, bool arr, uint64_t sz)
      : m_kind(k), m_array(a), m_len(a ? sz : 1), m_data()
    {
        init();
    }

    node_data(const node_data& n)
      : m_kind(), m_array(), m_len(), m_data()
    {
        copy(n);
    }

    node_data& operator=(const node_data& n)
    {
        if (this != &n) {
            destroy();
            copy(n);
        }
        return *this;
    }

    ~node_data()
    {
        destroy();
    }

    bool is_array() const { return m_array; }

    node_kind kind() const { return m_kind; }

    size_t length() const { return m_len; }

    template <class functor>
    void apply(functor& f)
    {
        if (m_array) {
            switch (m_kind) {
            case tINT:
                f(m_data.vlongarr, m_len);
                break;
            case tFLOAT:
                f(m_data.vfloatarr, m_len);
                break;
            case tSTRING:
                f((string*)(m_data.vstringarr), m_len);
                break;
            }
        } else {
            switch (m_kind) {
            case tINT:
                f(m_data.vlong);
                break;
            case tFLOAT:
                f(m_data.vfloat);
                break;
            case tCHAR:
                f(m_data.vchar);
                break;
            case tSTRING:
                f(*((string*)&(m_data.vstring)));
                break;
            }
        }
    }
  private:
    node_kind   m_kind :  2;
    bool        m_array:  1;
    uint64_t    m_len  : 61;
    node_fields m_data;

    void init()
    {
        if (m_array) {
            switch (m_kind) {
            case tINT:
                m_data.vlongarr = allocate<long>(m_len);
                break;
            case tFLOAT:
                m_data.vfloatarr = allocate<double>(m_len);
                break;
            case tSTRING:
                m_data.vstringarr = allocate<string>(m_len);
                fill(m_data.vstringarr, m_len);
                break;
            }
        } else {
            switch (m_kind) {
            case tINT:
                m_data.vlong = 0;
                break;
            case tFLOAT:
                m_data.vfloat = 0.0;
                break;
            case tCHAR:
                m_data.vchar = '\0';
                break;
            case tSTRING:
                new (&m_data.vstring) string;
                break;
            }
        }
    }

    void destroy()
    {
        if (m_array) {
            if (m_kind == tSTRING)
                for (size_t i = 0 ; i < m_len; ++i) {
                    ((string*)(&m_data.vstringarr[i]))->~string();
                }
            }
            deallocate(m_data.vlongarr);
        } else if (m_kind == tSTRING) {
            ((string*)(&m_data.vstring))->~string();
        }
    }

    void copy(node_data& n)
    {
        m_kind = n.m_kind;
        m_array = n.m_array;
        m_len = n.m_len;
        if (m_kind != tSTRING) {
            init();
        }
        if (m_array) {
            switch (m_kind) {
            case tINT:
                std::copy(m_data.vlongarr, n.m_data.vlongarr, m_len);
                break;
            case tFLOAT:
                std::copy(m_data.vfloatarr, n.m_data.vfloatarr, m_len);
                break;
            case tSTRING:
                std::copy(m_data.vstringarr, n.m_data.vstringarr, m_len);
                break;
            }
        } else {
            if (m_kind == tSTRING) {
                new (&(m_data.vstring)) string(*(const string*)(&n.m_data.vstring));
            } else {
                m_data.vfloat = n.m_data.vfloat;
            }
        }
    }
};

} // namespace qsh

#endif/*_qsh_parser_parse_tree_hpp_*/
