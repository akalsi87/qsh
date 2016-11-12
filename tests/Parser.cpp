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
#include <fstream>

static const qsh::parser Parser;

bool test_string(const char* str)
{
    return Parser.parse_string(str);
}

bool test_file(const char* str)
{
    char tmpfile[8193];
    const char* filename = std::tmpnam(tmpfile);
    std::fstream file(filename, std::ios::out | std::ios::binary);
    file << str;
    file.close();
    bool ok = Parser.parse_file(filename);
    std::remove(filename);
    return ok;
}

#define __PARSER_TEST(id, str, exp)             \
  CPP_TEST(id) {                                \
    auto id##_strg_ok = test_string(str) == exp;\
    auto id##_file_ok = test_file(str) == exp;  \
    TEST_TRUE(id##_strg_ok);                    \
    TEST_TRUE(id##_file_ok);                    \
  }

#define PARSER_TEST_POS(id, str) __PARSER_TEST(id, str, true)
#define PARSER_TEST_NEG(id, str) __PARSER_TEST(id, str, true)

PARSER_TEST_POS(
    singleLineCommentsOnly,
    "// foo\n"
    "\n"
    "     \t\f\v    \n"
    "// g\n"
    "//"
)
