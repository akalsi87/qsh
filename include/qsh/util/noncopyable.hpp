/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file noncopyable.hpp
 * \date Nov 12, 2016
 */

#ifndef _qsh_util_noncopyable_hpp_
#define _qsh_util_noncopyable_hpp_

namespace qsh {

struct noncopyable
{
    noncopyable() = default;
    ~noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

}// namespace qsh

#endif/*_qsh_util_noncopyable_hpp_*/
