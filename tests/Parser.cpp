/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file Parser.cpp
 * \date Nov 12, 2016
 */

#include "qsh/parser/parser.hpp"

#include "unittest.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

static const int turnOffStdoutBuff = []() { setvbuf(stdout, NULL, _IONBF, 0); return 1; }();
static qsh::parser parser;

void print_file(const char* str)
{
    (void)turnOffStdoutBuff;
    int line = 1;
    printf("--------\n");
    while (str) {
        const char* line_end = str;
        char c;
        while (((c = *line_end) != '\n') && c) {
            ++line_end;
        }
        if (c) {
            printf("%3d %.*s", line++, (int)(line_end+1-str), str);
            str = line_end+1;
        } else {
            printf("%3d %s", line++, str);
            str = 0;
        }
    }
    printf("$ <-- EOF\n");
    printf("--------\n");
}

bool test_string(const char* str)
{
    printf("Errors (string)\n--------\n");
    auto tree = parser.parse_string(str);
    if (!tree.is_valid()) {
        printf("%s\n", parser.err_msg());
    }
    printf("--------\n");
    return tree.is_valid();
}

const char* get_env(const char* e)
{
    auto val = getenv(e);
    return val ? val : "";
}

const char* tmpdir()
{
    static std::string tmp;
    if (!tmp.empty()) {
        return tmp.c_str();
    }
    if (tmp.empty()) {
        tmp = get_env("TMPDIR");
    }
    if (tmp.empty()) {
        tmp = get_env("TMP");
    }
    if (tmp.empty()) {
        tmp = get_env("TEMP");
    }
    if (tmp.empty()) {
        tmp = get_env("TEMPDIR");
    }
    if (tmp.empty()) {
        tmp = "/tmp";
    }
#if defined(WIN32)
    for (char& c : tmp) {
        if (c == '\\') {
            c = '/';
        }
    }
#endif
    return tmp.c_str();
}

bool test_file(const char* id, const char* str)
{
    char tmpfile[8193];
    sprintf(tmpfile, "%s/qsh_parser_%s.qsh", tmpdir(), id);
    const char* filename = tmpfile;
    std::fstream file(filename, std::ios::out | std::ios::binary);
    file << str;
    file.close();
    printf("Errors (file)\n--------\n");
    auto tree = parser.parse_file(filename);
    if (!tree.is_valid()) {
        printf("%s\n", parser.err_msg());
    }
    printf("--------\n");
    return tree.is_valid();
}

