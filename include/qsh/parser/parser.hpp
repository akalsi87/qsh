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
#include "qsh/util/stack_pimpl.hpp"

namespace qsh {

class parser_impl;

class QSH_API parser : noncopyable
{
  public:
    parser();
    ~parser();
    bool parse_file(char const* filename) const;
    bool parse_string(char const* str) const;
  private:
    static const int IMPL_SIZE = 2*sizeof(size_t) + 64;
    stack_pimpl<parser_impl, IMPL_SIZE> m_impl;
};

} // namespace qsh

#endif/*_qsh_parser_parser_h_*/
