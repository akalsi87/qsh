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
#include "qsh/alloc/arena.hpp"
#include "qsh/types/types.hpp"
#include "qsh/util/shared_ptr.hpp"
#include "qsh/util/noncopyable.hpp"

namespace qsh {

struct parse_node;

using parse_node_range = range<parse_node*>;
using const_parse_node_range = const_range<parse_node const*>;

struct parse_node : noncopyable
{
    int32_t kind;
    int32_t num_nodes;
    qsh::type const* type;
    int32_t line;
    int32_t col;
    char const* text;
    const_parse_node_range subnodes() const
    {
        auto arr = reinterpret_cast<parse_node* const*>(this+1);
        return const_parse_node_range(arr, static_cast<size_t>(num_nodes));
    }

    parse_node_range subnodes()
    {
        auto arr = reinterpret_cast<parse_node**>(this+1);
        return parse_node_range(arr, static_cast<size_t>(num_nodes));
    }
};

class parse_tree_builder_impl;
class parse_tree
{
  public:
    parse_node const* root() const { return m_root; }
    shared_ptr<arena> node_arena() const { return m_arena; }
    shared_ptr<arena> text_arena() const { return m_text; }
  private:
    parse_node* m_root;
    shared_ptr<arena> m_arena;
    shared_ptr<arena> m_text;
    friend class parse_tree_builder_impl;
};

class parse_tree_builder : noncopyable
{
  public:
    parse_tree_builder();
    ~parse_tree_builder();
    void push(int node_kind, int num_sub_nodes, char const* lit);
    parse_tree release();
    void push_node(int line, int col, char const* lit, int len, int node_kind, int num_sub_nodes);
  private:
    static const size_t IMPL_SIZE = 48;
    stack_pimpl<parse_tree_builder_impl, IMPL_SIZE> m_impl;
};

} // namespace qsh

#endif/*_qsh_parser_parse_tree_hpp_*/
