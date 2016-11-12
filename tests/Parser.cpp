/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file Parser.cpp
 * \date Nov 12, 2016
 */

#include "qsh/parser/parser.hpp"

#include <cstdlib>

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        exit(1);
    }
    return qsh::parser().parse_file(argv[1]) ? 0 : 1;
}
