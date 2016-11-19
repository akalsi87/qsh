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

enum parse_node_kind
{
    KWD_BREAK = 258,
    KWD_CONTINUE = 259,
    KWD_DEF = 260,
    KWD_ELSE = 261,
    KWD_FOR = 262,
    KWD_IF = 263,
    KWD_RETURN = 264,
    KWD_VAR = 265,
    KWD_WHILE = 266,
    IDENT = 267,
    LIT_INT_HEX = 268,
    LIT_INT_DEC = 269,
    LIT_FLOAT = 270,
    LIT_CHAR = 271,
    LIT_STRING = 272,
    OP_SHIFT_L = 273,
    OP_SHIFT_R = 274,
    OP_INCR = 275,
    OP_DECR = 276,
    OP_AND = 277,
    OP_OR = 278,
    OP_LE = 279,
    OP_GE = 280,
    OP_EQ = 281,
    OP_NE = 282,
    TOK_SEMI = 283,
    TOK_CURLY_L = 284,
    TOK_CURLY_R = 285,
    TOK_COMMA = 286,
    TOK_COLON = 287,
    TOK_ASSIGN = 288,
    TOK_PAREN_L = 289,
    TOK_PAREN_R = 290,
    TOK_SQ_L = 291,
    TOK_SQ_R = 292,
    TOK_AND = 293,
    TOK_OR = 294,
    TOK_NOT = 295,
    TOK_FLIP = 296,
    TOK_MINUS = 297,
    TOK_PLUS = 298,
    TOK_MUL = 299,
    TOK_DIV = 300,
    TOK_MOD = 301,
    TOK_LT = 302,
    TOK_GT = 303,
    TOK_XOR = 304,
    TOK_TERNARY = 305
    /* end of auto gen */,
    VAR_DEF = 306,
    TREE_ROOT = 307
};

struct parse_node : noncopyable
{
    parse_node_kind kind;
    int32_t num_nodes;
    qsh::type const* type;
    int32_t line;
    int32_t col;
    char const* text;
    const_parse_node_range sub() const
    {
        auto arr = reinterpret_cast<parse_node* const*>(this+1);
        return const_parse_node_range(arr, static_cast<size_t>(num_nodes));
    }

    parse_node_range sub()
    {
        auto arr = reinterpret_cast<parse_node**>(this+1);
        return parse_node_range(arr, static_cast<size_t>(num_nodes));
    }
};

class parse_tree_builder_impl;

class parse_tree
{
  public:
    parse_tree()
      : m_root(&m_sentinel)
      , m_arena()
      , m_text()
      , m_sentinel()
    { }

    parse_tree(parse_tree const& rhs)
      : m_root(rhs.is_valid() ? rhs.m_root : &m_sentinel)
      , m_arena(rhs.m_arena)
      , m_text(rhs.m_text)
      , m_sentinel()
    { }

    bool is_valid() const { return m_root != &m_sentinel; }
    parse_node const* root() const { return m_root; }
    shared_ptr<arena> node_arena() const { return m_arena; }
    shared_ptr<arena> text_arena() const { return m_text; }
  private:
    parse_node* m_root;
    shared_ptr<arena> m_arena;
    shared_ptr<arena> m_text;
    parse_node m_sentinel;
    friend class parse_tree_builder_impl;
};

} // namespace qsh

#endif/*_qsh_parser_parse_tree_hpp_*/
