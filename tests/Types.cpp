/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file Types.cpp
 * \date Nov 13, 2016
 */

#include "qsh/types/types.hpp"

#include "unittest.hpp"

CPP_TEST( test_uniqueness )
{
    qsh::type_factory f;
    using qsh::type;
    {// types...
        int kindBeg = type::INT;
        int kindEnd = static_cast<int>(type::UNKNOWN) + 1;
        for (int i = kindBeg; i < kindEnd; ++i) {
            auto kind = static_cast<type::kind_type>(i);
            auto p = f.get(kind);
            TEST_TRUE(p);
            TEST_TRUE(f.get(kind) == p);
            TEST_TRUE(p->kind() == kind);
            TEST_TRUE(p->kind() == type::TUPLE || p->impl());
            TEST_TRUE(p->is_unknown() == (kind == type::UNKNOWN));
            TEST_TRUE(p->is_tuple() == (kind == type::TUPLE));
            TEST_TRUE(p->types().size() == 0);
        }
    }
    {// tuple type
        using qsh::types_range;
        type const* ts[3] = { f.get(type::INT), f.get(type::CHAR), f.get(type::STRING_ARR) };
        auto p = f.get(type::TUPLE, types_range(ts, 3));
        TEST_TRUE(p);
        TEST_TRUE(p == f.get(type::TUPLE, types_range(ts, 3)));
        TEST_TRUE(p->types().size() == 3);
        TEST_TRUE(p->types()[0] == ts[0]);
        TEST_TRUE(p->types()[1] == ts[1]);
        TEST_TRUE(p->types()[2] == ts[2]);
    }
}

CPP_TEST( test_type_impl )
{
    qsh::type_factory f;
    using qsh::type;
    {// type INT
        type::int_type x = 5;
        type::int_type y;
        auto tInt = f.get(type::INT);
        ASSERT_TRUE(tInt && tInt->impl());
        TEST_TRUE(tInt->impl()->type_size() == sizeof(type::int_type));
        tInt->impl()->copy(&y, &x);
        TEST_TRUE(y == x);
    }
}
