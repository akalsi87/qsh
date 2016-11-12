/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Skeleton interface for Bison GLR parsers in C

   Copyright (C) 2002-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_SRC_LIBQSH_PARSER_GEN_PARSER_H_INCLUDED
# define YY_YY_SRC_LIBQSH_PARSER_GEN_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 11 "src/libqsh/parser/parser.l" /* glr.c:2555  */


#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif


#line 53 "src/libqsh/parser/gen_parser.h" /* glr.c:2555  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
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
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
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
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (void* scanner);

#endif /* !YY_YY_SRC_LIBQSH_PARSER_GEN_PARSER_H_INCLUDED  */
