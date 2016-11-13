/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file Utils.cpp
 * \date Nov 12, 2016
 */

#include "qsh/util/shared_ptr.hpp"

#include "unittest.hpp"

CPP_TEST(sharedPtr)
{
    auto p = qsh::shared_ptr<int>::create();
    TEST_TRUE(*p == 0);
    TEST_TRUE(p);
    TEST_TRUE(p.use_count() == 1);
    {// copy
        auto cp = p;
        TEST_TRUE(cp.get() == p.get());
        TEST_TRUE(cp.use_count() == 2);
    }
    TEST_TRUE(p.use_count() == 1);
    p.reset();
    TEST_FALSE(p);
}
