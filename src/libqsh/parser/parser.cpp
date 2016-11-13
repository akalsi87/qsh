/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file parser.cpp
 * \date Nov 12, 2016
 */

#include "qsh/parser/parser.hpp"

#include <assert.h>
#include <fstream>
#include <sstream>

namespace {

extern "C" {

#include "parser.h"
#include "parser.gen"
#include "tokenizer.gen"

} /* extern "C" */

} // namespace

namespace qsh {

struct parser_impl
{
    yyscan_t scanner;
    mutable YY_BUFFER_STATE buffstate;

    parser_impl()
    {
        auto fail = yylex_init(&scanner);
        assert(!fail);
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
        std::ifstream file(filename);
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
