/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file stack_storage.hpp
 * \date Nov 12, 2016
 */

#ifndef _qsh_util_stack_storage_hpp_
#define _qsh_util_stack_storage_hpp_

namespace qsh {

template <int sz>
struct stack_storage
{
    union
    {
        double __align;
        char   data[sz];
    };

    static const int size = sz;
};

} // namespace qsh

#endif/*_qsh_util_stack_storage_hpp_*/
