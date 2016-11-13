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
#include <cstring>
#include <fstream>

static const int turnOffStdoutBuff = []() { setvbuf(stdout, NULL, _IONBF, 0); return 1; }();
static const qsh::parser Parser;

void print_file(const char* str)
{
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

bool test_file(const char* str)
{
    char tmpfile[8193];
    const char* filename = std::tmpnam(tmpfile);
    std::fstream file(filename, std::ios::out | std::ios::binary);
    file << str;
    file.close();
    printf("Errors (file)\n--------\n");
    bool ok = Parser.parse_file(filename);
    printf("--------\n");
    std::remove(filename);
    return ok;
}

#define __PARSER_TEST(id, str, exp)             \
  CPP_TEST(id) {                                \
    print_file( str);                           \
    auto id##_strg_ok = test_string(str) == exp;\
    auto id##_file_ok = test_file(str) == exp;  \
    TEST_TRUE(id##_strg_ok);                    \
    TEST_TRUE(id##_file_ok);                    \
  }

#define PARSER_TEST_POS(id, str) __PARSER_TEST(id, str, true)
#define PARSER_TEST_NEG(id, str) __PARSER_TEST(id, str, false)

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
