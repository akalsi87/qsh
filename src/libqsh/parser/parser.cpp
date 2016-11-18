/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file parser.cpp
 * \date Nov 12, 2016
 */

#include "qsh/parser/parser.hpp"
#include "parse_tree.hpp"

#include <assert.h>
#include <fstream>
#include <sstream>

namespace qsh {

void push_node(void* ctxt, int line, int col, char const* lit, int len, int node_kind, int num_sub_nodes);

namespace {

#define YYDEBUG 1

#include "parse_tree_c.h"
#include "parser.h"
#include "parser.gen"
#include "tokenizer.gen"

void qsh_push_node(qsh_parse_context ctxt, int line, int col, char const* lit, int len, int node_kind, int num_sub_nodes)
{
    push_node(ctxt, line, col, lit, len, node_kind, num_sub_nodes);
}

} // namespace

struct parser_impl
{
    yyscan_t scanner;
    mutable YY_BUFFER_STATE buffstate;
    parse_tree_builder builder;

    parser_impl() : scanner(), buffstate(), builder()
    {
        auto fail = yylex_init(&scanner);
        assert(!fail);
        yyset_debug(YYDEBUG, scanner);
        yyset_extra(&builder, scanner);
    }

    bool parse_string(const char* str, int len = -1) const
    {
        if (len == -1) {
            len = strlen(str);
        }
        buffstate = yy_scan_bytes(str, len, scanner);
        yyset_lineno(1, scanner);
        yyset_column(1, scanner);
        bool fail = yyparse(scanner);
        yy_delete_buffer(buffstate, scanner);
        return !fail;
    }

    ~parser_impl()
    {
        yylex_destroy(scanner);
    }
};

class parse_tree_builder_impl
{
    shared_ptr<arena> m_arena;
    shared_ptr<arena> m_text;
    std::vector<parse_node*> m_stack;
  public:
    parse_tree_builder_impl()
      : m_arena()
      , m_text()
      , m_stack()
    {
        m_arena = shared_ptr<arena>::create();
        m_text = shared_ptr<arena>::create();
    }

    void push(int line, int col, char const* lit, int len, int node_kind, int num_sub_nodes)
    {
        assert(num_sub_nodes <= (int)m_stack.size());
        auto n = reinterpret_cast<parse_node*>(m_arena->allocate(sizeof(parse_node) + num_sub_nodes*sizeof(parse_node*)));
        n->kind = node_kind;
        n->num_nodes = num_sub_nodes;
        n->type = nullptr;
        n->line = line;
        n->col = col;
        n->text = m_text->create_string(lit, len);
        if (num_sub_nodes) {
            auto sn = n->subnodes();
            for (int i = num_sub_nodes-1; i >= 0; --i) {
                sn[i] = m_stack.back(); m_stack.pop_back();
            }
        }
        m_stack.push_back(n);
    }

    parse_tree release()
    {
        assert(m_stack.size() == 1);
        auto p = parse_tree();
        p.m_root = m_stack[0];
        p.m_arena = m_arena;
        p.m_text = m_text;
        m_arena = shared_ptr<arena>::create();
        m_text = shared_ptr<arena>::create();
        return p;
    }
};

parse_tree_builder::parse_tree_builder() : m_impl()
{
    m_impl.reset();
}

parse_tree_builder::~parse_tree_builder()
{
}

void parse_tree_builder::push_node(int line, int col, char const* lit, int len, int node_kind, int num_sub_nodes)
{
    m_impl->push(line, col, lit, len, node_kind, num_sub_nodes);
}

void push_node(void* ctxt, int line, int col, char const* lit, int len, int node_kind, int num_sub_nodes)
{
    auto builder = reinterpret_cast<parse_tree_builder*>(ctxt);
    builder->push_node(line, col, lit, len, node_kind, num_sub_nodes);
}

parser::parser() : m_impl()
{
    static_assert(sizeof(parser_impl) <= IMPL_SIZE, "Parser impl is sliced!");
    m_impl.reset();
}

parser::~parser()
{
}

bool parser::parse_file(const char* filename) const
{
    std::string contents;
    {// read file into string
        std::ifstream file(filename, std::ios::binary);
        std::ostringstream strm;
        strm << file.rdbuf();
        contents = strm.str(); 
    }
    return m_impl->parse_string(contents.c_str(), contents.size());
}

bool parser::parse_string(const char* str) const
{
    return m_impl->parse_string(str);
}

} // namespace qsh
