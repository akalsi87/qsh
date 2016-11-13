/*
Copyright (c) 2016 Aaditya Kalsi - All Rights Reserved.
*/

/**
 * \file config.h
 * \date Nov 11, 2016
 */

#ifndef _qsh_config_h_
#define _qsh_config_h_

#if defined(BUILD_QSH)
#  define QSH_API __attribute__ ((visibility ("default")))
#else
#  define QSH_API
#endif

#if defined(__cplusplus)
#  define QSH_EXTERN_C extern "C"
#  define QSH_NOEXCEPT noexcept
#  define QSH_STATIC_ASSERT(e, m) static_assert((e),(#m))
#else
#  define QSH_EXTERN_C
#  define QSH_NOEXCEPT
#  define __CONCAT_(a, b) a##b
#  define __CONCAT(a, b) __CONCAT_(a, b)
#  define QSH_STATIC_ASSERT(e, m) char __CONCAT(m, _assert)[e ? 1 : -1]
#endif

#define QSH_VER "0.0.1"

inline void __useless()
{
#if !defined(QSH_FUNCTION_NAME)
#  if defined(__GNUC__)
#    define QSH_FUNCTION_NAME __PRETTY_FUNCTION__
#  elif defined(_MSC_VER)
#   define QSH_FUNCTION_NAME __FUNCTION__
#  elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#    define QSH_FUNCTION_NAME __func__
#  else
#    define QSH_FUNCTION_NAME "(unknown function)"
#  endif
#endif
}

#include "stdint.h"
#include "stddef.h"

#endif/*_qsh_config_h_*/
