/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file LibQsh.cpp
 * \date Nov 11, 2016
 */

#include "qsh/version.hpp"

#include "unittest.hpp"
#include "string.h"

CPP_TEST(ver)
{
    TEST_TRUE(!strcmp(qsh::version(), "0.0.1"));
}
