 /* Parser grammar */

%define parse.error verbose
%define api.pure true
%locations
%token-table
%glr-parser
%lex-param {void* scanner}
%parse-param {void* scanner}

%code top {

#ifndef _qsh_parser_gen_
#define _qsh_parser_gen_

#include "parse_tree_c.h"

int yylex(YYSTYPE * yyval_param, YYLTYPE * yylloc_param, void * yyscanner);
int yyerror(YYLTYPE* yylloc_param, void *scanner, char const* s);

#define PUSH_NODE(k, n)

}

%token KWD_BREAK KWD_CONTINUE KWD_DEF KWD_ELSE KWD_FOR KWD_IF KWD_RETURN KWD_VAR KWD_WHILE
%token IDENT LIT_INT_HEX LIT_INT_DEC LIT_FLOAT LIT_CHAR LIT_STRING
%right OP_SHIFT_L OP_SHIFT_R OP_INCR OP_DECR OP_AND OP_OR OP_LE OP_GE OP_EQ OP_NE
%token TOK_SEMI TOK_CURLY_L TOK_CURLY_R TOK_COMMA TOK_COLON 
%right TOK_ASSIGN
%left TOK_PAREN_L TOK_PAREN_R TOK_SQ_L TOK_SQ_R TOK_AND TOK_OR TOK_NOT TOK_FLIP TOK_MINUS TOK_PLUS TOK_MUL TOK_DIV TOK_MOD TOK_LT TOK_GT TOK_XOR TOK_TERNARY

 //%token EXPR EXPR_INDX EXPR_CALL VAR_DEF

%start translation_unit
%%

primary_expression
    : IDENT
    | constant
    | TOK_PAREN_L expression TOK_PAREN_R { PUSH_NODE(EXPR, 3); }
    ;

constant
    : LIT_CHAR
    | LIT_STRING
    | LIT_INT_HEX
    | LIT_INT_DEC
    | LIT_FLOAT
    ;

postfix_expression
    : primary_expression
    | postfix_expression TOK_SQ_L expression TOK_SQ_R { PUSH_NODE(EXPR_INDX, 4); }
    | postfix_expression TOK_PAREN_L TOK_PAREN_R { PUSH_NODE(EXPR_CALL, 3); }
    | postfix_expression TOK_PAREN_L argument_expression_list TOK_PAREN_R
    | postfix_expression OP_INCR
    | postfix_expression OP_DECR
    ;

argument_expression_list
    : assignment_expression
    | argument_expression_list TOK_COMMA assignment_expression
    ;

unary_expression
    : postfix_expression
    | OP_INCR unary_expression
    | OP_DECR unary_expression
    | TOK_PLUS unary_expression
    | TOK_MINUS unary_expression
    | TOK_FLIP unary_expression
    | TOK_NOT unary_expression
    ;

multiplicative_expression
    : unary_expression
    | multiplicative_expression TOK_MUL multiplicative_expression
    | multiplicative_expression TOK_DIV multiplicative_expression
    | multiplicative_expression TOK_MOD multiplicative_expression
    ;

additive_expression
    : multiplicative_expression
    | additive_expression TOK_PLUS additive_expression
    | additive_expression TOK_MINUS additive_expression
    ;

shift_expression
    : additive_expression
    | shift_expression OP_SHIFT_L additive_expression
    | shift_expression OP_SHIFT_R additive_expression
    ;

relational_expression
    : shift_expression
    | relational_expression TOK_LT shift_expression
    | relational_expression TOK_GT shift_expression
    | relational_expression OP_LE shift_expression
    | relational_expression OP_GE shift_expression
    ;

equality_expression
    : relational_expression
    | equality_expression OP_EQ relational_expression
    | equality_expression OP_NE relational_expression
    ;

and_expression
    : equality_expression
    | and_expression TOK_AND equality_expression
    ;

xor_expression
    : and_expression
    | xor_expression TOK_XOR and_expression
    ;

or_expression
    : xor_expression
    | or_expression TOK_OR xor_expression
    ;

log_and_expression
    : or_expression
    | log_and_expression OP_AND or_expression
    ;

log_or_expression
    : log_and_expression
    | log_or_expression OP_OR log_and_expression
    ;

conditional_expression
    : log_or_expression
    | log_or_expression TOK_TERNARY expression TOK_COLON conditional_expression
    ;

assignment_expression
    : conditional_expression
    | unary_expression TOK_ASSIGN assignment_expression
    ;

expression
    : assignment_expression
    ;

var_definition
    : KWD_VAR init_declarator TOK_SEMI { PUSH_NODE(VAR_DEF, 3); }
    ;

init_declarator
    : IDENT TOK_ASSIGN assignment_expression
    | IDENT TOK_SQ_L TOK_SQ_R TOK_ASSIGN TOK_CURLY_L initializer_list TOK_CURLY_R
    ;

initializer_list
    : assignment_expression
    | initializer_list TOK_COMMA assignment_expression
    ;

statement
    : compound_statement
    | expression_statement
    | selection_statement
    | iteration_statement
    | jump_statement
    ;

compound_statement
    : TOK_CURLY_L TOK_CURLY_R
    | TOK_CURLY_L block_item_list TOK_CURLY_R
    ;

block_item_list
    : block_item
    | block_item_list block_item
    ;

block_item
    : var_definition
    | statement
    ;

expression_statement
    : TOK_SEMI
    | expression TOK_SEMI
    ;

selection_statement
    : KWD_IF TOK_PAREN_L expression TOK_PAREN_R statement KWD_ELSE statement
    | KWD_IF TOK_PAREN_L expression TOK_PAREN_R statement
    ;

iteration_statement
    : KWD_WHILE TOK_PAREN_L expression TOK_PAREN_R statement
    | KWD_FOR TOK_PAREN_L expression_statement expression_statement TOK_PAREN_R statement
    | KWD_FOR TOK_PAREN_L expression_statement expression_statement expression TOK_PAREN_R statement
    | KWD_FOR TOK_PAREN_L var_definition expression_statement TOK_PAREN_R statement
    | KWD_FOR TOK_PAREN_L var_definition expression_statement expression TOK_PAREN_R statement
    ;

jump_statement
    : KWD_CONTINUE TOK_SEMI
    | KWD_BREAK TOK_SEMI
    | KWD_RETURN TOK_SEMI
    | KWD_RETURN expression TOK_SEMI
    ;

translation_unit
    : /* empty */
    | external_declaration
    ;

external_declaration
    : function_definition
    | var_definition
    | external_declaration function_definition
    | external_declaration var_definition
    ;

formal
    : KWD_VAR IDENT
    ;

formals_list
    : formal
    | formals_list TOK_COMMA formal

function_definition
    : KWD_DEF IDENT TOK_PAREN_L TOK_PAREN_R compound_statement
    | KWD_DEF IDENT TOK_PAREN_L formals_list TOK_PAREN_R compound_statement
    ;

%%

#undef PUSH_NODE

#include <stdio.h>

int yyerror(YYLTYPE* locp, yyscan_t scanner, const char* msg) {
    (void)scanner;
    if (locp) {
        fprintf(stderr, "  %s:%d col %d : %s\n",
                "<file>",
                locp->first_line, locp->first_column,
                msg);
        /* todo: add some fancy ^^^^^ error handling here */
    } else {
        fprintf(stderr, "%s\n", msg);
    }
    return 0;
}

#endif// _qsh_parser_gen_