#define __PARSER_TEST(id, str, exp)                 \
  CPP_TEST(id) {                                    \
    print_file(str);                                \
    auto id##StringOK = test_string(str) == exp;    \
    auto id##FileOK = test_file(#id, str) == exp;   \
    TEST_TRUE(id##StringOK);                        \
    TEST_TRUE(id##FileOK);                          \
  }

#define __PARSER_TEST_TREE(id, str, exp)            \
  CPP_TEST(id) {                                    \
    print_file(str);                                \
    auto id##StringOK = test_string(str) == exp;    \
    auto id##FileOK = test_file(#id, str) == exp;   \
    TEST_TRUE(id##StringOK);                        \
    TEST_TRUE(id##FileOK);                          \
  }                                                 \
  struct id##Data                                   \
  {                                                 \
    static char const* const text;                  \
  };                                                \
  char const* const id##Data:: text = str;          \
  CPP_TEST(id##Tree)

#define PARSER_TEST_POS(id, str) __PARSER_TEST(id, str, true)
#define PARSER_TEST_NEG(id, str) __PARSER_TEST(id, str, false)

#define PARSER_TEST_TREE_POS(id, str) __PARSER_TEST_TREE(id, str, true)
#define PARSER_TEST_TREE_NEG(id, str) __PARSER_TEST_TREE(id, str, false)

#define TEST_WITH(x,...)  \
  if (!(__VA_ARGS__))   \
  assert(false);        \
  else if (decltype(__VA_ARGS__) x = __VA_ARGS__)

PARSER_TEST_TREE_POS(
    varDef,
    "var x = 1;\n")
{
    qsh::parser p;
    auto t = p.parse_string(varDefData::text);
    TEST_WITH(root, t.root()->sub()[0]) {
        TEST_TRUE(root->kind == qsh::VAR_DEF);
        TEST_TRUE(root->num_nodes == 3);
        TEST_TRUE(root->is_array() == false);
        TEST_TRUE(!strcmp(root->var_name(), "x"));
        TEST_WITH(init, root->var_init()) {
            TEST_TRUE(init->kind == qsh::LIT_INT_DEC);
            TEST_TRUE(init->num_nodes == 0);
            TEST_TRUE(!strcmp(init->text, "1"));
        }
    }
}

PARSER_TEST_TREE_POS(
    singleLineCommentsOnly,
    "// foo\n"
    "\n"
    "     \t\f\v    \n"
    "// g\n"
    "//"
)
{
    qsh::parser p;
    auto t = p.parse_string(singleLineCommentsOnlyData::text);
    auto root = t.root();
    TEST_TRUE(root->kind == qsh::TREE_ROOT);
    TEST_TRUE(root->sub().size() == 0);
}

PARSER_TEST_TREE_POS(
    emptyFcnDef,
    "// foo\n"
    "def foo(var x, var y) {\n"
    "}\n"
)
{
    qsh::parser p;
    auto t = p.parse_string(emptyFcnDefData::text);
    TEST_WITH(root, t.root()->sub()[0]) {
        TEST_TRUE(root->kind == qsh::FUNC_DEF);
        TEST_TRUE(root->num_nodes == 4);
        TEST_TRUE(root->num_formals() == 2);
        TEST_TRUE(!strcmp(root->fcn_name(), "foo"));
        TEST_TRUE(root->num_stmts() == 0);
        TEST_TRUE(root->stmts().size() == 0);
        TEST_WITH(formal0, root->formals()[0]) {
            TEST_TRUE(!strcmp(formal0->text, "x"));
        }
        TEST_WITH(formal1, root->formals()[1]) {
            TEST_TRUE(!strcmp(formal1->text, "y"));
        }
    }
}


PARSER_TEST_TREE_POS(
    singleStmtFcnDef,
    "// foo\n"
    "def foo(var x, var y) {\n"
    "  var z = 1;\n"
    "}\n"
)
{
    qsh::parser p;
    auto t = p.parse_string(singleStmtFcnDefData::text);
    TEST_WITH(root, t.root()->sub()[0]) {
        TEST_TRUE(root->kind == qsh::FUNC_DEF);
        TEST_TRUE(root->num_nodes == 5);
        TEST_TRUE(!strcmp(root->fcn_name(), "foo"));
        TEST_TRUE(root->num_formals() == 2);
        TEST_WITH(formal0, root->formals()[0]) {
            TEST_TRUE(!strcmp(formal0->text, "x"));
        }
        TEST_WITH(formal1, root->formals()[1]) {
            TEST_TRUE(!strcmp(formal1->text, "y"));
        }
        TEST_TRUE(root->num_stmts() == 1);
        TEST_TRUE(root->stmts().size() == 1);
        TEST_WITH(varDef, root->stmts()[0]) {
            TEST_TRUE(varDef->kind == qsh::VAR_DEF);
            TEST_TRUE(varDef->sub().size() == 3);
            TEST_TRUE(!strcmp(varDef->var_name(), "z"));
            TEST_WITH(val, varDef->var_init()) {
                TEST_TRUE(!strcmp(val->text, "1"));
                TEST_TRUE(val->kind == qsh::LIT_INT_DEC);
            }
        }
    }
}

PARSER_TEST_TREE_POS(
    returnStmtFcnDef,
    "// foo\n"
    "def foo(var x, var y) {\n"
    "  var z = 1;\n"
    "  return;\n"
    "  return z;\n"
    "}\n"
)
{
    qsh::parser p;
    auto t = p.parse_string(returnStmtFcnDefData::text);
    TEST_WITH(root, t.root()->sub()[0]) {
        TEST_TRUE(root->kind == qsh::FUNC_DEF);
        TEST_TRUE(root->num_nodes == 7);
        TEST_TRUE(!strcmp(root->fcn_name(), "foo"));
        TEST_TRUE(root->num_formals() == 2);
        TEST_WITH(formal0, root->formals()[0]) {
            TEST_TRUE(!strcmp(formal0->text, "x"));
        }
        TEST_WITH(formal1, root->formals()[1]) {
            TEST_TRUE(!strcmp(formal1->text, "y"));
        }
        TEST_TRUE(root->num_stmts() == 3);
        TEST_TRUE(root->stmts().size() == 3);
        TEST_WITH(varDef, root->stmts()[0]) {
            TEST_TRUE(varDef->kind == qsh::VAR_DEF);
            TEST_TRUE(varDef->sub().size() == 3);
            TEST_TRUE(!strcmp(varDef->var_name(), "z"));
            TEST_WITH(val, varDef->var_init()) {
                TEST_TRUE(!strcmp(val->text, "1"));
                TEST_TRUE(val->kind == qsh::LIT_INT_DEC);
            }
        }
        TEST_WITH(ret0, root->stmts()[1]) {
            TEST_TRUE(ret0->kind == qsh::KWD_RETURN);
            TEST_TRUE(ret0->sub().size() == 0);
        }
        TEST_WITH(ret1, root->stmts()[2]) {
            TEST_TRUE(ret1->kind == qsh::KWD_RETURN);
            TEST_TRUE(ret1->sub().size() == 1);
            TEST_WITH(expr, ret1->sub()[0]) {
                TEST_TRUE(expr->kind == qsh::IDENT);
                TEST_TRUE(!strcmp(expr->text, "z"));
            }
        }
    }
}

PARSER_TEST_TREE_POS(
    simpleMain,
    "// foo comment\n"
    "/* long\n"
    " *\n"
    " * comment*/\n"
    "\n"
    "var foo = \"foooooo\"     \"barrrr\"        \"x\";\n"
    "\n"
    "///*\n"
    "def foo(var x, var y) {\n"
    "\n"
    "}\n"
    "\n"
    "var t= 1;\n"
)
{
    qsh::parser p;
    auto tree = p.parse_string(simpleMainData::text);
    TEST_TRUE(tree.root()->sub().size() == 3);
    TEST_WITH(varDef, tree.root()->sub()[0]) {
        TEST_TRUE(varDef->kind == qsh::VAR_DEF);
    }
    TEST_WITH(funcDef, tree.root()->sub()[1]) {
        TEST_TRUE(funcDef->kind == qsh::FUNC_DEF);
    }
    TEST_WITH(varT, tree.root()->sub()[2]) {
        TEST_TRUE(varT->kind == qsh::VAR_DEF);
    }
}

PARSER_TEST_NEG(
    rightEndCommentLine9,
    "// foo comment\n"
    "/* long\n"
    " *\n"
    " * comment*/\n"
    "\n"
    "var foo = \"foooooo\"     \"barrrr\"        \"x\";\n"
    "\n"
    "///*\n"
    "*/\n"
    "def foo(var x, var y) {\n"
    "\n"
    "}\n"
    "\n"
    "var t= 1;\n"
)

PARSER_TEST_NEG(
    danglingVarIncomplete,
    "var x"
)

PARSER_TEST_NEG(
    varInitArrNonArr,
    "var x = {1};"
)

PARSER_TEST_NEG(
    varInitArrNoCurlyEnd,
    "var x[] = {1"
)

PARSER_TEST_POS(
    varInitArr,
    "var x[] = {1};"
)

PARSER_TEST_TREE_POS(
    accessGlobalConst,
    "var x[] = {1};\n"
    "def get(var i) {\n"
    "  return x[0];\n"
    "}\n"
)
{
    qsh::parser p;
    auto tree = p.parse_string(accessGlobalConstData::text);
    TEST_TRUE(tree.root()->sub().size() == 2);
    TEST_WITH(varDef, tree.root()->sub()[0]) {
        TEST_TRUE(varDef->kind == qsh::VAR_DEF);
    }
    TEST_WITH(funcDef, tree.root()->sub()[1]) {
        TEST_TRUE(funcDef->kind == qsh::FUNC_DEF);
        TEST_WITH(ret, funcDef->stmts()[0]) {
            TEST_WITH(val, ret->sub()[0]) {
                TEST_TRUE(val->kind == qsh::EXPR_INDEX);
                TEST_TRUE(val->sub()[1]->kind == qsh::LIT_INT_DEC);
            }
        }
    }
}

PARSER_TEST_TREE_POS(
    accessGlobalVar,
    "var x[] = {1};\n"
    "def get(var i) {\n"
    "  return x[i];\n"
    "}\n"
)
{
    qsh::parser p;
    auto tree = p.parse_string(accessGlobalVarData::text);
    TEST_TRUE(tree.root()->sub().size() == 2);
    TEST_WITH(varDef, tree.root()->sub()[0]) {
        TEST_TRUE(varDef->kind == qsh::VAR_DEF);
    }
    TEST_WITH(funcDef, tree.root()->sub()[1]) {
        TEST_TRUE(funcDef->kind == qsh::FUNC_DEF);
        TEST_WITH(ret, funcDef->stmts()[0]) {
            TEST_WITH(val, ret->sub()[0]) {
                TEST_TRUE(val->kind == qsh::EXPR_INDEX);
                TEST_TRUE(val->sub()[1]->kind == qsh::IDENT);
            }
        }
    }
}
