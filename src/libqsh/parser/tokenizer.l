 /* Tokenizer grammar, uses flex++ options */

%option bison-bridge
%option bison-locations
%option ecs
%option nodefault reentrant noyywrap

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
#include "parser.h"

#define YY_USER_ACTION                  \
    {                                   \
        yylloc->first_line = yylloc->last_line;  \
        yylloc->first_column = yylloc->last_column;  \
        yylloc->last_column += yyleng;     \
    }

static
void consume_comment(yyscan_t, YYLTYPE* l);
%}

%%

"/*"                            { consume_comment(yyscanner, yylloc); }
"//".*$                         { /* eat the line */    }
"break"                         { return KWD_BREAK;     }
"continue"                      { return KWD_CONTINUE;  }
"def"                           { return KWD_DEF;       }
"else"                          { return KWD_ELSE;      }
"for"                           { return KWD_FOR;       }
"if"                            { return KWD_IF;        }
"return"                        { return KWD_RETURN;    }
"var"                           { return KWD_VAR;       }
"while"                         { return KWD_WHILE;     }

{IL}{A}*                        { return IDENT;         }

{HL}{H}+                        { return LIT_INT_HEX;   }
{S}?{D}+                        { return LIT_INT_DEC;   }

{S}?{D}+([\.]{E})?              { return LIT_FLOAT;     }

"'"([^'\\\n]|{ES})"'"           { return LIT_CHAR;      }

(\"([^"\\\n]|{ES})*\"{WS}*)+    { return LIT_STRING;    }

">>"                            { return OP_SHIFT_L;    }
"<<"                            { return OP_SHIFT_R;    }

"++"                            { return OP_INCR;       }
"--"                            { return OP_DECR;       }

"&&"                            { return OP_AND;        }
"||"                            { return OP_OR;         }

"<="                            { return OP_LE;         }
">="                            { return OP_GE;         }
"=="                            { return OP_EQ;         }
"!="                            { return OP_NE;         }

";"                             { return TOK_SEMI;      }
"{"                             { return TOK_CURLY_L;   }
"}"                             { return TOK_CURLY_R;   }
","                             { return TOK_COMMA;     }
":"                             { return TOK_COLON;     }
"="                             { return TOK_ASSIGN;    }
"("                             { return TOK_PAREN_L;   }
")"                             { return TOK_PAREN_R;   }
"["                             { return TOK_SQ_L;      }
"]"                             { return TOK_SQ_R;      }
"&"                             { return TOK_AND;       }
"|"                             { return TOK_OR;        }
"!"                             { return TOK_NOT;       }
"~"                             { return TOK_FLIP;      }
"-"                             { return TOK_MINUS;     }
"+"                             { return TOK_PLUS;      }
"*"                             { return TOK_MUL;       }
"/"                             { return TOK_DIV;       }
"%"                             { return TOK_MOD;       }
"<"                             { return TOK_LT;        }
">"                             { return TOK_GT;        }
"^"                             { return TOK_XOR;       }
"?"                             { return TOK_TERNARY;   }
{WS}+                           { /* disregard */       }
{NL}                            { yylloc->first_column = yylloc->last_column = 0; yylloc->first_line = yylloc->last_line++; }
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
            if (c == '\n') {
                l->first_column = l->last_column = 0; 
                l->first_line = l->last_line++;
            }
        } else if (c == '\n') {
            l->first_column = l->last_column = 0; 
            l->first_line = l->last_line++;
        }
    }
    yyerror(l, scanner, "Line comment with regular comment "
                        "not terminated (//* comment...)");
}