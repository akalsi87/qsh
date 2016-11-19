/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file parser.hpp
 * \date Nov 12, 2016
 */

#ifndef _qsh_parser_parser_h_
#define _qsh_parser_parser_h_

#include "qsh/config.h"
#include "qsh/parser/parse_tree.hpp"
#include "qsh/util/stack_pimpl.hpp"

namespace qsh {

class parser_impl;

class QSH_API parser : noncopyable
{
  public:
    parser();
    ~parser();
    parse_tree parse_file(char const* filename);
    parse_tree parse_string(char const* str, ptrdiff_t len = -1);
    char const* err_msg() const;
  private:
    static const int IMPL_SIZE = 2*sizeof(size_t) + 120;
    stack_pimpl<parser_impl, IMPL_SIZE> m_impl;
};

} // namespace qsh

#endif/*_qsh_parser_parser_h_*/
