/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file Arena.cpp
 * \date Nov 13, 2016
 */

#include "qsh/alloc/arena.hpp"

#include "unittest.hpp"

CPP_TEST( arena_none )
{
    qsh::arena a;
}

CPP_TEST( arena_basic )
{
    qsh::arena a;
    {/* simple create */
        int* p = a.create<int>();
        TEST_TRUE(*p == 0);
    }
    {/* create int array */
        int* arr = a.create_array<int>(50);
        arr[0] = 0;
        arr[49] = 49;
    }
    {/* variadic create */
        struct Foo
        {
            size_t a;
            void* b;

            Foo(size_t x, void* y) : a(x), b(y) { }
        };

        auto pfoo = a.create<Foo>(42, reinterpret_cast<void*>(42));
        TEST_TRUE(pfoo->a == 42);
        TEST_TRUE(pfoo->b == reinterpret_cast<void*>(42));
    }
    {/* new segment create */
        struct Foo
        {
            char bytes[2048];
        };

        auto pfoo = a.create<Foo>();
        static_cast<void>(pfoo);
    }
    {/* new segment create; non trivial dtor */
        struct Foo
        {
            char bytes[2048];

            ~Foo()
            {
                printf("Ran %s()\n", QSH_FUNCTION_NAME);
            }
        };

        auto pfoo = a.create<Foo>();
        static_cast<void>(pfoo);
    }
    {/* new segment create_array; non trivial dtor */
        struct Foo
        {
            char bytes[2048];

            ~Foo()
            {
                printf("Ran %s()\n", QSH_FUNCTION_NAME);
            }
        };

        auto pfoo = a.create_array<Foo>(3);
        for (int i = 0; i < 3; ++i) {
            new (&pfoo[i]) Foo;
        }
    }
}
