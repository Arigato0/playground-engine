#pragma once

#include "defer.hpp"

#define CONCAT(a, b) __CONCAT__(a, b)
#define __CONCAT__(a, b) a##b

#define EXPAND_VEC4(vec) vec.x, vec.y, vec.z, vec.w
#define EXPAND_VEC3(vec) vec.x, vec.y, vec.z
#define EXPAND_VEC2(vec) vec.x, vec.y

#define ASSERT_ERR(err) \
do                        \
{                         \
    auto __RESULT023923 = (err);\
    if (__RESULT023923 != pge::ErrorCode::Ok) Logger::fatal(error_message(__RESULT023923)); \
} while(false)                   \

#define VALIDATE_ERR_AS(err, T) \
do                        \
{                         \
    auto __RESULT023923 = (err);\
    if ((T)__RESULT023923 != 0) return (T)__RESULT023923; \
} while(false)            \

#define VALIDATE_ERR(err) \
do                        \
{                         \
    auto __RESULT023923 = (err);\
    if ((int)__RESULT023923 != 0) return __RESULT023923; \
} while(false)            \

#define VALIDATE_RESULT_ERR(result) \
do                        \
{                         \
    auto __RESULT42315 = (result);\
    if (!__RESULT42315.ok()) return __RESULT023923.error(); \
} while(false)                      \

#define VALIDATE_RESULT(result) \
do                        \
{                         \
    auto __RESULT42315 = (result);\
    if (!__RESULT42315.ok()) return __RESULT023923; \
} while(false)                  \

