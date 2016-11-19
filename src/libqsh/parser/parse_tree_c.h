/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file parse_tree_c.h
 * \date Nov 12, 2016
 */

#ifndef _qsh_parser_parse_tree_c_h_
#define _qsh_parser_parse_tree_c_h_

#include "qsh/config.h"

typedef void* qsh_parse_context;

void qsh_push_node(qsh_parse_context ctxt, int line, int col, char const* lit, int len, int node_kind, int num_sub_nodes);

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
    int first_line;
    int first_column;
    int last_line;
    int last_column;
};

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

#endif/*_qsh_parser_parse_tree_c_h_*/
