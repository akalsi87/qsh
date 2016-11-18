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
static const qsh::parser Parser;

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
    auto ok = Parser.parse_string(str);
    printf("--------\n");
    return ok;
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
    bool ok = Parser.parse_file(filename);
    printf("--------\n");
    return ok;
}

#define __PARSER_TEST(id, str, exp)                 \
  CPP_TEST(id) {                                    \
    print_file(str);                                \
    auto id##_strg_ok = test_string(str) == exp;    \
    auto id##_file_ok = test_file(#id, str) == exp; \
    TEST_TRUE(id##_strg_ok);                        \
    TEST_TRUE(id##_file_ok);                        \
  }

#define PARSER_TEST_POS(id, str) __PARSER_TEST(id, str, true)
#define PARSER_TEST_NEG(id, str) __PARSER_TEST(id, str, false)

PARSER_TEST_POS(
    varDecl,
    "var x = 1;\n"
)

PARSER_TEST_POS(
    singleLineCommentsOnly,
    "// foo\n"
    "\n"
    "     \t\f\v    \n"
    "// g\n"
    "//"
)

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

PARSER_TEST_POS(
    varInit,
    "var x = 1;"
)

PARSER_TEST_NEG(
    varInitArrNonArr,
    "var x = {1};"
)

PARSER_TEST_NEG(
    varInitArrNoCurlyEnd,
    "var x = {1"
)

PARSER_TEST_POS(
    varInitArr,
    "var x[] = {1};"
)

PARSER_TEST_POS(
    accessGlobalConst,
    "var x[] = {1};\n"
    "def get(var i) {\n"
    "  return x[0];\n"
    "}\n"
)

PARSER_TEST_POS(
    accessGlobalVar,
    "var x[] = {1};\n"
    "def get(var i) {\n"
    "  return x[i];\n"
    "}\n"
)
