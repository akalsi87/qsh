 /* Tokenizer grammar, uses flex++ options */

%option bison-bridge
%option bison-locations
%option ecs
%option nodefault reentrant noyywrap yylineno



S  [+-]
D  [0-9]
H  [a-fA-F0-9]
A  [a-zA-Z]
E  ([Ee]{S}?{D}+)
WS [ \t\v\f\r]
NL \n
HL (0[xX])
IL [a-zA-Z_]
ES (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))

 /* for details, see http://www.quut.com/c/ANSI-C-grammar-l.html
  * or http://www.quut.com/c/ANSI-C-grammar-y.html
  */

%{
#include <stdio.h>
#include "parse_tree_c.h"
#include "parser.h"

#define YY_USER_ACTION                  \
    {                                   \
        yylloc->first_line = yylloc->last_line;  \
        yylloc->first_column = yylloc->last_column;  \
        yylloc->last_column += yyleng;     \
        yylloc->last_line = yylineno;   \
    }

#define PUSH_NODE(k) qsh_push_node(yyextra, yylloc->first_line, yylloc->first_column, yytext, yyleng, k, 0); *yylval = { yylloc, yytext, yyleng, yyextra }

static
void consume_comment(yyscan_t, YYLTYPE* l);
%}

%%

"/*"                            { consume_comment(yyscanner, yylloc); }
"//".*                          { /* eat the line */        }
"break"                         { PUSH_NODE(KWD_BREAK);     }
"continue"                      { PUSH_NODE(KWD_CONTINUE);  }
"def"                           { PUSH_NODE(KWD_DEF);       }
"else"                          { PUSH_NODE(KWD_ELSE);      }
"for"                           { PUSH_NODE(KWD_FOR);       }
"if"                            { PUSH_NODE(KWD_IF);        }
"return"                        { PUSH_NODE(KWD_RETURN);    }
"var"                           { PUSH_NODE(KWD_VAR);       }
"while"                         { PUSH_NODE(KWD_WHILE);     }

{IL}{A}*                        { PUSH_NODE(IDENT);         }

{HL}{H}+                        { PUSH_NODE(LIT_INT_HEX);   }
{S}?{D}+                        { PUSH_NODE(LIT_INT_DEC);   }

{S}?{D}+([\.]{E})?              { PUSH_NODE(LIT_FLOAT);     }

"'"([^'\\\n]|{ES})"'"           { PUSH_NODE(LIT_CHAR);      }

(\"([^"\\\n]|{ES})*\"{WS}*)+    { PUSH_NODE(LIT_STRING);    }

">>"                            { PUSH_NODE(OP_SHIFT_L);    }
"<<"                            { PUSH_NODE(OP_SHIFT_R);    }

"++"                            { PUSH_NODE(OP_INCR);       }
"--"                            { PUSH_NODE(OP_DECR);       }

"&&"                            { PUSH_NODE(OP_AND);        }
"||"                            { PUSH_NODE(OP_OR);         }

"<="                            { PUSH_NODE(OP_LE);         }
">="                            { PUSH_NODE(OP_GE);         }
"=="                            { PUSH_NODE(OP_EQ);         }
"!="                            { PUSH_NODE(OP_NE);         }

";"                             { PUSH_NODE(TOK_SEMI);      }
"{"                             { PUSH_NODE(TOK_CURLY_L);   }
"}"                             { PUSH_NODE(TOK_CURLY_R);   }
","                             { PUSH_NODE(TOK_COMMA);     }
":"                             { PUSH_NODE(TOK_COLON);     }
"="                             { PUSH_NODE(TOK_ASSIGN);    }
"("                             { PUSH_NODE(TOK_PAREN_L);   }
")"                             { PUSH_NODE(TOK_PAREN_R);   }
"["                             { PUSH_NODE(TOK_SQ_L);      }
"]"                             { PUSH_NODE(TOK_SQ_R);      }
"&"                             { PUSH_NODE(TOK_AND);       }
"|"                             { PUSH_NODE(TOK_OR);        }
"!"                             { PUSH_NODE(TOK_NOT);       }
"~"                             { PUSH_NODE(TOK_FLIP);      }
"-"                             { PUSH_NODE(TOK_MINUS);     }
"+"                             { PUSH_NODE(TOK_PLUS);      }
"*"                             { PUSH_NODE(TOK_MUL);       }
"/"                             { PUSH_NODE(TOK_DIV);       }
"%"                             { PUSH_NODE(TOK_MOD);       }
"<"                             { PUSH_NODE(TOK_LT);        }
">"                             { PUSH_NODE(TOK_GT);        }
"^"                             { PUSH_NODE(TOK_XOR);       }
"?"                             { PUSH_NODE(TOK_TERNARY);   }
{WS}+                           { /* disregard */       }
{NL}                            { yylloc->first_column = yylloc->last_column = 0; }
.                               { /* ignore bad chars */}

%%

static
void consume_comment(yyscan_t scanner, YYLTYPE* l)
{
    int c;
    while ((c = yyinput(scanner)) != 0) {
        if (c == '*') {
            while ((c = yyinput(scanner)) == '*') { }
            if (c == '/') {
                return;
            }
            if (c == 0) {
                break;
            }
        }
    }
    yyerror(l, scanner, "Line comment with regular comment "
                        "not terminated (//* comment...)");
}
