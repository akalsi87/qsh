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

class parser : noncopyable
{
  public:
    parser();
    ~parser();
    void parse_file(const char* filename);
  private:
    static const int IMPL_SIZE = 16;
    stack_pimpl<parser_impl, IMPL_SIZE> m_impl;
};

} // namespace qsh

#endif/*_qsh_parser_parser_h_*/
