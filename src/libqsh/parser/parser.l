%token KWD_BREAK KWD_CONTINUE KWD_DEF KWD_ELSE KWD_FOR KWD_IF KWD_RETURN KWD_VAR KWD_WHILE
%token IDENT LIT_INT_HEX LIT_INT_DEC LIT_FLOAT LIT_CHAR LIT_STRING
%token OP_SHIFT_L OP_SHIFT_R OP_INCR OP_DECR OP_AND OP_OR OP_LE OP_GE OP_EQ OP_NE
%token TOK_SEMI TOK_CURLY_L TOK_CURLY_R TOK_COMMA TOK_COLON TOK_ASSIGN TOK_PAREN_L TOK_PAREN_R TOK_SQ_L TOK_SQ_R TOK_AND TOK_OR TOK_NOT TOK_FLIP TOK_MINUS TOK_PLUS TOK_MUL TOK_DIV TOK_MOD TOK_LT TOK_GT TOK_XOR TOK_TERNARY

%start translation_unit
%%

primary_expression
    : IDENT
    | constant
    | TOK_PAREN_L expression TOK_PAREN_R
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
    | postfix_expression TOK_SQ_L expression TOK_SQ_R
    | postfix_expression TOK_PAREN_L TOK_PAREN_R
    | postfix_expression TOK_PAREN_L argument_expression_list TOK_PAREN_R
    | postfix_expression OP_INCR
    | postfix_expression OP_DECR
    ;
    /*| '(' type_name ')' '{' initializer_list '}'
      | '(' type_name ')' '{' initializer_list ',' '}'
     */

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


constant_expression
    : conditional_expression	/* with constraints */
    ;

declaration
    : declaration_specifiers TOK_SEMI
    | declaration_specifiers init_declarator TOK_SEMI
    ;

declaration_specifiers
    : KWD_VAR
    | KWD_DEF
    ;

init_declarator
    : declarator TOK_ASSIGN initializer
    | declarator
    ;

param_list
    : KWD_VAR IDENT
    | param_list TOK_COMMA KWD_VAR IDENT
    ;

declarator
    : IDENT
    | declarator TOK_SQ_L TOK_SQ_R
    | declarator TOK_SQ_L constant_expression TOK_SQ_R
    | declarator TOK_PAREN_L TOK_PAREN_R
    | declarator TOK_PAREN_L param_list TOK_PAREN_R
    ;

initializer
    : TOK_CURLY_L initializer_list TOK_CURLY_R
    | assignment_expression
    ;

initializer_list
    : initializer
    | initializer_list TOK_COMMA initializer
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
    : declaration
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
    | KWD_FOR TOK_PAREN_L declaration expression_statement TOK_PAREN_R statement
    | KWD_FOR TOK_PAREN_L declaration expression_statement expression TOK_PAREN_R statement
    ;

jump_statement
    : KWD_CONTINUE TOK_SEMI
    | KWD_BREAK TOK_SEMI
    | KWD_RETURN TOK_SEMI
    | KWD_RETURN expression TOK_SEMI
    ;

translation_unit
    : external_declaration
    | translation_unit external_declaration
    ;

external_declaration
    : function_definition
    | declaration
    ;

formal
    : KWD_VAR IDENT
    ;

formals_list
    : formal
    | formals_list TOK_COMMA formal

function_definition
    : declaration_specifiers IDENT TOK_PAREN_L TOK_PAREN_R compound_statement
    | declaration_specifiers IDENT TOK_PAREN_L formals_list TOK_PAREN_R compound_statement
    ;


%%
#include <stdio.h>

void yyerror(const char* s)
{
    fflush(stdout);
    fprintf(stderr, "*** %s\n", s);
}

int main(int argc, const char* argv[])
{
    yydebug = 1;
    yyparse();
}
